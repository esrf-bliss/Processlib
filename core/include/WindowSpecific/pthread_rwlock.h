#ifndef __PTHREAD_RWLOCK_H__
#define __PTHREAD_RWLOCK_H__
#include <Windows.h>
#include "Compatibility.h"

#if (_WIN32_WINNT >= _WIN32_WINNT_LONGHORN)
typedef SRWLOCK pthread_rwlock_t;
#else
struct pthread_rwlock_t
{
  HANDLE	mutex;
  HANDLE 	sema_read;
  HANDLE	sema_write;
  int 		reader_count;
  int		nb_waiting_reader;
  int		nb_waiting_writer;
};
#endif

typedef int pthread_rwlockattr_t;

#ifdef __cplusplus
extern "C"{
#endif

  DLL_EXPORT int pthread_rwlock_init(pthread_rwlock_t *l, pthread_rwlockattr_t *a);

  DLL_EXPORT int pthread_rwlock_destroy(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_rdlock(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_wrlock(pthread_rwlock_t *l);

#define PTHREAD_RWLOCK_INITIALIZER {0}

  DLL_EXPORT int pthread_rwlock_tryrdlock(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_trywrlock(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_unlock(pthread_rwlock_t *l);

  DLL_EXPORT int pthread_rwlock_timedrdlock(pthread_rwlock_t *l, const struct timespec *ts);

  DLL_EXPORT int pthread_rwlock_timedwrlock(pthread_rwlock_t *l, const struct timespec *ts);

  typedef int pthread_rwlockattr_t;

  DLL_EXPORT int pthread_rwlockattr_destroy(pthread_rwlockattr_t *a);

  DLL_EXPORT int pthread_rwlockattr_init(pthread_rwlockattr_t *a);

  DLL_EXPORT int pthread_rwlockattr_getpshared(pthread_rwlockattr_t *a,int *s);

  DLL_EXPORT int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *a,int s);

#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif
