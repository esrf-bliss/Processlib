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
#define PTHREAD_PRIO_MULT 32
#define PTHREAD_PROCESS_SHARED 0
#define PTHREAD_PROCESS_PRIVATE 1

int _pthread_concur;
pthread_once_t _pthread_tls_once;
DWORD _pthread_tls;
static int _pthread_once_raw(pthread_once_t *o, void (*func)(void))
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
static void pthread_tls_init(void)
{
	_pthread_tls = TlsAlloc();
	
	/* Cannot continue if out of indexes */
	if (_pthread_tls == TLS_OUT_OF_INDEXES) abort();
}

static void _pthread_cleanup_dest(pthread_t t)
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

static pthread_t pthread_self(void)
{
	pthread_t t;
	
	_pthread_once_raw(&_pthread_tls_once, pthread_tls_init);
	
	t = TlsGetValue(_pthread_tls);
	
	/* Main thread? */
	if (!t)
	{
		t = malloc(sizeof(struct _pthread_v));
		
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

static int pthread_get_concurrency(int *val)
{
	*val = _pthread_concur;
	return 0;
}

static int pthread_set_concurrency(int val)
{
	_pthread_concur = val;
	return 0;
}

#define pthread_getschedparam(T, P, S) ENOTSUP
#define pthread_setschedparam(T, P, S) ENOTSUP
#define pthread_getcpuclockid(T, C) ENOTSUP

static int pthread_exit(void *res)
{
	pthread_t t = pthread_self();

	t->ret_arg = res;
	
	_pthread_cleanup_dest(t);
	
	longjmp(t->jb, 1);
}

static unsigned _pthread_get_state(pthread_attr_t *attr, unsigned flag)
{
	return attr->p_state & flag;
}

static int _pthread_set_state(pthread_attr_t *attr, unsigned flag, unsigned val)
{
	if (~flag & val) return EINVAL;
	attr->p_state &= ~flag;
	attr->p_state |= val;
	
	return 0;
}

static int pthread_attr_init(pthread_attr_t *attr)
{
	attr->p_state = PTHREAD_DEFAULT_ATTR;
	attr->stack = NULL;
	attr->s_size = 0;
	return 0;
}

static int pthread_attr_destroy(pthread_attr_t *attr)
{
	/* No need to do anything */
	return 0;
}


static int pthread_attr_setdetachstate(pthread_attr_t *a, int flag)
{
	return _pthread_set_state(a, PTHREAD_CREATE_DETACHED, flag);
}

static int pthread_attr_getdetachstate(pthread_attr_t *a, int *flag)
{
	*flag = _pthread_get_state(a, PTHREAD_CREATE_DETACHED);
	return 0;
}

static int pthread_attr_setinheritsched(pthread_attr_t *a, int flag)
{
	return _pthread_set_state(a, PTHREAD_INHERIT_SCHED, flag);
}

static int pthread_attr_getinheritsched(pthread_attr_t *a, int *flag)
{
	*flag = _pthread_get_state(a, PTHREAD_INHERIT_SCHED);
	return 0;
}

static int pthread_attr_setscope(pthread_attr_t *a, int flag)
{
	return _pthread_set_state(a, PTHREAD_SCOPE_SYSTEM, flag);
}

static int pthread_attr_getscope(pthread_attr_t *a, int *flag)
{
	*flag = _pthread_get_state(a, PTHREAD_SCOPE_SYSTEM);
	return 0;
}

static int pthread_attr_getstackaddr(pthread_attr_t *attr, void **stack)
{
	*stack = attr->stack;
	return 0;
}

static int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stack)
{
	attr->stack = stack;
	return 0;
}

static int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *size)
{
	*size = attr->s_size;
	return 0;
}

static int pthread_attr_setstacksize(pthread_attr_t *attr, size_t size)
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


static int pthread_setcancelstate(int state, int *oldstate)
{
	pthread_t t = pthread_self();
	
	if ((state & PTHREAD_CANCEL_ENABLE) != state) return EINVAL;
	if (oldstate) *oldstate = t->p_state & PTHREAD_CANCEL_ENABLE;
	t->p_state &= ~PTHREAD_CANCEL_ENABLE;
	t->p_state |= state;
	
	return 0;
}

static int pthread_setcanceltype(int type, int *oldtype)
{
	pthread_t t = pthread_self();
	
	if ((type & PTHREAD_CANCEL_ASYNCHRONOUS) != type) return EINVAL;
	if (oldtype) *oldtype = t->p_state & PTHREAD_CANCEL_ASYNCHRONOUS;
	t->p_state &= ~PTHREAD_CANCEL_ASYNCHRONOUS;
	t->p_state |= type;
	
	return 0;
}

static int pthread_create_wrapper(void *args)
{
	struct _pthread_v *tv = args;
	int i, j;
	
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
	if (!tv->h) free(tv);
	
	return 0;
}

static int pthread_create(pthread_t *th, pthread_attr_t *attr, void *(* func)(void *), void *arg)
{
	struct _pthread_v *tv = malloc(sizeof(struct _pthread_v));
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
		ssize = attr->s_size;
	}
	
	/* Make sure tv->h has value of -1 */
	_ReadWriteBarrier();

	tv->h = (HANDLE) _beginthreadex(NULL, ssize, pthread_create_wrapper, tv, 0, NULL);
	
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

static int pthread_join(pthread_t t, void **res)
{
	struct _pthread_v *tv = t;
	
	pthread_testcancel();
	
	WaitForSingleObject(tv->h, INFINITE);
	CloseHandle(tv->h);
	
	/* Obtain return value */
	if (res) *res = tv->ret_arg;
	
	free(tv);

	return 0;
}

static int pthread_detach(pthread_t t)
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

