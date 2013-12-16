#ifndef __XUTIL_H__
#define __XUTIL_H__

// VC++ 6.0/EVC scope error
#if defined(_WIN32_WCE) || (_MSC_VER <= 1200)
#	define for if (0) ; else for
#endif

#ifdef _DEBUG
#	pragma comment(lib, "XUtilD.lib")
#	pragma message("Automatically linking with XUtilD.lib(Debug Static Library)")
#else
#	pragma comment(lib, "XUtil.lib")
#	pragma message("Automatically linking with XUtil.lib(Release Static Library)")
#endif

#endif // __XUTIL_H__