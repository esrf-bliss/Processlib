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

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 t = 0;
  ULARGE_INTEGER li;
  static int tzflag;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
	li.LowPart  = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    t = li.QuadPart;       /* In 100-nanosecond intervals */
    t /= 10;                /* In microseconds */
	t -= DELTA_EPOCH_IN_MICROSECS;     /* Offset to the Epoch time */
	tv->tv_sec  = (long)(t / 1000000UL);
    tv->tv_usec = (long)(t % 1000000UL);
  }

  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _get_timezone(NULL) / 60;
    tz->tz_dsttime = _get_daylight(NULL);
  }

  return 0;
}

#ifdef __cplusplus
}       //  Assume C declarations for C++
#endif  //C++

#endif //__GETTIMEOFDAY__
