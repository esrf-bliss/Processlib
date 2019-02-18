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
#include "processlib/LinkTask.h"
#include "processlib/PoolThreadMgr.h"

LinkTask::LinkTask() : _processingInPlaceFlag(true), _eventCbkPt(NULL), _refCounter(1) {}

LinkTask::LinkTask(bool aProcessingInPlaceFlag)
    : _processingInPlaceFlag(aProcessingInPlaceFlag), _eventCbkPt(NULL), _refCounter(1)
{
}

LinkTask::LinkTask(const LinkTask &aLinkTask) : _processingInPlaceFlag(aLinkTask._processingInPlaceFlag)
{
    if (aLinkTask._eventCbkPt)
        aLinkTask._eventCbkPt->ref();
    _eventCbkPt = aLinkTask._eventCbkPt;
}

LinkTask::~LinkTask()
{
    if (_eventCbkPt)
        _eventCbkPt->unref();
}
void LinkTask::setEventCallback(TaskEventCallback *aEventCbk)
{
    std::lock_guard<std::mutex> aLock(_lock);
    if (aEventCbk)
        aEventCbk->ref();
    if (_eventCbkPt)
        _eventCbkPt->unref();
    _eventCbkPt = aEventCbk;
}

void LinkTask::ref()
{
    std::lock_guard<std::mutex> aLock(_lock);
    ++_refCounter;
}

void LinkTask::unref()
{
    _lock.lock();
    if (!(--_refCounter))
    {
        _lock.unlock();
        delete this;
    }
}

int LinkTask::getRefCounter() const
{
    std::lock_guard<std::mutex> aLock(_lock);
    return _refCounter;
}
