#include "PoolThreadMgr.h"
#include "TaskEventCallback.h"

TaskEventCallback::TaskEventCallback() : _refCounter(1)
{
  pthread_mutex_init(&_lock,NULL);
}

TaskEventCallback::~TaskEventCallback()
{
  pthread_mutex_destroy(&_lock);
}

void TaskEventCallback::ref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  ++_refCounter;
}

void TaskEventCallback::unref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  if(!(--_refCounter))
    {
      aLock.unLock();
      delete this;
    }
}
