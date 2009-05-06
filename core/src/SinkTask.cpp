#include "SinkTask.h"

SinkTaskBase::SinkTaskBase() : 
  _eventCbkPt(NULL),
  _refCounter(1)
{
  pthread_mutex_init(&_lock,NULL);
}

SinkTaskBase::SinkTaskBase(const SinkTaskBase &aTask) :
  _refCounter(1)
{
  if(aTask._eventCbkPt)
    aTask._eventCbkPt->ref();
  _eventCbkPt = aTask._eventCbkPt;
  pthread_mutex_init(&_lock,NULL);
}

SinkTaskBase::~SinkTaskBase()
{
  if(_eventCbkPt)
    _eventCbkPt->unref();
  pthread_mutex_destroy(&_lock);
}

void SinkTaskBase:: setEventCallback(TaskEventCallback *aEventCbk)
{
  if(_eventCbkPt)
    {
      _eventCbkPt->unref();
      _eventCbkPt = NULL;
    }
  if(aEventCbk)
    {
      aEventCbk->ref();
      _eventCbkPt = aEventCbk;
    }
}

void SinkTaskBase::ref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  ++_refCounter;
}

void SinkTaskBase::unref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  if(!(--_refCounter))
    {
      aLock.unLock();
      delete this;
    }
}

