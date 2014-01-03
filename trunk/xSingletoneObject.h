#ifndef __X_SINGLETONE_OBJECT_H__
#define __X_SINGLETONE_OBJECT_H__

#include <DebugSupport/xDebug.h>
#include <stdlib.h>

template <typename T> class CxSingleton
{
private:
    static T* m_pSingleton;
protected:
    CxSingleton()
    {
        XASSERT( !m_pSingleton );
        __int64 offset = (__int64)(T*)1 - (__int64)(CxSingleton<T>*)(T*)1;
        m_pSingleton = (T*)( (__int64)this + offset );
        XTRACE( _T("Create Singleton: %p\n"), m_pSingleton );
        ::atexit( DeleteInstance );
    }
    ~CxSingleton()
    {
        XTRACE( _T("Destroy Singleton: %p\n"), m_pSingleton );
        XASSERT( m_pSingleton );
        m_pSingleton = NULL;
    }
public:
    static T* Instance( void )
    {
        if ( !m_pSingleton )
            m_pSingleton = new T;
        XASSERT( m_pSingleton );
        return m_pSingleton;
    }
protected:
    static void DeleteInstance()
    {
        if (m_pSingleton)
            delete m_pSingleton;
    }
};

template <typename T> T* CxSingleton<T>::m_pSingleton = NULL;

#endif //__X_SINGLETONE_OBJECT_H__