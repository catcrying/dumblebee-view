/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#ifndef __XGRAPHIC_EXPORT_H__
#define __XGRAPHIC_EXPORT_H__

#ifdef XGRAPHIC_EXPORTS
#define XGRAPHIC_API	__declspec(dllexport)
#else
#define XGRAPHIC_API	__declspec(dllimport)
#endif

#endif //__XGRAPHIC_EXPORT_H__