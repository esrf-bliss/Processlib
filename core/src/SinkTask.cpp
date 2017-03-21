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
#include "processlib/SinkTask.h"

SinkTaskBase::SinkTaskBase() : 
  _eventCbkPt(NULL),
  _refCounter(1)
{
  pthread_mutex_init(&_lock,NULL);
}

SinkTaskBase::SinkTaskBase(const SinkTaskBase &aTask) :
  _refCounter(1)
{
  if(aTask._eventCbkPt)
    aTask._eventCbkPt->ref();
  _eventCbkPt = aTask._eventCbkPt;
  pthread_mutex_init(&_lock,NULL);
}

SinkTaskBase::~SinkTaskBase()
{
  if(_eventCbkPt)
    _eventCbkPt->unref();
  pthread_mutex_destroy(&_lock);
}

void SinkTaskBase:: setEventCallback(TaskEventCallback *aEventCbk)
{
  PoolThreadMgr::Lock aLock(&_lock);
  if(aEventCbk)
      aEventCbk->ref();
  if(_eventCbkPt)
      _eventCbkPt->unref();
  _eventCbkPt = aEventCbk;
}

void SinkTaskBase::ref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  ++_refCounter;
}

void SinkTaskBase::unref()
{
  PoolThreadMgr::Lock aLock(&_lock);
  if(!(--_refCounter))
    {
      aLock.unLock();
      delete this;
    }
}

int SinkTaskBase::getRefCounter() const
{
  PoolThreadMgr::Lock aLock(&_lock);
  return _refCounter;
}
