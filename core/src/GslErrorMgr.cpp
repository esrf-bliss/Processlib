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
#ifndef PROCESSLIB_WITHOUT_GSL
#include "processlib/GslErrorMgr.h"
#include "processlib/PoolThreadMgr.h"
#include <gsl/gsl_errno.h>

// Static variable
GslErrorMgr GslErrorMgr::_errorMgr;
std::mutex GslErrorMgr::_lock;

static const int MAX_ERROR_SIZE = 1024;

/** @brief just set the gsl handler
 */
GslErrorMgr::GslErrorMgr()
{
    gsl_set_error_handler(&GslErrorMgr::_error_handler);
}
/** @brief return the last thread error message or empty string ("")
 */
const char *GslErrorMgr::lastErrorMsg() const
{
    std::lock_guard<std::mutex> lock(_lock);
    auto i = _errorMessage.find(std::this_thread::get_id());
    return (i != _errorMessage.end()) ? i->second.c_str() : "";
}
/** @brief return the last errno
 */
int GslErrorMgr::lastErrno() const
{
    std::lock_guard<std::mutex> lock(_lock);
    auto i = _lastGslErrno.find(std::this_thread::get_id());
    return (i != _lastGslErrno.end()) ? i->second : 0;
}
/** @brief reset the error gsl string for this thread
 */
void GslErrorMgr::resetErrorMsg()
{
    std::lock_guard<std::mutex> lock(_lock);
    _errorMessage.insert(ErrorMessageType::value_type(std::this_thread::get_id(), ""));
    _lastGslErrno.insert(ErrnoType::value_type(std::this_thread::get_id(), 0));
}

/** @brief store gsl error into the thread's error message
 */
void GslErrorMgr::_error_handler(const char *reason, const char *file, int line, int gsl_errno)
{
    char aTmpBuffer[MAX_ERROR_SIZE];
    snprintf(aTmpBuffer, MAX_ERROR_SIZE, "GSL call failed ! : %s %s %d %d", reason, file, line, gsl_errno);

    std::lock_guard<std::mutex> lock(_lock);
    auto i = get()._errorMessage.find(std::this_thread::get_id());
    if (i != get()._errorMessage.end())
    {
        i->second += '\n';
        i->second += aTmpBuffer;
    } else
        get()._errorMessage.insert(ErrorMessageType::value_type(std::this_thread::get_id(), aTmpBuffer));
    get()._lastGslErrno.insert(ErrnoType::value_type(std::this_thread::get_id(), gsl_errno));
}
#endif // PROCESSLIB_WITHOUT_GSL
