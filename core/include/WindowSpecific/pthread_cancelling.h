#ifndef __PTHREAD_CANCELLING_H__
#define __PTHREAD_CANCELLING_H__
#include <Windows.h>

#ifdef __cplusplus
extern "C"{
#endif
  int pthread_once(pthread_once_t *o, void (*func)(void));
  void pthread_testcancel(void);
  int pthread_cancel(pthread_t t);
#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif

