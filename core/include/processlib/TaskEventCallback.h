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

#pragma once

#if !defined(PROCESSLIB_TASKEVENTCALLBACK_H)
#define PROCESSLIB_TASKEVENTCALLBACK_H

#include <mutex>

#include "processlib/Data.h"

class PROCESSLIB_EXPORT TaskEventCallback
{
  public:
    TaskEventCallback();
    virtual void started(Data &) {}
    virtual void finished(Data &) {}
    virtual void error(Data &, const char *) {}

    void ref();
    void unref();

  private:
    std::mutex _lock;
    int _refCounter;
};

#endif //! defined(PROCESSLIB_TASKEVENTCALLBACK_H)
