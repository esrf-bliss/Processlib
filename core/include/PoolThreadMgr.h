#ifndef __POOLTHREAD_H__
#define __POOLTHREAD_H__

#ifdef __cplusplus

#include <list>
#include <vector>
#include <pthread.h>
class TaskMgr;
struct Data;

enum START_PROCESS_ERROR {STARTED = 0,NO_TASK,BAD_DATA,OVERRUN};
extern "C"
{
#endif
  START_PROCESS_ERROR startNewProcess(long frameNumber,
				      int depth,int width,int height,
				      const char *data,void*);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
class PoolThreadMgr
{
  friend START_PROCESS_ERROR startNewProcess(long,int,int,int,const char*);
public:
  PoolThreadMgr();
  ~PoolThreadMgr();
  static inline PoolThreadMgr& get() throw() 
  {return PoolThreadMgr::_processMgr;}
  
  void addProcess(TaskMgr *aProcess,bool lock = true);
  void removeProcess(TaskMgr *aProcess,bool lock = true);
  void setNumberOfThread(int);
  void setQueueLimit(int);
  int queueLimit();
  void setTaskMgr(const TaskMgr *);
  void abort();
  void quit();

  class Lock
  {
  public:
    Lock(pthread_mutex_t *aLock,bool aLockFlag = true) :
      _lock(aLock),_lockFlag(false)
    {if(aLockFlag) lock();}

    ~Lock() {unLock();}
    inline void lock() 
    {
      if(!_lockFlag)
	while(pthread_mutex_lock(_lock));
      _lockFlag = true;
    }
    inline void unLock()
    {
      if(_lockFlag)
	{
	  _lockFlag = false;
	  pthread_mutex_unlock(_lock);
	}
    }
  private:
    pthread_mutex_t *_lock;
    volatile bool   _lockFlag;
  };

private:
  pthread_mutex_t                    _lock;
  pthread_cond_t                     _cond;
  volatile bool                      _stopFlag;
  volatile bool			     _suspendFlag;
  volatile int			     _queueLimit;
  volatile int			     _runningThread;
  std::list<TaskMgr*>   _processQueue;
  std::vector<pthread_t>             _threadID;
  static PoolThreadMgr               _processMgr;
  TaskMgr		             *_taskMgr;
  static void* _run(void*);
  void _createProcessThread(int aNumber);
};

#endif

#endif
