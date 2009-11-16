#include <iostream>
#include <sys/time.h>
#include <errno.h>
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
  _suspendFlag = false;
  _runningThread = 0;
  _queueLimit = QUEUE_DEFAULT_LIMIT;
  _taskMgr = NULL;
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

/** @brief change the number of thread in the pool
 * @warning this methode is not MT-Safe
 * @see quit
 */
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
 *  each time an image is received, this TaskMgr is clone
 * @param aMgr set a TaskMgr or NULL to remove the previous one
 */
void PoolThreadMgr::setTaskMgr(const TaskMgr *aMgr)
{
  TaskMgr *refBackgrounMgr = NULL;
  if(aMgr)
    refBackgrounMgr = new TaskMgr(*aMgr);
  Lock aLock(&_lock);
  delete _taskMgr;
  _taskMgr = refBackgrounMgr;
}

/** @brief clean quit
 * @warning this methode is not MT-Safe
 * @see setNumberOfThread
 */

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
  _stopFlag = false;
  _threadID.clear();
}
/** @brief abort all process
 */
void PoolThreadMgr::abort()
{
  Lock aLock(&_lock);
  _suspendFlag = true;
  while(_runningThread) pthread_cond_wait(&_cond,&_lock);
  for(std::list<TaskMgr*>::iterator i = _processQueue.begin();
      i != _processQueue.end();++i)
    delete *i;
  _processQueue.clear();
  _suspendFlag = false;
}

/** @brief suspend all process
 */
void PoolThreadMgr::suspend(bool aFlag)
{
  Lock aLock(&_lock);
  _suspendFlag = aFlag;
  if(aFlag)
    pthread_cond_broadcast(&_cond);
}
/** @brief wait until queue is empty
 */
bool PoolThreadMgr::wait(double askedTimeout)
{
  if(askedTimeout >= 0.)
    {
      struct timeval now;
      struct timespec timeout;
      int retcode = 0;
      gettimeofday(&now,NULL);
      timeout.tv_sec = now.tv_sec + long(askedTimeout);
      timeout.tv_nsec = (now.tv_usec * 1000) + 
	long((askedTimeout - long(askedTimeout)) * 1e9);
      Lock aLock(&_lock);
      while(_runningThread)
	retcode = pthread_cond_timedwait(&_cond,&_lock,&timeout);
      return retcode != ETIMEDOUT;
    }
  else
    {
      Lock aLock(&_lock);
      while(_runningThread)
	pthread_cond_wait(&_cond,&_lock);
      return true;
    }
}
void* PoolThreadMgr::_run(void *arg) 
{
  PoolThreadMgr* processMgrPt = (PoolThreadMgr*)arg;
  Lock aLock(&processMgrPt->_lock);
  processMgrPt->_runningThread++;
  while(1)
    {
      processMgrPt->_runningThread--;

      if(processMgrPt->_suspendFlag)
	 pthread_cond_broadcast(&processMgrPt->_cond);	// synchro if abort

      while(processMgrPt->_suspendFlag ||
	    (!processMgrPt->_stopFlag && processMgrPt->_processQueue.empty()))
	 pthread_cond_wait(&processMgrPt->_cond,&processMgrPt->_lock);
      
      processMgrPt->_runningThread++;

      if(!processMgrPt->_processQueue.empty())
	{
	  TaskMgr *processPt = processMgrPt->_processQueue.front();
	  TaskMgr::TaskWrap *aNextTask = processPt->next();
	  aLock.unLock();
	  try
	    {
	      aNextTask->process();
	    }
	  catch(...)
	    {
	    }
	  aLock.lock();
	  delete aNextTask;
	}
      else break;		// stop
    }
  processMgrPt->_runningThread--;
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
 * @return 
 * - STARTED every thing is OK
 * - NO_TASK no task manager defined
 * - BAD_DATA input data not valid
 * - OVERRUN queue limit exceeded 
 */
START_PROCESS_ERROR startNewProcess(long frameNumber,
				    int depth,int width,int height,const char *data)
{
  const char *errorMessagePt = NULL;
  START_PROCESS_ERROR errorEnum = NO_TASK;
  PoolThreadMgr &pollThreadMgr = PoolThreadMgr::get();
  PoolThreadMgr::Lock aLock(&pollThreadMgr._lock);
  TaskMgr *aTaskMgr = pollThreadMgr._taskMgr;

  if(aTaskMgr)
    {
      if(pollThreadMgr._queueLimit > 0 && 
	 int(pollThreadMgr._processQueue.size()) > pollThreadMgr._queueLimit)
	{
	  errorEnum = OVERRUN;
	  errorMessagePt = "ProcessLib overrun";
	  goto error;
	}

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
	  errorEnum = BAD_DATA;
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
	  errorEnum = BAD_DATA;
	  goto error;
	}

      TaskMgr *aNewTaskPt = new TaskMgr(*aTaskMgr);
      aNewTaskPt->setInputData(aNewData);
      pollThreadMgr.addProcess(aNewTaskPt,false);
      errorEnum = STARTED;	// OK
    }
  else
    errorMessagePt = "There is no image chain process set";
  
 error:
  if(errorMessagePt)
    std::cerr << "Processlib core : " << errorMessagePt << std::endl;
  return errorEnum;
}
