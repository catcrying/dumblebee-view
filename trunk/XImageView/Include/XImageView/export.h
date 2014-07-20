#ifndef __XIMAGE_VIEW_EXPORT_H__
#define __XIMAGE_VIEW_EXPORT_H__

#ifdef XIMAGE_VIEW_EXPORTS
#define XIMAGE_VIEW_API	__declspec(dllexport)
#else
#define XIMAGE_VIEW_API	__declspec(dllimport)
#endif

#endif //__XIMAGE_VIEW_EXPORT_H__