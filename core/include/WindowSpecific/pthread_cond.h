#ifndef __PTHREAD_COND__H__
#define __PTHREAD_COND__H__
#include <Windows.h>
#include <pthread_mutex.h>
#include "Compatibility.h"

#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
typedef CONDITION_VARIABLE pthread_cond_t;
#else
struct pthread_cond_t
{
  HANDLE	mutex;
  HANDLE 	sema;
  HANDLE 	sema_signal;
  int 		count_waiting;
};
#endif

typedef int pthread_condattr_t;

#define PTHREAD_COND_INITIALIZER {0}

#ifdef __cplusplus
extern "C" {
#endif

  DLL_EXPORT int pthread_cond_init(pthread_cond_t *c, pthread_condattr_t *a);
  DLL_EXPORT int pthread_cond_signal(pthread_cond_t *c);
  DLL_EXPORT int pthread_cond_broadcast(pthread_cond_t *c);
  DLL_EXPORT int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
  DLL_EXPORT int pthread_cond_destroy(pthread_cond_t *c);

  DLL_EXPORT int pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, struct timespec *t); 
  typedef int pthread_condattr_t;
  DLL_EXPORT int pthread_condattr_destroy(pthread_condattr_t *a);

#define pthread_condattr_getclock(A, C) ENOTSUP
#define pthread_condattr_setclock(A, C) ENOTSUP

  DLL_EXPORT int pthread_condattr_init(pthread_condattr_t *a);

  DLL_EXPORT int pthread_condattr_getpshared(pthread_condattr_t *a, int *s);
  DLL_EXPORT int pthread_condattr_setpshared(pthread_condattr_t *a, int s);
#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif
