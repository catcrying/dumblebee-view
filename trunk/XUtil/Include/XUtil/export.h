#ifndef __XUTIL_EXPORT_H__
#define __XUTIL_EXPORT_H__

#ifdef XUTIL_EXPORTS
#define XUTIL_API	__declspec(dllexport)
#else
#define XUTIL_API	__declspec(dllimport)
#endif

#endif //__XUTIL_EXPORT_H__