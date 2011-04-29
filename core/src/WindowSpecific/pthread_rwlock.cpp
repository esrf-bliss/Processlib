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

#ifndef MAX_INT
#define MAX_INT 0x7fffffff
#endif

extern unsigned long long _pthread_time_in_ms_from_timespec(const struct timespec *ts);

int pthread_rwlock_init(pthread_rwlock_t *l, pthread_rwlockattr_t *a)
{
  (void) a;
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)	
  InitializeSRWLock(l);
#else
  l->sema_read = CreateSemaphore(NULL,0,MAX_INT,NULL);
  l->sema_write = CreateSemaphore(NULL,0,MAX_INT,NULL);
  l->mutex = CreateMutex(NULL,FALSE,NULL);
  l->reader_count = 0;
  l->nb_waiting_reader = 0;
  l->nb_waiting_writer = 0;
#endif
  return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *l)
{
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)	
  (void) *l;
#else
  CloseHandle(l->mutex);
  CloseHandle(l->sema_write);
  CloseHandle(l->sema_read);
#endif
  return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *l)
{
  pthread_testcancel();
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)	
  AcquireSRWLockShared(l);
#else
  WaitForSingleObject(l->mutex,INFINITE);
  ++(l->nb_waiting_reader);
  while(l->reader_count < 0)
    {
      DWORD state = SignalObjectAndWait(l->mutex,
					l->sema_read,
					INFINITE,
					FALSE);
      WaitForSingleObject(l->mutex,INFINITE);
    }
  ++(l->reader_count);
  --(l->nb_waiting_reader);
  ReleaseMutex(l->mutex);
#endif	
  return 0;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *l)
{
  pthread_testcancel();
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
  AcquireSRWLockExclusive(l);
#else
  WaitForSingleObject(l->mutex,INFINITE);
  ++(l->nb_waiting_writer);
  while(l->reader_count != 0)
    {
      DWORD state = SignalObjectAndWait(l->mutex,
					l->sema_write,
					INFINITE,
					FALSE);
      WaitForSingleObject(l->mutex,INFINITE);
    }
  l->reader_count = -1;		// writer take exclusive lock
  --(l->nb_waiting_writer);
  ReleaseMutex(l->mutex);
#endif
  return 0;
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *l)
{
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
  /* Get the current state of the lock */
  void *state = *(void **) l;
	
  if (!state)
    {
      /* Unlocked to locked */
      if (!InterlockedCompareExchangePointer((PVOID *) l, (void *)0x11, NULL)) return 0;
      return EBUSY;
    }
	
  /* A single writer exists */
  if (state == (void *) 1) return EBUSY;
	
  /* Multiple writers exist? */
  if ((uintptr_t) state & 14) return EBUSY;
	
  if (InterlockedCompareExchangePointer((PVOID *) l, (void *) ((uintptr_t)state + 16), state) == state) return 0;
	
  return EBUSY;
#else
  int returnState = EBUSY;
  WaitForSingleObject(l->mutex,INFINITE);
  if(l->reader_count >= 0)
    returnState = 0,++(l->reader_count);
  ReleaseMutex(l->mutex);
  return returnState;
#endif
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *l)
{
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
  /* Try to grab lock if it has no users */
  if (!InterlockedCompareExchangePointer((PVOID *) l, (void *)1, NULL)) return 0;
	
  return EBUSY;
#else
  int returnState = EBUSY;
  WaitForSingleObject(l->mutex,INFINITE);
  if(l->reader_count == 0)
    returnState = 0,l->reader_count = -1;
  ReleaseMutex(l->mutex);
  return returnState;
#endif
}

int pthread_rwlock_unlock(pthread_rwlock_t *l)
{
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
  void *state = *(void **)l;
	
  if (state == (void *) 1)
    {
      /* Known to be an exclusive lock */
      ReleaseSRWLockExclusive(l);
    }
  else
    {
      /* A shared unlock will work */
      ReleaseSRWLockShared(l);
    }
#else
  WaitForSingleObject(l->mutex,INFINITE);
  if(l->reader_count < 0)	// Known to be an exclusive lock 
    {
      l->reader_count = 0;
      if(l->nb_waiting_writer)	// writter have the priority
	ReleaseSemaphore(l->sema_write,1,NULL);	// Wakeup one writer
      else if(l->nb_waiting_reader)
	ReleaseSemaphore(l->sema_read,l->nb_waiting_reader,NULL); // Wake up all readers
    }
  else if(!--(l->reader_count) && l->nb_waiting_writer)	// maybe wake up one writer
    ReleaseSemaphore(l->sema_write,1,NULL);
  ReleaseMutex(l->mutex);
#endif
  return 0;
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t *l, const struct timespec *ts)
{
  unsigned long long ct = _pthread_time_in_ms();
  unsigned long long t = _pthread_time_in_ms_from_timespec(ts);

  pthread_testcancel();
  /* Use a busy-loop */
  while (1)
    {
      /* Try to grab lock */
      if (!pthread_rwlock_tryrdlock(l)) return 0;
		
      /* Get current time */
      ct = _pthread_time_in_ms();
		
      /* Have we waited long enough? */
      if (ct > t) return ETIMEDOUT;
    }
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t *l, const struct timespec *ts)
{
  unsigned long long ct = _pthread_time_in_ms();
  unsigned long long t = _pthread_time_in_ms_from_timespec(ts);

  pthread_testcancel();
	
  /* Use a busy-loop */
  while (1)
    {
      /* Try to grab lock */
      if (!pthread_rwlock_trywrlock(l)) return 0;
		
      /* Get current time */
      ct = _pthread_time_in_ms();
		
      /* Have we waited long enough? */
      if (ct > t) return ETIMEDOUT;
    }
}
typedef int pthread_rwlockattr_t;
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *a)
{
  (void) a;
  return 0;
}

int pthread_rwlockattr_init(pthread_rwlockattr_t *a)
{
  *a = 0;
  return 0;
}

int pthread_rwlockattr_getpshared(pthread_rwlockattr_t *a, int *s)
{
  *s = *a;
  return 0;
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *a, int s)
{
  *a = s;
  return 0;
}
