#include <iostream>
#include "PoolThreadMgr.h"
#include "TaskMgr.h"

//Static variable
static const int NB_DEFAULT_THREADS = 2;
static const int QUEUE_DEFAULT_LIMIT = 8;
PoolThreadMgr PoolThreadMgr::_processMgr;

PoolThreadMgr::PoolThreadMgr()
{
  pthread_mutex_init(&_lock,NULL);
  pthread_cond_init(&_cond,NULL);

  _stopFlag = false;
  _queueLimit = QUEUE_DEFAULT_LIMIT;
  _backgroundProcessMgrSequential = _backgroundProcessMgrParallel = NULL;
  _createProcessThread(NB_DEFAULT_THREADS);
}

PoolThreadMgr::~PoolThreadMgr()
{
  quit();
  pthread_mutex_destroy(&_lock);
  pthread_cond_destroy(&_cond);
}
/** @brief add a process in the process queue.
 *  Notice that after calling this methode aProcess will be own by PoolThreadMgr,
 *  do not modify or even delete it.
 *  @param aProcess a background process
 *  @param aFlag if true lock the queue which is the default
*/
void PoolThreadMgr::addProcess(TaskMgr *aProcess,bool aFlag) 
{
  Lock aLock(&_lock,aFlag);
  _processQueue.push_back(aProcess);
  pthread_cond_broadcast(&_cond);
}

void PoolThreadMgr::removeProcess(TaskMgr *aProcess,bool aFlag)
{
  Lock aLock(&_lock,aFlag);
  for(std::list<TaskMgr*>::iterator i = _processQueue.begin();
      i != _processQueue.end();++i)
    {
      if(*i == aProcess)
	{
	  _processQueue.erase(i);
	  if(_queueLimit == int(_processQueue.size()))
	    pthread_cond_broadcast(&_cond);
	  break;
	}
    }
}
//@warning this methode is not MT-Safe
void PoolThreadMgr::setNumberOfThread(int nbThread) 
{
  if(int(_threadID.size()) <= nbThread)
    _createProcessThread(nbThread - _threadID.size());
  else
    {
      quit();
      _createProcessThread(nbThread);
    }
      
}
/** @brief set the process queue limit.
    This methode limit the number of process queued by startNewProcess
    @see startNewProcess
    @param size the queue size limit, if <= 0 unlimited
*/
void PoolThreadMgr::setQueueLimit(int size)
{
  Lock aLock(&_lock);
  _queueLimit = size;
  pthread_cond_broadcast(&_cond);
}
/** @brief get the max lenght queue process
 */
int PoolThreadMgr::queueLimit()
{
  Lock aLock(&_lock);
  return _queueLimit;
}
/** @brief set the image processing mgr 
 *
 *  this is the way to defined the chained process of all images.
 *  each time an image is received, this BackgroundProcess is clone
 */
void PoolThreadMgr::setTaskMgr(const TaskMgr *aMgr,
					    PoolThreadMgr::SyncMode aSyncMode)
{
  TaskMgr *refBackgrounMgr = NULL;
  if(aMgr)
    refBackgrounMgr = new TaskMgr(*aMgr);
  Lock aLock(&_lock);
  TaskMgr *&backGroundProcess = 
    (aSyncMode == PoolThreadMgr::Sequential) ? _backgroundProcessMgrSequential : _backgroundProcessMgrParallel;
  delete backGroundProcess;
  backGroundProcess = refBackgrounMgr;
}

/** @brief get background process manager address
 *  @return the address of the choosen manager
 */
long PoolThreadMgr::backgroundProcessMgrAddress(SyncMode aSyncMode)
{
  Lock aLock(&_lock);
  return (aSyncMode == PoolThreadMgr::Sequential) ? 
    (long)_backgroundProcessMgrSequential : (long)_backgroundProcessMgrParallel;
}
//@warning this methode is not MT-Safe
void PoolThreadMgr::quit()
{
  Lock aLock(&_lock);
  _stopFlag = true;
  pthread_cond_broadcast(&_cond);
  aLock.unLock();

  for(std::vector<pthread_t>::iterator i = _threadID.begin();
      i != _threadID.end();++i)
    {
      void *returnThread;
      while(pthread_join(*i,&returnThread));
    }
  _threadID.clear();
}

void* PoolThreadMgr::_run(void *arg) 
{
  PoolThreadMgr* processMgrPt = (PoolThreadMgr*)arg;
  Lock aLock(&processMgrPt->_lock);
  while(1)
    {
      while(!processMgrPt->_stopFlag && processMgrPt->_processQueue.empty())
	pthread_cond_wait(&processMgrPt->_cond,&processMgrPt->_lock);
      if(!processMgrPt->_processQueue.empty())
	{
	  TaskMgr *processPt = processMgrPt->_processQueue.front();
	  TaskMgr::TaskWrap aNextTask = processPt->next();
	  aLock.unLock();
	  try
	    {
	      aNextTask();
	    }
	  catch(...)
	    {
	    }
	  aLock.lock();
	}
      else break;
    }
  return NULL;
}

void PoolThreadMgr::_createProcessThread(int aNumber)
{
  for(int i = aNumber;i;--i)
    {
      pthread_t  threadID;
      if(!pthread_create(&threadID,NULL,_run,this))
	_threadID.push_back(threadID);
    }
}
/** @brief this function is the entry point of the processing library.
 *
 * This function is usually called by an external acquisition library.
 * new process is limited by the queue limit size @see setQueueLimit
 * @param frameNumber the image unique id
 * @param depth the number of bytes per pixel
 * @param width the image width
 * @param height the image height
 * @param data the array data pointer
 * @param voidTaskMgr an address of a backgroundProcessMgr @see backgroundProcessMgrAddress
 * @return true if all ok
 */
bool startNewProcess(long frameNumber,
		     int depth,int width,int height,const char *data,
		     void *voidTaskMgr)
{
  const char *errorMessagePt = NULL;
  PoolThreadMgr &pollThreadMgr = PoolThreadMgr::get();
  PoolThreadMgr::Lock aLock(&pollThreadMgr._lock);
  TaskMgr *aTaskMgr = (TaskMgr*)voidTaskMgr;
  PoolThreadMgr::SyncMode syncMode = (aTaskMgr == pollThreadMgr._backgroundProcessMgrSequential) ? 
    PoolThreadMgr::Sequential : PoolThreadMgr::Parallel;

  if(aTaskMgr)
    {
      TaskMgr *aNewTaskPt = new TaskMgr(*aTaskMgr);
      //Init new Data
      Data aNewData = Data();
      aNewData.frameNumber = frameNumber;
      aNewData.width = width;
      aNewData.height = height;
      switch(depth)
	{
	case 1: aNewData.type = Data::UINT8;break;
	case 2: aNewData.type = Data::UINT16;break;
	case 4: aNewData.type = Data::UINT32;break;
	case 8: aNewData.type = Data::UINT64;break;
	default:
	  errorMessagePt = "Depth not managed";
	  goto error;
	}
      if(data)
	{
	  Buffer *aNewBuffer = new Buffer();
	  aNewBuffer->owner = Buffer::MAPPED;
	  aNewBuffer->data = (void*)data;
	  aNewData.setBuffer(aNewBuffer);
	  aNewBuffer->unref();
	}
      else
	{
	  errorMessagePt = "Data array is empty";
	  goto error;
	}
      aNewTaskPt->setInputData(aNewData);
      if(syncMode == PoolThreadMgr::Sequential)
	{
	  aLock.unLock();
	  aNewTaskPt->syncProcess();
	  delete aNewTaskPt;
	}
      else
	{
	  while(pollThreadMgr._queueLimit > 0 && 
		int(pollThreadMgr._processQueue.size()) > pollThreadMgr._queueLimit)
	    pthread_cond_wait(&pollThreadMgr._cond,&pollThreadMgr._lock);
	  pollThreadMgr.addProcess(aNewTaskPt,false);
	}
      return true;
    }
  else
    errorMessagePt = "There is no image chain process set";
      
 error:
  if(errorMessagePt)
    std::cerr << "Processlib core : " << errorMessagePt << std::endl;
  return false;
}
