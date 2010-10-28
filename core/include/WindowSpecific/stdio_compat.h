#ifndef __STDIO_COMPAT_H__
#define __STDIO_COMPAT_H__

#ifndef snprintf
#define snprintf sprintf_s
#endif
#define NAN NAN_func()
double NAN_func();
double round(double a);

#endif
