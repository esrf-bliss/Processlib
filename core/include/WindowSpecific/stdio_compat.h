#ifndef __STDIO_COMPAT_H__
#define __STDIO_COMPAT_H__

#define snprintf sprintf_s
#define NAN NAN_func()
double NAN_func();
double round(double a);

#endif
