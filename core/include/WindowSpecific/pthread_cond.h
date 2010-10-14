typedef CONDITION_VARIABLE pthread_cond_t;
#define PTHREAD_COND_INITIALIZER {0}
static int pthread_cond_init(pthread_cond_t *c, pthread_condattr_t *a)
{
	(void) a;
	
	InitializeConditionVariable(c);
	return 0;
}

static int pthread_cond_signal(pthread_cond_t *c)
{
	WakeConditionVariable(c);
	return 0;
}

static int pthread_cond_broadcast(pthread_cond_t *c)
{
	WakeAllConditionVariable(c);
	return 0;
}

static int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m)
{
	pthread_testcancel();
	SleepConditionVariableCS(c, m, INFINITE);
	return 0;
}

static int pthread_cond_destroy(pthread_cond_t *c)
{
	(void) c;
	return 0;
}

static int pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, struct timespec *t)
{
	unsigned long long tm = _pthread_rel_time_in_ms(t);
	
	pthread_testcancel();
	
	if (!SleepConditionVariableCS(c, m, tm)) return ETIMEDOUT;
	
	/* We can have a spurious wakeup after the timeout */
	if (!_pthread_rel_time_in_ms(t)) return ETIMEDOUT;
	
	return 0;
}
typedef int pthread_condattr_t;
static int pthread_condattr_destroy(pthread_condattr_t *a)
{
	(void) a;
	return 0;
}

#define pthread_condattr_getclock(A, C) ENOTSUP
#define pthread_condattr_setclock(A, C) ENOTSUP

static int pthread_condattr_init(pthread_condattr_t *a)
{
	*a = 0;
	return 0;
}

static int pthread_condattr_getpshared(pthread_condattr_t *a, int *s)
{
	*s = *a;
	return 0;
}

static int pthread_condattr_setpshared(pthread_condattr_t *a, int s)
{
	*a = s;
	return 0;
}
