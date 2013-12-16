#ifndef __X_IO_H__
#define __X_IO_H__

#if _MSC_VER > 1000
#pragma once
#endif

#define GENERIC_RDWR   (GENERIC_READ | GENERIC_WRITE)

#define _BEGIN    FILE_BEGIN
#define _CURRENT  FILE_CURRENT
#define _END      FILE_END
#define SEEK(_hf, _ofs, _wc)       ::SetFilePointer(_hf, _ofs, NULL, _wc)
#define READ(_hf, _pd, _ns, _dw)   ::ReadFile(_hf, _pd, _ns, &_dw, NULL)
#define WRITE(_hf, _pd, _ns, _dw)  ::WriteFile(_hf, _pd, _ns, &_dw, NULL)

#define GENERIC_RWMASK (GENERIC_READ | GENERIC_WRITE | GENERIC_RDWR)

#endif // __X_IO_H__