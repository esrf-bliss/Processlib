#ifndef __COMPATIBILITY_H__
#define __COMPATIBILITY_H__

#ifdef WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#endif /* __COMPATIBILITY_H__ */
