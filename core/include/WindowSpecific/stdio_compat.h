#ifndef __STDIO_COMPAT_H__
#define __STDIO_COMPAT_H__

#define snprintf sprintf_s
#define NAN NAN_func()
inline double NAN_func()
{
  unsigned long nan[2] = {0xffffffff, 0x7fffffff};
  return *(double*)nan;
}
inline double round(double a) 
{
  int returnVal;
  returnVal = int(a + 0.5);
  return (double)returnVal;
}
#endif
