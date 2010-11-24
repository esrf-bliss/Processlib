#ifndef __PTHREAD_KEY_H__
#define __PTHREAD_KEY_H__

#include "Compatibility.h"

typedef long pthread_key_t;

#ifdef __cplusplus
extern "C"{
#endif

  DLL_EXPORT int pthread_key_create(pthread_key_t *key, void (* dest)(void *));

  DLL_EXPORT int pthread_key_delete(pthread_key_t key);

  DLL_EXPORT void* pthread_getspecific(pthread_key_t key);
  DLL_EXPORT int pthread_setspecific(pthread_key_t key,
				     const void *pointer);
#ifdef __cplusplus
} //  Assume C declarations for C++
#endif
#endif
