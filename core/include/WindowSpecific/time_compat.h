#ifndef __TIME_COMPAT_H__
#define __TIME_COMPAT_H__

#include <WinSock2.h>
#include <Windows.h>
#include <time.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif



struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

#ifdef __cplusplus
extern "C"{
#endif

  int gettimeofday(struct timeval *tv, struct timezone *tz);
  
#ifdef __cplusplus
}       //  Assume C declarations for C++
#endif  //C++

#endif //__GETTIMEOFDAY__
