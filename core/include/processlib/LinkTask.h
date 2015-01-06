//###########################################################################
// This file is part of ProcessLib, a submodule of LImA project the
// Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
#include <pthread.h>
#include "Data.h"
#include "TaskEventCallback.h"

#ifndef __LINKTASK_H
#define __LINKTASK_H

class DLL_EXPORT LinkTask
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
