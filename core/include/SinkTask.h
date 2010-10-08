#include "Data.h"
#include "SinkTaskMgr.h"
#include "TaskEventCallback.h"

#ifndef __SINKTASK_H
#define __SINKTASK_H

class SinkTaskBase
{
public:
  SinkTaskBase();
  SinkTaskBase(const SinkTaskBase &aTask);
  

  //@brief start the processing of this SinkTask
  virtual void process(Data&) {};

  void setEventCallback(TaskEventCallback *aEventCbk);
  TaskEventCallback* getEventCallback() {return _eventCbkPt;}

  void ref();
  void unref();
  
  // @brief only use for debuging purpose
  int getRefCounter() const;
protected:
  virtual ~SinkTaskBase();

  mutable pthread_mutex_t _lock;

private:
  TaskEventCallback	*_eventCbkPt;
  int			_refCounter;
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
