#include <stdio_compat.h>

double NAN_func()
{
  unsigned long nan[2] = {0xffffffff, 0x7fffffff};
  return *(double*)nan;
}

double round(double a) 
{
  int returnVal;
  returnVal = int(a + 0.5);
  return (double)returnVal;
}

