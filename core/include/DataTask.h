#include "Data.h"
#ifndef __TASK_H
#define __TASK_H

class DataTask
{
public:
  DataTask(bool aProcessingInPlaceFlag = true) : 
    _processingInPlaceFlag(aProcessingInPlaceFlag) {}
  virtual ~DataTask() {}
  //@brief ask if this task need a internal tmp buffer
  //
  //@brief start the processing of this DataTask
  virtual Data process(Data &aData) {return aData;}
  //@brief like a copy constructor
  virtual DataTask* copy() const {return NULL;}
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
protected:
  bool _processingInPlaceFlag;
};
#endif
