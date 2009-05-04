#include "Data.h"
#include "TaskEventCallback.h"

#ifndef __LINKTASK_H
#define __LINKTASK_H

class LinkTask
{
public:
  LinkTask(bool aProcessingInPlaceFlag = true) : 
    _processingInPlaceFlag(aProcessingInPlaceFlag),_eventCbkPt(NULL) {}
  LinkTask(const LinkTask &aLinkTask) :
    _processingInPlaceFlag(aLinkTask._processingInPlaceFlag)
  {
    if(aLinkTask._eventCbkPt)
      aLinkTask._eventCbkPt->ref();
    _eventCbkPt = aLinkTask._eventCbkPt;
      
  }
      
  virtual ~LinkTask()
  {
    if(_eventCbkPt)
      _eventCbkPt->unref();
  }
  //@brief ask if this task need a internal tmp buffer
  //
  //@brief start the processing of this LinkTask
  virtual Data process(Data &aData) {return aData;}
  //@brief like a copy constructor
  virtual LinkTask* copy() const {return NULL;}

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

protected:
  bool _processingInPlaceFlag;
  TaskEventCallback *_eventCbkPt;
};
#endif
