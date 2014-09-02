/*
 * Author:
 *   HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Copyright (C) 2014 HyeongCheol Kim <bluewiz96@gmail.com>
 *
 * Released under GNU Lesser GPL, read the file 'COPYING' for more information
 */

#ifndef __XIMAGE_VIEW_EXPORT_H__
#define __XIMAGE_VIEW_EXPORT_H__

#ifdef XIMAGE_VIEW_EXPORTS
#define XIMAGE_VIEW_API	__declspec(dllexport)
#else
#define XIMAGE_VIEW_API	__declspec(dllimport)
#endif

#endif //__XIMAGE_VIEW_EXPORT_H__