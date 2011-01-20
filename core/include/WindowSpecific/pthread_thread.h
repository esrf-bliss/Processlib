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
#ifndef __PTHREAD_THREAD_H__
#define __PTHREAD_THREAD_H__
#include <Windows.h>
#include <setjmp.h>
#include "Compatibility.h"

#define PTHREAD_ONCE_INIT 0

typedef long pthread_once_t;

typedef struct _pthread_cleanup _pthread_cleanup;
struct _pthread_cleanup
{
	void (*func)(void *);
	void *arg;
	_pthread_cleanup *next;
};

struct _pthread_v
{
  void *ret_arg;
  void *(* func)(void *);
  _pthread_cleanup *clean;
  HANDLE h;
  int cancelled;
  unsigned p_state;
  int keymax;
  void **keyval;

  jmp_buf jb;
};

typedef struct _pthread_v *pthread_t;

typedef struct pthread_attr_t pthread_attr_t;
struct pthread_attr_t
{
  unsigned p_state;
  void *stack;
  size_t s_size;
};

#define PTHREAD_DEFAULT_ATTR 0

#define PTHREAD_CREATE_JOINABLE 0
#define PTHREAD_CREATE_DETACHED 0x04

#define PTHREAD_EXPLICT_SCHED 0
#define PTHREAD_INHERIT_SCHED 0x08

#define PTHREAD_SCOPE_PROCESS 0
#define PTHREAD_SCOPE_SYSTEM 0x10

#define PTHREAD_DESTRUCTOR_ITERATIONS 256

#define PTHREAD_PRIO_NONE 0

#define PTHREAD_PRIO_INHERIT 8
#define PTHREAD_PRIO_PROTECT 16
#define PTHREAD_PROCESS_SHARED 0
#define PTHREAD_PROCESS_PRIVATE 1

#ifdef __cplusplus
extern "C"{
#endif

  int _pthread_once_raw(pthread_once_t *o, void (*func)(void));

  void pthread_tls_init(void);

  void _pthread_cleanup_dest(pthread_t t);

  DLL_EXPORT pthread_t pthread_self(void);

  DLL_EXPORT int pthread_get_concurrency(int *val);

  DLL_EXPORT int pthread_set_concurrency(int val);

#define pthread_getschedparam(T, P, S) ENOTSUP
#define pthread_setschedparam(T, P, S) ENOTSUP
#define pthread_getcpuclockid(T, C) ENOTSUP

  DLL_EXPORT int pthread_exit(void *res);

  unsigned _pthread_get_state(pthread_attr_t *attr, unsigned flag);

  int _pthread_set_state(pthread_attr_t *attr, unsigned flag, unsigned val);

  DLL_EXPORT int pthread_attr_init(pthread_attr_t *attr);

  DLL_EXPORT int pthread_attr_destroy(pthread_attr_t *attr);

  DLL_EXPORT int pthread_attr_setdetachstate(pthread_attr_t *a, int flag);

  DLL_EXPORT int pthread_attr_getdetachstate(pthread_attr_t *a, int *flag);

  DLL_EXPORT int pthread_attr_setinheritsched(pthread_attr_t *a, int flag);

  DLL_EXPORT int pthread_attr_getinheritsched(pthread_attr_t *a, int *flag);

  DLL_EXPORT int pthread_attr_setscope(pthread_attr_t *a, int flag);

  DLL_EXPORT int pthread_attr_getscope(pthread_attr_t *a, int *flag);

  DLL_EXPORT int pthread_attr_getstackaddr(pthread_attr_t *attr, void **stack);

  DLL_EXPORT int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack);

  DLL_EXPORT int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *size);

  DLL_EXPORT int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size);

#define pthread_attr_getguardsize(A, S) ENOTSUP
#define pthread_attr_setgaurdsize(A, S) ENOTSUP
#define pthread_attr_getschedparam(A, S) ENOTSUP
#define pthread_attr_setschedparam(A, S) ENOTSUP
#define pthread_attr_getschedpolicy(A, S) ENOTSUP
#define pthread_attr_setschedpolicy(A, S) ENOTSUP


  DLL_EXPORT int pthread_setcancelstate(int state, int *oldstate);

  DLL_EXPORT int pthread_setcanceltype(int type, int *oldtype);

  DLL_EXPORT unsigned int pthread_create_wrapper(void *args);

  DLL_EXPORT int pthread_create(pthread_t *th, pthread_attr_t *attr, void *(* func)(void *), void *arg);

  DLL_EXPORT int pthread_join(pthread_t t, void **res);

  DLL_EXPORT int pthread_detach(pthread_t t);

#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif
