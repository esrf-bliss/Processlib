#ifndef __PTHREAD_CANCELLING_H__
#define __PTHREAD_CANCELLING_H__
#include <Windows.h>
#include "Compatibility.h"

#ifdef __cplusplus
extern "C"{
#endif
  DLL_EXPORT int pthread_once(pthread_once_t *o, void (*func)(void));
  DLL_EXPORT void pthread_testcancel(void);
  DLL_EXPORT int pthread_cancel(pthread_t t);
#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif

