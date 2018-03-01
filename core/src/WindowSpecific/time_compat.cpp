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
#include <time_compat.h>

int gettimeofday(struct timeval *tv, struct compat_timezone *tz)
{
  FILETIME ft;
  unsigned __int64 t = 0;
  ULARGE_INTEGER li;
  static int tzflag;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
	li.LowPart  = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    t = li.QuadPart;       /* In 100-nanosecond intervals */
    t /= 10;                /* In microseconds */
	t -= DELTA_EPOCH_IN_MICROSECS;     /* Offset to the Epoch time */
	tv->tv_sec  = (long)(t / 1000000UL);
    tv->tv_usec = (long)(t % 1000000UL);
  }

  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _get_timezone(NULL) / 60;
    tz->tz_dsttime = _get_daylight(NULL);
  }

  return 0;
}
