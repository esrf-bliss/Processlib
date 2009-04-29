#include "Data.h"
#include "SinkTaskMgr.h"

#ifndef __SINKTASK_H
#define __SINKTASK_H

class SinkTaskBase
{
public:
  virtual ~SinkTaskBase() {}
  //@brief start the processing of this SinkTask
  virtual void process(Data&) {};
  //@brief like a copy constructor
  virtual SinkTaskBase* copy() const {return NULL;}
};

template<class T>
class SinkTask : public SinkTaskBase
{
public:
  SinkTask(SinkTaskMgr<T> &aMgr) : _mgr(aMgr) {};
  
protected:
  SinkTaskMgr<T>	&_mgr;
};

#endif
