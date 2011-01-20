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
#ifndef __PTHREAD_RWLOCK_H__
#define __PTHREAD_RWLOCK_H__
#include <Windows.h>
#include "Compatibility.h"

#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
typedef SRWLOCK pthread_rwlock_t;
#else
struct pthread_rwlock_t
{
  HANDLE	mutex;
  HANDLE 	sema_read;
  HANDLE	sema_write;
  int 		reader_count;
  int		nb_waiting_reader;
  int		nb_waiting_writer;
};
#endif

typedef int pthread_rwlockattr_t;

#ifdef __cplusplus
extern "C"{
#endif

  DLL_EXPORT int pthread_rwlock_init(pthread_rwlock_t *l, pthread_rwlockattr_t *a);

  DLL_EXPORT int pthread_rwlock_destroy(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_rdlock(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_wrlock(pthread_rwlock_t *l);

#define PTHREAD_RWLOCK_INITIALIZER {0}

  DLL_EXPORT int pthread_rwlock_tryrdlock(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_trywrlock(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_unlock(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_timedrdlock(pthread_rwlock_t *l, const struct timespec *ts);

  DLL_EXPORT int pthread_rwlock_timedwrlock(pthread_rwlock_t *l, const struct timespec *ts);

  typedef int pthread_rwlockattr_t;

  DLL_EXPORT int pthread_rwlockattr_destroy(pthread_rwlockattr_t *a);

  DLL_EXPORT int pthread_rwlockattr_init(pthread_rwlockattr_t *a);

  DLL_EXPORT int pthread_rwlockattr_getpshared(pthread_rwlockattr_t *a,int *s);

  DLL_EXPORT int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *a,int s);

#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif
