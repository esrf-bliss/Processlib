#include <pthread.h>

#ifndef MAX_INT
#define MAX_INT 0x7fffffff
#endif

DLL_EXPORT int pthread_cond_init(pthread_cond_t *c, pthread_condattr_t *a)
{
	(void) a;
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)	
	InitializeConditionVariable(c);
#else
	c->sema = CreateSemaphore(NULL,0,MAX_INT,NULL);
	c->mutex = CreateMutex(NULL,FALSE,NULL);
	c->count_waiting = 0;
#endif
	return 0;
}

int pthread_cond_signal(pthread_cond_t *c)
{
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
	WakeConditionVariable(c);
#else
	bool signalFlag = false;
	WaitForSingleObject(c->mutex,INFINITE);
	if(c->count_waiting)
	  signalFlag = true,--(c->count_waiting);

	if(signalFlag)
	  ReleaseSemaphore(c->sema,1,NULL);
	ReleaseMutex(c->mutex);

#endif
	return 0;
}

int pthread_cond_broadcast(pthread_cond_t *c)
{
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
	WakeAllConditionVariable(c);
#else
	WaitForSingleObject(c->mutex,INFINITE);
	int count = c->count_waiting;
	c->count_waiting = 0;
	if(count)
	  ReleaseSemaphore(c->sema,count,NULL);
	ReleaseMutex(c->mutex);

#endif
	return 0;
}

int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m)
{
	pthread_testcancel();
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
	SleepConditionVariableCS(c, m, INFINITE);
#else
	WaitForSingleObject(c->mutex,INFINITE);
	++(c->count_waiting);

	LeaveCriticalSection(m);

	DWORD state = SignalObjectAndWait(c->mutex,
					  c->sema,
					  INFINITE,
					  FALSE);

	EnterCriticalSection(m);
#endif
	return 0;
}

int pthread_cond_destroy(pthread_cond_t *c)
{
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
	(void) c;
#else
	CloseHandle(c->mutex);
	CloseHandle(c->sema);
#endif
	return 0;
}

int pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, struct timespec *t)
{
	unsigned long long tm = _pthread_rel_time_in_ms(t);
	
	pthread_testcancel();
	int returnState = 0;
#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
	if (!SleepConditionVariableCS(c, m, DWORD(tm))) return ETIMEDOUT;
#else
	WaitForSingleObject(c->mutex,INFINITE);
	++(c->count_waiting);
	
	LeaveCriticalSection(m);

	DWORD state = SignalObjectAndWait(c->mutex,
					  c->sema,
					  (DWORD)tm,
					  FALSE);
	/** with window, this state can't be managed properly
	    because SignalObjectAndWait relase the mutex but don't retake it atomicly
	    so the c->sema can't be increment in the mean time....
	    
	*/
	if(state == WAIT_TIMEOUT)
	  {
	    WaitForSingleObject(c->mutex,INFINITE);
	    if(c->count_waiting) // As is not atomic, need to be tested
	      --(c->count_waiting);
	    ReleaseMutex(c->mutex);
	    returnState = ETIMEDOUT;
	  }

	EnterCriticalSection(m);
#endif	
	/* We can have a spurious wakeup after the timeout */
	if (!_pthread_rel_time_in_ms(t)) return ETIMEDOUT;
	
	return returnState;
}
typedef int pthread_condattr_t;
int pthread_condattr_destroy(pthread_condattr_t *a)
{
	(void) a;
	return 0;
}

#define pthread_condattr_getclock(A, C) ENOTSUP
#define pthread_condattr_setclock(A, C) ENOTSUP

int pthread_condattr_init(pthread_condattr_t *a)
{
	*a = 0;
	return 0;
}

int pthread_condattr_getpshared(pthread_condattr_t *a, int *s)
{
	*s = *a;
	return 0;
}

int pthread_condattr_setpshared(pthread_condattr_t *a, int s)
{
	*a = s;
	return 0;
}

