#include <pthread.h>
#include "Data.h"
#include "TaskEventCallback.h"

#ifndef __LINKTASK_H
#define __LINKTASK_H

class LinkTask
{
public:
  LinkTask();
  LinkTask(bool aProcessingInPlaceFlag);
  LinkTask(const LinkTask &aLinkTask);
      
  //@brief start the processing of this LinkTask
  virtual Data process(Data &aData) {return aData;}

  void setEventCallback(TaskEventCallback *aEventCbk);

  /* @brief tell the task that the destination buffer of
   * process is the same as the source.
   * @param aFlag
   * - if true the src and dest buffer will be the same, no other allocation
   * - else the destination buffer will be allocated
   *
   * @warning if the src and dest are the same buffer,
   * the task can't be process in paralelle with other
   * because it will smashed the data source
   */
  void setProcessingInPlace(bool aFlag) 
  {_processingInPlaceFlag = aFlag;}

  TaskEventCallback* getEventCallback() {return _eventCbkPt;}

  void ref();
  void unref();

  int getRefCounter() const;

protected:

  virtual ~LinkTask();

  bool _processingInPlaceFlag;
  TaskEventCallback *_eventCbkPt;

  mutable pthread_mutex_t _lock;
private:
  int		  _refCounter;
};
#endif
