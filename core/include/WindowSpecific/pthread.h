#ifndef __PTHREAD_H__
#define __PTHREAD_H__
#define PTHREAD_CANCEL_DISABLE 0
#define PTHREAD_CANCEL_ENABLE 0x01

#define PTHREAD_CANCEL_DEFERRED 0
#define PTHREAD_CANCEL_ASYNCHRONOUS 0x02

#define PTHREAD_CANCELED ((void *) 0xDEADBEEF)
#include "stdio_compat.h"
#include "time_compat.h"
#include "pthread_mutex.h"
#include "pthread_rwlock.h"
#include "pthread_key.h"
#include "pthread_thread.h"
#include "pthread_cancelling.h"
#include "pthread_cond.h"

#endif

