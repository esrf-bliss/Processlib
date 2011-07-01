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
#include "Data.h"
#include "SinkTaskMgr.h"
#include "TaskEventCallback.h"

#ifndef __SINKTASK_H
#define __SINKTASK_H

class DLL_EXPORT SinkTaskBase
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
  SinkTask(SinkTaskMgr<T> &aMgr) : _mgr(aMgr) {aMgr.ref();};
  SinkTask(const SinkTask &aTask) : SinkTaskBase(aTask),_mgr(aTask._mgr) 
    {
      _mgr.ref();
    }
  virtual ~SinkTask() {_mgr.unref();}
protected:
  SinkTaskMgr<T>	&_mgr;
};

#endif
