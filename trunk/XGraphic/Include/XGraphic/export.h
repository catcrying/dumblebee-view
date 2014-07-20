#ifndef __XGRAPHIC_EXPORT_H__
#define __XGRAPHIC_EXPORT_H__

#ifdef XGRAPHIC_EXPORTS
#define XGRAPHIC_API	__declspec(dllexport)
#else
#define XGRAPHIC_API	__declspec(dllimport)
#endif

#endif //__XGRAPHIC_EXPORT_H__