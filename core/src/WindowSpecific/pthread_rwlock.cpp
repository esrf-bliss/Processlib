#include <pthread_rwlock.h>
#include <pthread_mutex.h>
#include <errno.h>

extern void pthread_testcancel(void);
extern unsigned long long _pthread_time_in_ms_from_timespec(const struct timespec *ts);

int pthread_rwlock_init(pthread_rwlock_t *l, pthread_rwlockattr_t *a)
{
  (void) a;
  InitializeSRWLock(l);
	
  return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *l)
{
  (void) *l;
  return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *l)
{
  pthread_testcancel();
  AcquireSRWLockShared(l);
	
  return 0;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *l)
{
  pthread_testcancel();
  AcquireSRWLockExclusive(l);
	
  return 0;
}

#define PTHREAD_RWLOCK_INITIALIZER {0}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *l)
{
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
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *l)
{
  /* Try to grab lock if it has no users */
  if (!InterlockedCompareExchangePointer((PVOID *) l, (void *)1, NULL)) return 0;
	
  return EBUSY;
}

int pthread_rwlock_unlock(pthread_rwlock_t *l)
{
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
