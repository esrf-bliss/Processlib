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
#ifndef __PTHREAD_MUTEX_H__
#define __PTHREAD_MUTEX_H__

#include <Windows.h>
#include "processlib/Compatibility.h"
#include <math.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"{
#endif
  typedef CRITICAL_SECTION pthread_mutex_t;

  DLL_EXPORT int pthread_mutex_lock(pthread_mutex_t *m);

  DLL_EXPORT int pthread_mutex_unlock(pthread_mutex_t *m);
	
  DLL_EXPORT int pthread_mutex_trylock(pthread_mutex_t *m);

typedef int pthread_mutexattr_t;

  DLL_EXPORT int pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *a);

  DLL_EXPORT int pthread_mutex_destroy(pthread_mutex_t *m);

#define PTHREAD_MUTEX_INITIALIZER {(PRTL_CRITICAL_SECTION_DEBUG)-1,-1,0,0,0,0}

/* MSC 14 2015 provides this struct */
#if _MSC_VER < 1900
struct timespec
{
	/* long long in windows is the same as long in unix for 64bit */
	long long tv_sec;
	long long tv_nsec;
};
#else
#include <time.h>
#endif

  unsigned long long _pthread_time_in_ms(void);

  unsigned long long _pthread_time_in_ms_from_timespec(const struct timespec *ts);

  unsigned long long _pthread_rel_time_in_ms(const struct timespec *ts);

  DLL_EXPORT int pthread_mutex_timedlock(pthread_mutex_t *m, struct timespec *ts);

#ifndef ETIMEDOUT
#define ETIMEDOUT	110
#endif
#define PTHREAD_MUTEX_NORMAL 0
#define PTHREAD_MUTEX_ERRORCHECK 1
#define PTHREAD_MUTEX_RECURSIVE 2
#define PTHREAD_MUTEX_DEFAULT 3
#define PTHREAD_MUTEX_SHARED 4
#define PTHREAD_MUTEX_PRIVATE 0
#define PTHREAD_PRIO_MULT 32

#ifndef ENOTSUP
#define ENOTSUP		134
#endif
#define pthread_mutex_getprioceiling(M, P) ENOTSUP
#define pthread_mutex_setprioceiling(M, P) ENOTSUP

  DLL_EXPORT int pthread_mutexattr_init(pthread_mutexattr_t *a);

  DLL_EXPORT int pthread_mutexattr_destroy(pthread_mutexattr_t *a);

  DLL_EXPORT int pthread_mutexattr_gettype(const pthread_mutexattr_t *a, int *type);

  DLL_EXPORT int pthread_mutexattr_settype(pthread_mutexattr_t *a, int type);

  DLL_EXPORT int pthread_mutexattr_getpshared(pthread_mutexattr_t *a, int *type);

  DLL_EXPORT int pthread_mutexattr_setpshared(pthread_mutexattr_t * a, int type);

  DLL_EXPORT int pthread_mutexattr_getprotocol(pthread_mutexattr_t *a, int *type);

  DLL_EXPORT int pthread_mutexattr_setprotocol(pthread_mutexattr_t *a, int type);

  DLL_EXPORT int pthread_mutexattr_getprioceiling(pthread_mutexattr_t *a, int * prio);

  DLL_EXPORT int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *a, int prio);

#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif
