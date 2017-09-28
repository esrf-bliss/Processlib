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
#include <pthread.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <intrin.h>
#include <errno.h>

#pragma intrinsic(_ReadWriteBarrier)

extern void (**_pthread_key_dest)(void *);

int _pthread_concur;
pthread_once_t _pthread_tls_once = PTHREAD_ONCE_INIT;
DWORD _pthread_tls;

extern pthread_rwlock_t _pthread_key_lock;

int _pthread_once_raw(pthread_once_t *o, void (*func)(void))
{
  long state = *o;

  _ReadWriteBarrier();
	
  while (state != 1)
    {
      if (!state)
	{
	  if (!_InterlockedCompareExchange(o, 2, 0))
	    {
	      /* Success */
	      func();
				
	      /* Mark as done */
	      *o = 1;
				
	      return 0;
	    }
	}
		
      YieldProcessor();
		
      _ReadWriteBarrier();
		
      state = *o;
    }
	
  /* Done */
  return 0;
}
void pthread_tls_init(void)
{
  _pthread_tls = TlsAlloc();
	
  /* Cannot continue if out of indexes */
  if (_pthread_tls == TLS_OUT_OF_INDEXES) abort();
}

void _pthread_cleanup_dest(pthread_t t)
{
  int i, j;
	
  for (j = 0; j < PTHREAD_DESTRUCTOR_ITERATIONS; j++)
    {
      int flag = 0;
	
      for (i = 0; i < t->keymax; i++)
	{
	  void *val = t->keyval[i];
			
	  if (val)
	    {
	      pthread_rwlock_rdlock(&_pthread_key_lock);
	      if ((uintptr_t) _pthread_key_dest[i] > 1)
		{
		  /* Call destructor */
		  t->keyval[i] = NULL;
		  _pthread_key_dest[i](val);
		  flag = 1;
		}
	      pthread_rwlock_unlock(&_pthread_key_lock);
	    }
	}
	
      /* Nothing to do? */
      if (!flag) return;
    }
}

pthread_t pthread_self(void)
{
  pthread_t t;
	
  _pthread_once_raw(&_pthread_tls_once, pthread_tls_init);
	
  t = (pthread_t)TlsGetValue(_pthread_tls);
	
  /* Main thread? */
  if (!t)
    {
      t = (pthread_t)malloc(sizeof(struct _pthread_v));
		
      /* If cannot initialize main thread, then the only thing we can do is abort */
      if (!t) abort();
	
      t->ret_arg = NULL;
      t->func = NULL;
      t->clean = NULL;
      t->cancelled = 0;
      t->p_state = PTHREAD_DEFAULT_ATTR;
      t->keymax = 0;
      t->keyval = NULL;
      t->h = GetCurrentThread();
		
      /* Save for later */
      TlsSetValue(_pthread_tls, t);
		
      if (setjmp(t->jb))
	{
	  /* Make sure we free ourselves if we are detached */
	  if (!t->h) free(t);
			
	  /* Time to die */
	  _endthreadex(0);
	}
    }
	
  return t;
}

int pthread_get_concurrency(int *val)
{
  *val = _pthread_concur;
  return 0;
}

int pthread_set_concurrency(int val)
{
  _pthread_concur = val;
  return 0;
}

#define pthread_getschedparam(T, P, S) ENOTSUP
#define pthread_setschedparam(T, P, S) ENOTSUP
#define pthread_getcpuclockid(T, C) ENOTSUP

int pthread_exit(void *res)
{
  pthread_t t = pthread_self();

  t->ret_arg = res;
	
  _pthread_cleanup_dest(t);
	
  longjmp(t->jb, 1);
}

unsigned _pthread_get_state(pthread_attr_t *attr, unsigned flag)
{
  return attr->p_state & flag;
}

int _pthread_set_state(pthread_attr_t *attr, unsigned flag, unsigned val)
{
  if (~flag & val) return EINVAL;
  attr->p_state &= ~flag;
  attr->p_state |= val;
	
  return 0;
}

int pthread_attr_init(pthread_attr_t *attr)
{
  attr->p_state = PTHREAD_DEFAULT_ATTR;
  attr->stack = NULL;
  attr->s_size = 0;
  return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
  /* No need to do anything */
  return 0;
}


int pthread_attr_setdetachstate(pthread_attr_t *a, int flag)
{
  return _pthread_set_state(a, PTHREAD_CREATE_DETACHED, flag);
}

int pthread_attr_getdetachstate(pthread_attr_t *a, int *flag)
{
  *flag = _pthread_get_state(a, PTHREAD_CREATE_DETACHED);
  return 0;
}

int pthread_attr_setinheritsched(pthread_attr_t *a, int flag)
{
  return _pthread_set_state(a, PTHREAD_INHERIT_SCHED, flag);
}

int pthread_attr_getinheritsched(pthread_attr_t *a, int *flag)
{
  *flag = _pthread_get_state(a, PTHREAD_INHERIT_SCHED);
  return 0;
}

int pthread_attr_setscope(pthread_attr_t *a, int flag)
{
  return _pthread_set_state(a, PTHREAD_SCOPE_SYSTEM, flag);
}

int pthread_attr_getscope(pthread_attr_t *a, int *flag)
{
  *flag = _pthread_get_state(a, PTHREAD_SCOPE_SYSTEM);
  return 0;
}

int pthread_attr_getstackaddr(pthread_attr_t *attr, void **stack)
{
  *stack = attr->stack;
  return 0;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack)
{
  attr->stack = stack;
  return 0;
}

int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *size)
{
  *size = attr->s_size;
  return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size)
{
  attr->s_size = size;
  return 0;
}

#define pthread_attr_getguardsize(A, S) ENOTSUP
#define pthread_attr_setgaurdsize(A, S) ENOTSUP
#define pthread_attr_getschedparam(A, S) ENOTSUP
#define pthread_attr_setschedparam(A, S) ENOTSUP
#define pthread_attr_getschedpolicy(A, S) ENOTSUP
#define pthread_attr_setschedpolicy(A, S) ENOTSUP


int pthread_setcancelstate(int state, int *oldstate)
{
  pthread_t t = pthread_self();
	
  if ((state & PTHREAD_CANCEL_ENABLE) != state) return EINVAL;
  if (oldstate) *oldstate = t->p_state & PTHREAD_CANCEL_ENABLE;
  t->p_state &= ~PTHREAD_CANCEL_ENABLE;
  t->p_state |= state;
	
  return 0;
}

int pthread_setcanceltype(int type, int *oldtype)
{
  pthread_t t = pthread_self();
	
  if ((type & PTHREAD_CANCEL_ASYNCHRONOUS) != type) return EINVAL;
  if (oldtype) *oldtype = t->p_state & PTHREAD_CANCEL_ASYNCHRONOUS;
  t->p_state &= ~PTHREAD_CANCEL_ASYNCHRONOUS;
  t->p_state |= type;
	
  return 0;
}

unsigned int pthread_create_wrapper(void *args)
{
  struct _pthread_v *tv = (struct _pthread_v*)args;
	
  _pthread_once_raw(&_pthread_tls_once, pthread_tls_init);
	
  TlsSetValue(_pthread_tls, tv);
		
  if (!setjmp(tv->jb))
    {
      /* Call function and save return value */
      tv->ret_arg = tv->func(tv->ret_arg);
		
      /* Clean up destructors */
      _pthread_cleanup_dest(tv);
    }
	
  /* If we exit too early, then we can race with create */
  while (tv->h == (HANDLE) -1)
    {
      YieldProcessor();
      _ReadWriteBarrier();
    }
	
  /* Make sure we free ourselves if we are detached */
  if (!tv->h)
    {
      if(tv->keyval)
	free(tv->keyval);
      free(tv);
    }
	
  return 0;
}

int pthread_create(pthread_t *th, pthread_attr_t *attr, void *(* func)(void *), void *arg)
{
  struct _pthread_v *tv = (struct _pthread_v*)malloc(sizeof(struct _pthread_v));
  unsigned ssize = 0;
	
  if (!tv) return 1;
	
  *th = tv;
	
  /* Save data in pthread_t */
  tv->ret_arg = arg;
  tv->func = func;
  tv->clean = NULL;
  tv->cancelled = 0;
  tv->p_state = PTHREAD_DEFAULT_ATTR;
  tv->keymax = 0;
  tv->keyval = NULL;
  tv->h = (HANDLE) -1;
	
  if (attr)
    {
      tv->p_state = attr->p_state;
      ssize = unsigned int(attr->s_size);
    }
	
  /* Make sure tv->h has value of -1 */
  _ReadWriteBarrier();

  tv->h = (HANDLE) _beginthreadex(NULL, ssize,(unsigned int(__stdcall*)(void*)) pthread_create_wrapper, tv, 0, NULL);
	
  /* Failed */
  if (!tv->h) return 1;
	
	
  if (tv->p_state & PTHREAD_CREATE_DETACHED)
    {
      CloseHandle(tv->h);
      _ReadWriteBarrier();
      tv->h = 0;
    }

  return 0;
}

int pthread_join(pthread_t t, void **res)
{
  struct _pthread_v *tv = t;
	
  pthread_testcancel();
	
  WaitForSingleObject(tv->h, INFINITE);
  CloseHandle(tv->h);
	
  /* Obtain return value */
  if (res) *res = tv->ret_arg;

  if(tv->keyval)
    free(tv->keyval);
  free(tv);

  return 0;
}

int pthread_detach(pthread_t t)
{
  struct _pthread_v *tv = t;
	
  /*
   * This can't race with thread exit because
   * our call would be undefined if called on a dead thread.
   */
	
  CloseHandle(tv->h);
  _ReadWriteBarrier();
  tv->h = 0;
	
  return 0;
}
