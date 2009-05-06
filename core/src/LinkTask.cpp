#include "LinkTask.h"
#include "PoolThreadMgr.h"

LinkTask::LinkTask() :
  _processingInPlaceFlag(true),
  _eventCbkPt(NULL),_refCounter(1)
{
  pthread_mutex_init(&_lock,NULL);
}

LinkTask::LinkTask(bool aProcessingInPlaceFlag) :
  _processingInPlaceFlag(aProcessingInPlaceFlag),
  _eventCbkPt(NULL),_refCounter(1)
{
  pthread_mutex_init(&_lock,NULL);
}

LinkTask::LinkTask(const LinkTask &aLinkTask) :
  _processingInPlaceFlag(aLinkTask._processingInPlaceFlag)
{
  if(aLinkTask._eventCbkPt)
      aLinkTask._eventCbkPt->ref();
  _eventCbkPt = aLinkTask._eventCbkPt;
  pthread_mutex_init(&_lock,NULL);
}

LinkTask::~LinkTask()
{
  if(_eventCbkPt)
    _eventCbkPt->unref();
  pthread_mutex_destroy(&_lock);
}
void LinkTask::setEventCallback(TaskEventCallback *aEventCbk)
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

void LinkTask::ref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  ++_refCounter;
}

void LinkTask::unref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  if(!(--_refCounter))
    {
      aLock.unLock();
      delete this;
    }
}
