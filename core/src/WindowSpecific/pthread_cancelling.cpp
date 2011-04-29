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
#include <errno.h>
#include <intrin.h>

#define pthread_cleanup_push(F, A)\
{\
	const _pthread_cleanup _pthread_cup = {(F), (A), pthread_self()->clean};\
	_ReadWriteBarrier();\
	pthread_self()->clean = (_pthread_cleanup *) &_pthread_cup;\
	_ReadWriteBarrier()
	
/* Note that if async cancelling is used, then there is a race here */
#define pthread_cleanup_pop(E)\
	(pthread_self()->clean = _pthread_cup.next, (E?_pthread_cup.func(_pthread_cup.arg):0));}

static void _pthread_once_cleanup(pthread_once_t *o)
{
	*o = 0;
}

pthread_t pthread_self(void);

int pthread_once(pthread_once_t *o, void (*func)(void))
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
	      pthread_cleanup_push((void(*)(void*))_pthread_once_cleanup, o);
	      func();
	      pthread_cleanup_pop(0);
				
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


volatile long _pthread_cancelling;

static void _pthread_invoke_cancel(void)
{
  _pthread_cleanup *pcup;
	
  _InterlockedDecrement(&_pthread_cancelling);
	
  /* Call cancel queue */
  for (pcup = pthread_self()->clean; pcup; pcup = pcup->next)
    {
      pcup->func(pcup->arg);
    }
		
  pthread_exit(PTHREAD_CANCELED);
}

void pthread_testcancel(void)
{
  if (_pthread_cancelling)
    {
      pthread_t t = pthread_self();
		
      if (t->cancelled && (t->p_state & PTHREAD_CANCEL_ENABLE))
	{
	  _pthread_invoke_cancel();
	}
    }
}

int pthread_cancel(pthread_t t)
{
  if (t->p_state & PTHREAD_CANCEL_ASYNCHRONOUS)
    {
      /* Dangerous asynchronous cancelling */
      CONTEXT ctxt;
		
      /* Already done? */
      if (t->cancelled) return ESRCH;
		
      ctxt.ContextFlags = CONTEXT_CONTROL;
		
      SuspendThread(t->h);
      GetThreadContext(t->h, &ctxt);
#ifdef _M_X64
      ctxt.Rip = (uintptr_t) _pthread_invoke_cancel;
#else
      ctxt.Eip = (uintptr_t) _pthread_invoke_cancel;
#endif
      SetThreadContext(t->h, &ctxt);
		
      /* Also try deferred Cancelling */
      t->cancelled = 1;
	
      /* Notify everyone to look */
      _InterlockedIncrement(&_pthread_cancelling);
		
      ResumeThread(t->h);
    }
  else
    {
      /* Safe deferred Cancelling */
      t->cancelled = 1;
	
      /* Notify everyone to look */
      _InterlockedIncrement(&_pthread_cancelling);
    }
	
  return 0;
}
