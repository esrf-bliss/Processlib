#include "TaskMgr.h"
#include "LinkTask.h"
#include "SinkTask.h"
#include "PoolThreadMgr.h"

//Class TaskMgr::TaskLinkWrap
class TaskLinkWrap : public TaskMgr::TaskWrap
{
public:
  TaskLinkWrap(TaskMgr &aMgr,LinkTask *aTask) : TaskWrap(aMgr),_LinkTask(aTask) {}
  virtual ~TaskLinkWrap()
  {
    _endLinkTask(_LinkTask);
  }
  virtual void process()
  {
    TaskEventCallback* eventCbkPt = _LinkTask->getEventCallback();
    if(eventCbkPt)
      eventCbkPt->started(_currentData());
    Data aResult = _LinkTask->process(_currentData());
    if(eventCbkPt)
      eventCbkPt->finnished(aResult);
    _setNextData(aResult);
  }
private:
  LinkTask	*_LinkTask;
};

//Class TaskSinkWrap
class TaskSinkWrap : public TaskMgr::TaskWrap
{
public:
  TaskSinkWrap(TaskMgr &aMgr,SinkTaskBase *aTask) : TaskWrap(aMgr),_SinkTask(aTask) {}
  virtual ~TaskSinkWrap()
  {
    _endSinkTask(_SinkTask);
  }
  virtual void process()
  {
    TaskEventCallback* eventCbkPt = _SinkTask->getEventCallback();
    if(eventCbkPt)
      eventCbkPt->started(_currentData());
    _SinkTask->process(_currentData());
    if(eventCbkPt)
      eventCbkPt->finnished(_currentData());
  }
private:
  SinkTaskBase	*_SinkTask;
};

//struct Task

TaskMgr::Task* TaskMgr::Task::copy() const
{
  TaskMgr::Task *aReturnTask = new TaskMgr::Task();
  aReturnTask->_linkTask = _linkTask ? _linkTask->copy() : NULL;
  for(std::deque<SinkTaskBase*>::const_iterator i = _sinkTaskQueue.begin();
      i != _sinkTaskQueue.end();++i)
    aReturnTask->_sinkTaskQueue.push_back((*i)->copy());
  return aReturnTask;
}

TaskMgr::Task::~Task()
{
  delete _linkTask;
  for(std::deque<SinkTaskBase*>::iterator i = _sinkTaskQueue.begin();
      i != _sinkTaskQueue.end();++i)
    delete *i;
}

TaskMgr::TaskMgr() {}

TaskMgr::TaskMgr(const TaskMgr &aMgr)
{
  for(StageTask::const_iterator i = aMgr._Tasks.begin();
      i != aMgr._Tasks.end();++i)
    _Tasks.push_back((*i)->copy());
  _currentData = aMgr._currentData;
}

TaskMgr::~TaskMgr()
{
  for(StageTask::iterator i = _Tasks.begin();
      i != _Tasks.end();++i)
    delete *i;
}
/** @brief add a linked task
 *  @return true if task set for stage
 */
bool TaskMgr::setLinkTask(int aStage,LinkTask *aNewTask)
{
  bool aReturnFlag = false;
  while(int(_Tasks.size()) <= aStage)
    _Tasks.push_back(new Task());
  if(!_Tasks[aStage]->_linkTask)
    _Tasks[aStage]->_linkTask = aNewTask,aReturnFlag =  true;

  return aReturnFlag;
}

void TaskMgr::addSinkTask(int aStage,SinkTaskBase *aNewTask)
{
  while(int(_Tasks.size()) <= aStage)
    _Tasks.push_back(new Task());
  _Tasks[aStage]->_sinkTaskQueue.push_back(aNewTask);
}
TaskMgr::TaskWrap* TaskMgr::next()
{
#define CHECK_END_STAGE()					\
  if(aTaskPt->_sinkTaskQueue.empty())				\
    {								\
      PoolThreadMgr::get().removeProcess(this,false);		\
      delete aTaskPt;						\
      _Tasks.pop_front();					\
    }								

  Task *aTaskPt = _Tasks.front();
  if(aTaskPt->_linkTask)		// first linked task
    {
      _PendingLinkTask = aTaskPt->_linkTask;
      aTaskPt->_linkTask = NULL;
      CHECK_END_STAGE();
      return new TaskLinkWrap(*this,_PendingLinkTask);
    }
  else
    {
      SinkTaskBase *aNewSinkTaskPt = aTaskPt->_sinkTaskQueue.front();
      aTaskPt->_sinkTaskQueue.pop_front();
      _PendingSinkTask.push_back(std::pair<SinkTaskBase*,bool>(aNewSinkTaskPt,false));
      CHECK_END_STAGE();
      return new TaskSinkWrap(*this,aNewSinkTaskPt);
    }
}

void TaskMgr::_endLinkTask(LinkTask*)
{
  delete _PendingLinkTask;
  _PendingLinkTask = NULL;
  int aNbPendingSinkTask = _PendingSinkTask.size();
  int aNbFinnishedSinkTask = 0;
  for(PendingSinkTask::iterator i = _PendingSinkTask.begin();
      i != _PendingSinkTask.end();++i)
    if(i->second) ++aNbFinnishedSinkTask;
  
  if(aNbFinnishedSinkTask == aNbPendingSinkTask) 
    _goToNextStage();
}

void TaskMgr::_endSinkTask(SinkTaskBase *aFinnishedTask)
{
  int aNbPendingTask = _PendingSinkTask.size();
  int aNbFinnishedTask = 0;
  for(PendingSinkTask::iterator i = _PendingSinkTask.begin();
      i != _PendingSinkTask.end();++i)
    {
      if(i->first == aFinnishedTask)
	delete i->first,i->second = true;
      if(i->second) ++aNbFinnishedTask;
    }
  if(!_PendingLinkTask && aNbFinnishedTask == aNbPendingTask)
    {
      _PendingSinkTask.clear();	// Reset Pending task for this stage
      _goToNextStage();
    }
}

//@brief Processing Stage finnished, going to next stage
void TaskMgr::_goToNextStage()
{
  _currentData = _nextData;	// swap
  _nextData.releaseBuffer();
  if(_Tasks.empty())	// Process is finnished
    delete this;		// suicide
  else
    PoolThreadMgr::get().addProcess(this,false); // Re insert in the Poll
}

void TaskMgr::syncProcess()
{
  for(StageTask::iterator aStageTask = _Tasks.begin();
      aStageTask != _Tasks.end();++aStageTask)
    {
      Task *aStageTaskPt = *aStageTask;
      if(aStageTaskPt->_linkTask)
	{
	  _nextData = aStageTaskPt->_linkTask->process(_currentData);
	  delete aStageTaskPt->_linkTask;
	  aStageTaskPt->_linkTask = NULL;
	}
      for(std::deque<SinkTaskBase*>::iterator aSinkTask = aStageTaskPt->_sinkTaskQueue.begin();
	  aSinkTask != aStageTaskPt->_sinkTaskQueue.end();++aSinkTask)
	{
	  (*aSinkTask)->process(_currentData);
	  delete *aSinkTask;
	}
      aStageTaskPt->_sinkTaskQueue.clear();
      if(!_nextData.empty())
	{
	  _currentData = _nextData; // swap
	  _nextData.releaseBuffer();
	}
      delete aStageTaskPt;
    }
  _Tasks.clear();
}
