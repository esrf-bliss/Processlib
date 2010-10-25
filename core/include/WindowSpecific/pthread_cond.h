#ifndef __PTHREAD_COND__H__
#define __PTHREAD_COND__H__
#include <Windows.h>
#include <pthread_mutex.h>

typedef CONDITION_VARIABLE pthread_cond_t;
typedef int pthread_condattr_t;

#define PTHREAD_COND_INITIALIZER {0}

#ifdef __cplusplus
extern "C" {
#endif

  int pthread_cond_init(pthread_cond_t *c, pthread_condattr_t *a);
  int pthread_cond_signal(pthread_cond_t *c);
  int pthread_cond_broadcast(pthread_cond_t *c);
  int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m);
  int pthread_cond_destroy(pthread_cond_t *c);

  int pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, struct timespec *t); 
  typedef int pthread_condattr_t;
  int pthread_condattr_destroy(pthread_condattr_t *a);

#define pthread_condattr_getclock(A, C) ENOTSUP
#define pthread_condattr_setclock(A, C) ENOTSUP

  int pthread_condattr_init(pthread_condattr_t *a);

  int pthread_condattr_getpshared(pthread_condattr_t *a, int *s);
  int pthread_condattr_setpshared(pthread_condattr_t *a, int s);
#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif
