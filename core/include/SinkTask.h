#include "Data.h"
#include "SinkTaskMgr.h"
#include "TaskEventCallback.h"

#ifndef __SINKTASK_H
#define __SINKTASK_H

class SinkTaskBase
{
public:
  SinkTaskBase() : _eventCbkPt(NULL) {}
  SinkTaskBase(const SinkTaskBase &aTask)
    {
      if(aTask._eventCbkPt)
	aTask._eventCbkPt->ref();
      _eventCbkPt = aTask._eventCbkPt;
    }
  virtual ~SinkTaskBase()
    {
      if(_eventCbkPt)
	_eventCbkPt->unref();
    }
  //@brief start the processing of this SinkTask
  virtual void process(Data&) {};
  //@brief like a copy constructor
  virtual SinkTaskBase* copy() const {return NULL;}

  void setEventCallback(TaskEventCallback *aEventCbk)
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
  TaskEventCallback* getEventCallback() {return _eventCbkPt;}
 private:
  TaskEventCallback *_eventCbkPt;
};

template<class T>
class SinkTask : public SinkTaskBase
{
public:
  SinkTask(SinkTaskMgr<T> &aMgr) : _mgr(aMgr) {};
  SinkTask(const SinkTask &aTask) : SinkTaskBase(aTask),_mgr(aTask._mgr) {}
protected:
  SinkTaskMgr<T>	&_mgr;
};

#endif
