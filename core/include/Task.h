#include "Data.h"
#ifndef __TASK_H
#define __TASK_H

class Task
{
public:
  Task(bool aProcessingOnDataSourceFlag = true) : 
    _processingOnDataSourceFlag(aProcessingOnDataSourceFlag) {}
  virtual ~Task() {}
  //@brief ask if this task need a internal tmp buffer
  //
  //@return the size needed or 0
  virtual int needTmpBuffer(const Data *) const {return 0;}
  void setTmpBuffer(Data &aData) {_tmpBuffer.setData(aData);}
  //@brief start the processing of this Task
  virtual Data process(Data &aData) {return aData;}
  //@brief like a copy constructor
  virtual Task* copy() const {return NULL;}
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
  void setProcessingOnDataSource(bool aFlag) 
  {_processingOnDataSourceFlag = aFlag;}
protected:
  Data _tmpBuffer;
  bool _processingOnDataSourceFlag;
};
#endif
