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
#ifndef __TIME_COMPAT_H__
#define __TIME_COMPAT_H__

#include <WinSock2.h>
#include <Windows.h>
#include <time.h>

#include "Compatibility.h"

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#define ctime_r(rTime,buffer) ctime_s(buffer,sizeof(buffer),rTime)
#define localtime_r(timep,result) localtime_s(result,timep)

struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

#ifdef __cplusplus
extern "C"{
#endif

  DLL_EXPORT int gettimeofday(struct timeval *tv, struct timezone *tz);
  
#ifdef __cplusplus
}       //  Assume C declarations for C++
#endif  //C++

#endif //__GETTIMEOFDAY__
