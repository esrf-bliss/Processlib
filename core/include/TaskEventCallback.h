#include "Data.h"
#include <pthread.h>

#ifndef __TASKEVENTCALLBACK_H
#define __TASKEVENTCALLBACK_H

class DLL_EXPORT TaskEventCallback
{
 public:
  TaskEventCallback();
  virtual void started(Data &) {}
  virtual void finished(Data &) {}
  virtual void error(Data &,const char*) {}
  
  void ref();
  void unref();

 protected:
  virtual ~TaskEventCallback();
 private:
  pthread_mutex_t _lock;
  int		  _refCounter;
};
#endif
