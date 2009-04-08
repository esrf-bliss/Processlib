#include "TaskMgr.h"
#include "Task.h"
#include "PoolThreadMgr.h"

TaskMgr::TaskMgr() {}

TaskMgr::TaskMgr(const TaskMgr &aMgr)
{
  _Tasks.resize(aMgr._Tasks.size());
  StageTask::iterator i = _Tasks.begin();
  StageTask::const_iterator j = aMgr._Tasks.begin();
  for(;i != _Tasks.end();++i,++j)
    for(std::deque<Task*>::const_iterator k = j->begin();
	k != j->end();++k)
      i->push_back((*k)->copy());
}

TaskMgr::~TaskMgr()
{
}

void TaskMgr::addTask(int aStage,Task *aNewTask)
{
  if(int(_Tasks.size()) <= aStage)
    _Tasks.resize(aStage + 1);
  std::deque<Task*> &aStageTask = _Tasks[aStage];
  aStageTask.push_back(aNewTask);
}

TaskMgr::TaskWrap TaskMgr::next()
{
  std::deque<Task*> &aStageTaskList = _Tasks.front();
  Task *aNextTask = aStageTaskList.front();
  aStageTaskList.pop_front();
  _PendingTask.push_back(std::pair<Task*,bool>(aNextTask,false));
  if(aStageTaskList.empty())		// Nothing else for this stage
    {
       // Remove processmrg from the queue
      PoolThreadMgr::get().removeProcess(this,false);
      _Tasks.pop_front();
    }
  return TaskWrap(*this,aNextTask);
}

void TaskMgr::_endTask(Task *aFinnishedTask)
{
  int aNbPendingTask = _PendingTask.size();
  int aNbFinnishedTask = 0;
  for(PendingTask::iterator i = _PendingTask.begin();
      i != _PendingTask.end();++i)
    {
      if(i->first == aFinnishedTask)
	delete i->first,i->second = true;
      if(i->second) ++aNbFinnishedTask;
    }
 // Processing Stage finnished, going to next stage
  if(aNbFinnishedTask == aNbPendingTask)
    {
      _currentData = _nextData;	// swap
      _nextData.releaseBuffer();
      _PendingTask.clear();	// Reset Pending task for this stage
      if(_Tasks.empty())	// Process is finnished
	delete this;		// suicide
      else
	PoolThreadMgr::get().addProcess(this,false); // Re insert in the Poll
    }
}

void TaskMgr::syncProcess()
{
  for(StageTask::iterator aStageTaskList = _Tasks.begin();
      aStageTaskList != _Tasks.end();++aStageTaskList)
    {
      for(std::deque<Task*>::iterator aTaskIter = aStageTaskList->begin();
	  aTaskIter != aStageTaskList->end();++aTaskIter)
	{
	  Data aResult = (*aTaskIter)->process(_currentData);
	  if(!aResult.empty())
	    _nextData = aResult;
	  delete *aTaskIter;	// delete the task
	}
      _currentData = _nextData; // swap
      _nextData.releaseBuffer();
    }
  _Tasks.clear();
}
//Class TaskMgr::TaskWrap

TaskMgr::TaskWrap::TaskWrap(TaskMgr &aMgr,Task *aTask) :
  _Task(aTask),_Mgr(aMgr) {}

TaskMgr::TaskWrap::~TaskWrap()
{
  _Mgr._endTask(_Task);
}
void TaskMgr::TaskWrap::operator() ()
{
  Data aResult = _Task->process(_Mgr._currentData);
  if(!aResult.empty())
    _Mgr._nextData = aResult;
}
