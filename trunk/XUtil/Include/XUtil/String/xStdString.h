// xStdString.h: interface for the CxStdString class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_XSTDSTRING_H__1B0CDD31_7B7F_4776_A7AE_DBBACAFAAF37__INCLUDED_)
#define AFX_XSTDSTRING_H__1B0CDD31_7B7F_4776_A7AE_DBBACAFAAF37__INCLUDED_

#include <wtypes.h>
#include <tchar.h>
#include <XUtil/DebugSupport/xDebug.h>

#ifndef NULL
#	define NULL 0
#endif

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class XUTIL_API CxStdString
{
  public :
    // CxStdString constructor, based on a string
    CxStdString (const char * instring);

    // CxStdString empty constructor
    CxStdString ()
    {
        allocated = 0;
        cstring = NULL;
        current_length = 0;
    }

    // CxStdString copy constructor
    CxStdString (const CxStdString& copy);

    // CxStdString destructor
    ~ CxStdString ()
    {
        empty_it ();
    }

    // Convert a CxStdString into a classical char *
    const char * c_str () const
    {
        if (allocated)
            return cstring;
        return "";
    }

    // Return the length of a CxStdString
    unsigned length () const
	{
		return ( allocated ) ? current_length : 0;
	}

    // CxStdString = operator
    void operator = (const char * content);

    // = operator
    void operator = (const CxStdString & copy);

    // += operator. Maps to append
    CxStdString& operator += (const char * suffix)
    {
        append (suffix);
		return *this;
    }

    // += operator. Maps to append
    CxStdString& operator += (char single)
    {
        append (single);
		return *this;
    }

    // += operator. Maps to append
    CxStdString& operator += (CxStdString & suffix)
    {
        append (suffix);
		return *this;
    }
    bool operator == (const CxStdString & compare) const;
    bool operator < (const CxStdString & compare) const;
    bool operator > (const CxStdString & compare) const;

    // Checks if a CxStdString is empty
    BOOL empty () const
    {
        return length () ? FALSE : TRUE;
    }

    // single char extraction
    const char& at (unsigned index) const
    {
        XASSERT( index < length ());
        return cstring [index];
    }

    // find a char in a string. Return CxStdString::notfound if not found
    unsigned find (char lookup) const
    {
        return find (lookup, 0);
    }

    // find a char in a string from an offset. Return CxStdString::notfound if not found
    unsigned find (char tofind, unsigned offset) const;

    /*	Function to reserve a big amount of data when we know we'll need it. Be aware that this
		function clears the content of the CxStdString if any exists.
    */
    void reserve (unsigned size)
    {
        empty_it ();
        if (size)
        {
            allocated = size;
            cstring = new char [size];
            cstring [0] = 0;
            current_length = 0;
        }
    }

    // [] operator 
    char& operator [] (unsigned index) const
    {
        XASSERT( index < length ());
        return cstring [index];
    }

    // Error value for find primitive 
    enum {	notfound = 0xffffffff,
            npos = notfound };

    void append (const char *str, int len );

  protected :

    // The base string
    char * cstring;
    // Number of chars allocated
    unsigned allocated;
    // Current string size
    unsigned current_length;

    // New size computation. It is simplistic right now : it returns twice the amount
    // we need
    unsigned assign_new_size (unsigned minimum_to_allocate)
    {
        return minimum_to_allocate * 2;
    }

    // Internal function that clears the content of a CxStdString
    void empty_it ()
    {
        if (cstring)
            delete [] cstring;
        cstring = NULL;
        allocated = 0;
        current_length = 0;
    }

    void append (const char *suffix );

    // append function for another CxStdString
    void append (const CxStdString & suffix)
    {
        append (suffix . c_str ());
    }

    // append for a single char.
    void append (char single)
    {
        if ( cstring && current_length < (allocated-1) )
		{
			cstring[ current_length ] = single;
			++current_length;
			cstring[ current_length ] = 0;
		}
		else
		{
			char smallstr [2];
			smallstr [0] = single;
			smallstr [1] = 0;
			append (smallstr);
		}
    }

};

class CxOStream : public CxStdString
{
public :
    CxOStream () : CxStdString () {}

    // CxOStream << operator. Maps to CxStdString::append
    CxOStream & operator << (const char * in)
    {
        append (in);
        return (* this);
    }

    // CxOStream << operator. Maps to CxStdString::append
    CxOStream & operator << (const CxStdString & in)
    {
        append (in . c_str ());
        return (* this);
    }
} ;

#endif // !defined(AFX_XSTDSTRING_H__1B0CDD31_7B7F_4776_A7AE_DBBACAFAAF37__INCLUDED_)
