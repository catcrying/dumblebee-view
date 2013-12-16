#ifndef __X_XML_H__
#define __X_XML_H__

#if _MSC_VER > 1000
#pragma once
#endif

#include <XUtil/xFile.h>
#include <XUtil/xMemFile.h>
#include <XUtil/String/xStdString.h>

class CxXmlDocument;
class CxXmlElement;
class CxXmlComment;
class CxXmlUnknown;
class CxXmlAttribute;
class CxXmlText;
class CxXmlDeclaration;
class CxXmlParsingData;

//////////////////////////////////////////////////////////////////////////
//	Internal structure for tracking location of items 
//	in the XML file.
struct XUTIL_API CxXmlCursor
{
	CxXmlCursor()		{ Clear(); }
	void Clear()		{ row = col = -1; }

	int row;	// 0 based.
	int col;	// 0 based.
};

//////////////////////////////////////////////////////////////////////////
// Only used by Attribute::Query functions
enum 
{ 
	XXML_SUCCESS,
	XXML_NO_ATTRIBUTE,
	XXML_WRONG_TYPE
};

//////////////////////////////////////////////////////////////////////////
// Used by the parsing routines.
enum CxXmlEncoding
{
	XXML_ENCODING_UNKNOWN,
	XXML_ENCODING_UTF8,
	XXML_ENCODING_LEGACY
};

const CxXmlEncoding XXML_DEFAULT_ENCODING = XXML_ENCODING_UNKNOWN;

class XUTIL_API CxXmlBase
{
	friend class CxXmlNode;
	friend class CxXmlElement;
	friend class CxXmlDocument;

public:
	CxXmlBase()	:	userData(0) {}
	virtual ~CxXmlBase()					{}

	virtual void Print( CxArchive& cFile, int depth ) const = 0;

	static void SetCondenseWhiteSpace( BOOL condense )		{ condenseWhiteSpace = condense; }

	static BOOL IsWhiteSpaceCondensed()						{ return condenseWhiteSpace; }

	int Row() const			{ return location.row + 1; }
	int Column() const		{ return location.col + 1; }	///< See Row()

	void  SetUserData( void* user )			{ userData = user; }
	void* GetUserData()						{ return userData; }

	static const int utf8ByteTable[256];

	virtual const char* Parse(	const char* p, 
								CxXmlParsingData* data, 
								CxXmlEncoding encoding /*= XXML_ENCODING_UNKNOWN */ ) = 0;

	enum
	{
		XXML_NO_ERROR = 0,
		XXML_ERROR,
		XXML_ERROR_OPENING_FILE,
		XXML_ERROR_OUT_OF_MEMORY,
		XXML_ERROR_PARSING_ELEMENT,
		XXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
		XXML_ERROR_READING_ELEMENT_VALUE,
		XXML_ERROR_READING_ATTRIBUTES,
		XXML_ERROR_PARSING_EMPTY,
		XXML_ERROR_READING_END_TAG,
		XXML_ERROR_PARSING_UNKNOWN,
		XXML_ERROR_PARSING_COMMENT,
		XXML_ERROR_PARSING_DECLARATION,
		XXML_ERROR_DOCUMENT_EMPTY,
		XXML_ERROR_EMBEDDED_NULL,

		XXML_ERROR_STRING_COUNT
	};

protected:

	class StringToBuffer
	{
	  public:
		StringToBuffer( const CxStdString& str );
		~StringToBuffer();
		char* buffer;
	};

	static const char*	SkipWhiteSpace( const char*, CxXmlEncoding encoding );
	inline static BOOL	IsWhiteSpace( char c )		
	{ 
		return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' ); 
	}

	virtual void StreamOut (CxOStream *) const = 0;

	static const char* ReadName( const char* p, CxStdString* name, CxXmlEncoding encoding );

	static const char* ReadText(	const char* in,				// where to start
									CxStdString* text,			// the string read
									BOOL ignoreWhiteSpace,		// whether to keep the white space
									const char* endTag,			// what ends this text
									BOOL ignoreCase,			// whether to ignore case in the end tag
									CxXmlEncoding encoding );	// the current encoding

	static const char* GetEntity( const char* in, char* value, int* length, CxXmlEncoding encoding );

	inline static const char* GetChar( const char* p, char* _value, int* length, CxXmlEncoding encoding )
	{
		XASSERT( p );
		if ( encoding == XXML_ENCODING_UTF8 )
		{
			*length = utf8ByteTable[ *((unsigned char*)p) ];
			XASSERT( *length >= 0 && *length < 5 );
		}
		else
		{
			*length = 1;
		}

		if ( *length == 1 )
		{
			if ( *p == '&' )
				return GetEntity( p, _value, length, encoding );
			*_value = *p;
			return p+1;
		}
		else if ( *length )
		{
			strncpy( _value, p, *length );
			return p + (*length);
		}
		else
		{
			// Not valid text.
			return 0;
		}
	}

	static void PutString( const CxStdString& str, CxOStream* out );

	static void PutString( const CxStdString& str, CxStdString* out );

	static BOOL StringEqual(	const char* p,
								const char* endTag,
								BOOL ignoreCase,
								CxXmlEncoding encoding );

	static const char* errorString[ XXML_ERROR_STRING_COUNT ];

	CxXmlCursor location;

    /// Field containing a generic user pointer
	void*			userData;
	
	static int IsAlpha( unsigned char anyByte, CxXmlEncoding encoding );
	static int IsAlphaNum( unsigned char anyByte, CxXmlEncoding encoding );
	inline static int ToLower( int v, CxXmlEncoding encoding )
	{
		if ( encoding == XXML_ENCODING_UTF8 )
		{
			if ( v < 128 ) return tolower( v );
			return v;
		}
		else
		{
			return tolower( v );
		}
	}
	static void ConvertUTF32ToUTF8( unsigned long input, char* output, int* length );

private:
	CxXmlBase( const CxXmlBase& );				// not implemented.
	void operator=( const CxXmlBase& base );	// not allowed.

	struct Entity
	{
		const char*     str;
		unsigned int	strLength;
		char		    chr;
	};
	enum
	{
		NUM_ENTITY = 5,
		MAX_ENTITY_LENGTH = 6

	};
	static Entity entity[ NUM_ENTITY ];
	static BOOL condenseWhiteSpace;
};

class XUTIL_API CxXmlNode : public CxXmlBase
{
	friend class CxXmlDocument;
	friend class CxXmlElement;

public:
    friend CxOStream& operator<< (CxOStream& out, const CxXmlNode& base);

	enum NodeType
	{
		DOCUMENT,
		ELEMENT,
		COMMENT,
		UNKNOWN,
		TEXT,
		DECLARATION,
		TYPECOUNT
	};

	virtual ~CxXmlNode();

	const char * Value() const { return value.c_str (); }

	void SetValue(const char * _value) { value = _value;}

	void Clear();

	CxXmlNode* Parent()							{ return parent; }
	const CxXmlNode* Parent() const				{ return parent; }

	const CxXmlNode* FirstChild()	const	{ return firstChild; }		///< The first child of this node. Will be null if there are no children.
	CxXmlNode* FirstChild()					{ return firstChild; }
	const CxXmlNode* FirstChild( const char * value ) const;			///< The first child of this node with the matching 'value'. Will be null if none found.
	CxXmlNode* FirstChild( const char * value );						///< The first child of this node with the matching 'value'. Will be null if none found.

	const CxXmlNode* LastChild() const	{ return lastChild; }		/// The last child of this node. Will be null if there are no children.
	CxXmlNode* LastChild()	{ return lastChild; }
	const CxXmlNode* LastChild( const char * value ) const;			/// The last child of this node matching 'value'. Will be null if there are no children.
	CxXmlNode* LastChild( const char * value );	

	const CxXmlNode* IterateChildren( CxXmlNode* previous ) const;
	CxXmlNode* IterateChildren( CxXmlNode* previous );

	const CxXmlNode* IterateChildren( const char * value, CxXmlNode* previous ) const;
	CxXmlNode* IterateChildren( const char * value, CxXmlNode* previous );

	CxXmlNode* InsertEndChild( const CxXmlNode& addThis );

	CxXmlNode* LinkEndChild( CxXmlNode* addThis );

	CxXmlNode* InsertBeforeChild( CxXmlNode* beforeThis, const CxXmlNode& addThis );

	CxXmlNode* InsertAfterChild(  CxXmlNode* afterThis, const CxXmlNode& addThis );

	CxXmlNode* ReplaceChild( CxXmlNode* replaceThis, const CxXmlNode& withThis );

	BOOL RemoveChild( CxXmlNode* removeThis );

	const CxXmlNode* PreviousSibling() const			{ return prev; }
	CxXmlNode* PreviousSibling()						{ return prev; }

	const CxXmlNode* PreviousSibling( const char * ) const;
	CxXmlNode* PreviousSibling( const char * );

	const CxXmlNode* NextSibling() const				{ return next; }
	CxXmlNode* NextSibling()							{ return next; }

	const CxXmlNode* NextSibling( const char * ) const;
	CxXmlNode* NextSibling( const char * );

	const CxXmlElement* NextSiblingElement() const;
	CxXmlElement* NextSiblingElement();

	const CxXmlElement* NextSiblingElement( const char * ) const;
	CxXmlElement* NextSiblingElement( const char * );

	const CxXmlElement* FirstChildElement()	const;
	CxXmlElement* FirstChildElement();

	const CxXmlElement* FirstChildElement( const char * value ) const;
	CxXmlElement* FirstChildElement( const char * value );

	virtual int Type() const	{ return type; }

	const CxXmlDocument* GetDocument() const;
	CxXmlDocument* GetDocument();

	BOOL NoChildren() const						{ return !firstChild; }

	const CxXmlDocument* ToDocument()	const		{ return ( this && type == DOCUMENT ) ? (const CxXmlDocument*) this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	const CxXmlElement*  ToElement() const			{ return ( this && type == ELEMENT  ) ? (const CxXmlElement*)  this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	const CxXmlComment*  ToComment() const			{ return ( this && type == COMMENT  ) ? (const CxXmlComment*)  this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	const CxXmlUnknown*  ToUnknown() const			{ return ( this && type == UNKNOWN  ) ? (const CxXmlUnknown*)  this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	const CxXmlText*	   ToText()    const		{ return ( this && type == TEXT     ) ? (const CxXmlText*)     this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	const CxXmlDeclaration* ToDeclaration() const	{ return ( this && type == DECLARATION ) ? (const CxXmlDeclaration*) this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.

	CxXmlDocument* ToDocument()			{ return ( this && type == DOCUMENT ) ? (CxXmlDocument*) this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	CxXmlElement*  ToElement()			{ return ( this && type == ELEMENT  ) ? (CxXmlElement*)  this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	CxXmlComment*  ToComment()			{ return ( this && type == COMMENT  ) ? (CxXmlComment*)  this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	CxXmlUnknown*  ToUnknown()			{ return ( this && type == UNKNOWN  ) ? (CxXmlUnknown*)  this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	CxXmlText*	   ToText()   			{ return ( this && type == TEXT     ) ? (CxXmlText*)     this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.
	CxXmlDeclaration* ToDeclaration()	{ return ( this && type == DECLARATION ) ? (CxXmlDeclaration*) this : 0; } ///< Cast to a more defined type. Will return null not of the requested type.

	virtual CxXmlNode* Clone() const = 0;

protected:
	CxXmlNode( NodeType _type );

	void CopyTo( CxXmlNode* target ) const;

	CxXmlNode* Identify( const char* start, CxXmlEncoding encoding );

	const CxStdString& SValue() const	{ return value ; }

	CxXmlNode*		parent;
	NodeType		type;

	CxXmlNode*		firstChild;
	CxXmlNode*		lastChild;

	CxStdString	value;

	CxXmlNode*		prev;
	CxXmlNode*		next;

private:
	CxXmlNode( const CxXmlNode& );				// not implemented.
	void operator=( const CxXmlNode& base );	// not allowed.
};

//////////////////////////////////////////////////////////////////////////
// CxXmlAttribute
class XUTIL_API CxXmlAttribute : public CxXmlBase
{
	friend class CxXmlAttributeSet;

public:
	CxXmlAttribute() : CxXmlBase()
	{
		document = 0;
		prev = next = 0;
	}

	CxXmlAttribute( const char * _name, const char * _value )
	{
		name = _name;
		value = _value;
		document = 0;
		prev = next = 0;
	}

	const char*		Name()  const		{ return name.c_str (); }		///< Return the name of this attribute.
	const char*		Value() const		{ return value.c_str (); }		///< Return the value of this attribute.
	const int       IntValue() const;									///< Return the value of this attribute, converted to an integer.
	const double	DoubleValue() const;								///< Return the value of this attribute, converted to a double.

	int QueryIntValue( int* value ) const;
	int QueryDoubleValue( double* value ) const;

	void SetName( const char* _name )	{ name = _name; }				///< Set the name of this attribute.
	void SetValue( const char* _value )	{ value = _value; }				///< Set the value.

	void SetIntValue( int value );										///< Set the value from an integer.
	void SetDoubleValue( double value );								///< Set the value from a double.

	const CxXmlAttribute* Next() const;
	CxXmlAttribute* Next();
	const CxXmlAttribute* Previous() const;
	CxXmlAttribute* Previous();

	bool operator==( const CxXmlAttribute& rhs ) const { return rhs.name == name; }
	bool operator<( const CxXmlAttribute& rhs )	 const { return name < rhs.name; }
	bool operator>( const CxXmlAttribute& rhs )  const { return name > rhs.name; }

	virtual const char* Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding );

	virtual void Print( CxArchive& cfile, int depth ) const;

	virtual void StreamOut( CxOStream * out ) const;
	void SetDocument( CxXmlDocument* doc )	{ document = doc; }

private:
	CxXmlAttribute( const CxXmlAttribute& );				// not implemented.
	void operator=( const CxXmlAttribute& base );	// not allowed.

	CxXmlDocument*	document;	// A pointer back to a document, for error reporting.
	CxStdString name;
	CxStdString value;
	CxXmlAttribute*	prev;
	CxXmlAttribute*	next;
};

//////////////////////////////////////////////////////////////////////////
// CxXmlAttributeSet
class XUTIL_API CxXmlAttributeSet
{
public:
	CxXmlAttributeSet();
	~CxXmlAttributeSet();

	void Add( CxXmlAttribute* attribute );
	void Remove( CxXmlAttribute* attribute );

	const CxXmlAttribute* First()	const	{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
	CxXmlAttribute* First()					{ return ( sentinel.next == &sentinel ) ? 0 : sentinel.next; }
	const CxXmlAttribute* Last() const		{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
	CxXmlAttribute* Last()					{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }

	const CxXmlAttribute*	Find( const char * name ) const;
	CxXmlAttribute*	Find( const char * name );

private:
	CxXmlAttributeSet( const CxXmlAttributeSet& );	// not allowed
	void operator=( const CxXmlAttributeSet& );	// not allowed (as CxXmlAttribute)

	CxXmlAttribute sentinel;
};

//////////////////////////////////////////////////////////////////////////
// CxXmlElement
class XUTIL_API CxXmlElement : public CxXmlNode
{
public:
	CxXmlElement (const char * in_value);

	CxXmlElement( const CxXmlElement& );

	void operator=( const CxXmlElement& base );

	virtual ~CxXmlElement();

	const char* Attribute( const char* name ) const;

	const char* Attribute( const char* name, int* i ) const;

	const char* Attribute( const char* name, double* d ) const;

	int QueryIntAttribute( const char* name, int* value ) const;
	int QueryDoubleAttribute( const char* name, double* value ) const;
	int QueryDoubleAttribute( const char* name, float* value ) const {
		double d;
		int result = QueryDoubleAttribute( name, &d );
		*value = (float)d;
		return result;
	}

	void SetAttribute( const char* name, const char * value );

	void SetAttribute( const char * name, int value );

	void SetDoubleAttribute( const char * name, double value );

	void RemoveAttribute( const char * name );

	const CxXmlAttribute* FirstAttribute() const	{ return attributeSet.First(); }
	CxXmlAttribute* FirstAttribute() 				{ return attributeSet.First(); }
	const CxXmlAttribute* LastAttribute()	const 	{ return attributeSet.Last(); }
	CxXmlAttribute* LastAttribute()					{ return attributeSet.Last(); }

	virtual CxXmlNode* Clone() const;
	virtual void Print( CxArchive& cfile, int depth ) const;

	virtual const char* Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding );

protected:

	void CopyTo( CxXmlElement* target ) const;
	void ClearThis();

	virtual void StreamOut( CxOStream * out ) const;

	const char* ReadValue( const char* in, CxXmlParsingData* prevData, CxXmlEncoding encoding );

private:

	CxXmlAttributeSet attributeSet;
};

//////////////////////////////////////////////////////////////////////////
// CxXmlComment
class XUTIL_API CxXmlComment : public CxXmlNode
{
public:
	CxXmlComment() : CxXmlNode( CxXmlNode::COMMENT ) {}
	CxXmlComment( const CxXmlComment& );
	void operator=( const CxXmlComment& base );

	virtual ~CxXmlComment()	{}

	virtual CxXmlNode* Clone() const;
	virtual void Print( CxArchive& cfile, int depth ) const;

	virtual const char* Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding );

protected:
	void CopyTo( CxXmlComment* target ) const;

	virtual void StreamOut( CxOStream * out ) const;

private:

};


//////////////////////////////////////////////////////////////////////////
// CxXmlText
class XUTIL_API CxXmlText : public CxXmlNode
{
	friend class CxXmlElement;
public:
	CxXmlText (const char * initValue) : CxXmlNode (CxXmlNode::TEXT)
	{
		SetValue( initValue );
	}
	virtual ~CxXmlText() {}

	CxXmlText( const CxXmlText& copy ) : CxXmlNode( CxXmlNode::TEXT )	{ copy.CopyTo( this ); }
	void operator=( const CxXmlText& base )							 	{ base.CopyTo( this ); }

	virtual void Print( CxArchive& cfile, int depth ) const;

	virtual const char* Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding );

protected :
	virtual CxXmlNode* Clone() const;
	void CopyTo( CxXmlText* target ) const;

	virtual void StreamOut ( CxOStream * out ) const;
	BOOL Blank() const;

private:
};

//////////////////////////////////////////////////////////////////////////
// CxXmlDeclearation
class XUTIL_API CxXmlDeclaration : public CxXmlNode
{
public:
	CxXmlDeclaration()   : CxXmlNode( CxXmlNode::DECLARATION ) {}

	CxXmlDeclaration(	const char* _version,
						const char* _encoding,
						const char* _standalone );

	CxXmlDeclaration( const CxXmlDeclaration& copy );
	void operator=( const CxXmlDeclaration& copy );

	virtual ~CxXmlDeclaration()	{}

	const char *Version() const			{ return version.c_str (); }
	const char *Encoding() const		{ return encoding.c_str (); }
	const char *Standalone() const		{ return standalone.c_str (); }

	virtual CxXmlNode* Clone() const;
	virtual void Print( CxArchive& cfile, int depth ) const;

	virtual const char* Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding );

protected:
	void CopyTo( CxXmlDeclaration* target ) const;
	virtual void StreamOut ( CxOStream * out) const;

private:

	CxStdString version;
	CxStdString encoding;
	CxStdString standalone;
};


//////////////////////////////////////////////////////////////////////////
// CxXmlUnknown
class XUTIL_API CxXmlUnknown : public CxXmlNode
{
public:
	CxXmlUnknown() : CxXmlNode( CxXmlNode::UNKNOWN )	{}
	virtual ~CxXmlUnknown() {}

	CxXmlUnknown( const CxXmlUnknown& copy ) : CxXmlNode( CxXmlNode::UNKNOWN )		{ copy.CopyTo( this ); }
	void operator=( const CxXmlUnknown& copy )										{ copy.CopyTo( this ); }

	virtual CxXmlNode* Clone() const;
	virtual void Print( CxArchive& cfile, int depth ) const;

	virtual const char* Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding );

protected:
	void CopyTo( CxXmlUnknown* target ) const;

	virtual void StreamOut ( CxOStream * out ) const;

private:

};


//////////////////////////////////////////////////////////////////////////
// CxXmlDocument
class XUTIL_API CxXmlDocument : public CxXmlNode
{
public:
	CxXmlDocument();
	CxXmlDocument( const char * documentName );

	CxXmlDocument( const CxXmlDocument& copy );
	void operator=( const CxXmlDocument& copy );

	virtual ~CxXmlDocument() {}

	BOOL LoadFile( CxXmlEncoding encoding = XXML_DEFAULT_ENCODING );
	BOOL SaveFile() const;
	BOOL LoadFile( const char * filename, CxXmlEncoding encoding = XXML_DEFAULT_ENCODING );
	BOOL LoadFile( CxFile& archive, CxXmlEncoding encoding = XXML_DEFAULT_ENCODING );
	BOOL LoadFile( CxMemFile& archive, CxXmlEncoding encoding = XXML_DEFAULT_ENCODING );
	BOOL SaveFile( CxArchive& archive ) const;
	BOOL SaveFile( const char * filename ) const;

	virtual const char* Parse( const char* p, CxXmlParsingData* data = 0, CxXmlEncoding encoding = XXML_DEFAULT_ENCODING );

	const CxXmlElement* RootElement() const		{ return FirstChildElement(); }
	CxXmlElement* RootElement()					{ return FirstChildElement(); }

	BOOL Error() const						{ return error; }

	const char * ErrorDesc() const	{ return errorDesc.c_str (); }

	const int ErrorId()	const				{ return errorId; }
	
	int ErrorRow()	{ return errorLocation.row+1; }
	int ErrorCol()	{ return errorLocation.col+1; }	///< The column where the error occured. See ErrorRow()

	void SetTabSize( int _tabsize )		{ tabsize = _tabsize; }

	int TabSize() const	{ return tabsize; }

	void ClearError()						{	error = FALSE; 
												errorId = 0; 
												errorDesc = ""; 
												errorLocation.row = errorLocation.col = 0; 
												//errorLocation.last = 0; 
											}

	//////////////////////////////////////////////////////////////////////////
	// yaong
	void Print() const						{ /*Print( stdout, 0 );*/ }

	virtual void Print( CxArchive&, int depth = 0 ) const;
	void SetError( int err, const char* errorLocation, CxXmlParsingData* prevData, CxXmlEncoding encoding );

protected :
	virtual void StreamOut ( CxOStream * out) const;
	virtual CxXmlNode* Clone() const;

private:
	void CopyTo( CxXmlDocument* target ) const;

	BOOL error;
	int  errorId;
	CxStdString errorDesc;
	int tabsize;
	CxXmlCursor errorLocation;
};

//////////////////////////////////////////////////////////////////////////
// CxXmlHandle
class XUTIL_API CxXmlHandle
{
public:
	CxXmlHandle( CxXmlNode* node )					{ this->node = node; }
	CxXmlHandle( const CxXmlHandle& ref )			{ this->node = ref.node; }
	CxXmlHandle operator=( const CxXmlHandle& ref ) { this->node = ref.node; return *this; }

	CxXmlHandle FirstChild() const;
	CxXmlHandle FirstChild( const char * value ) const;
	CxXmlHandle FirstChildElement() const;
	CxXmlHandle FirstChildElement( const char * value ) const;

	CxXmlHandle Child( const char* value, int index ) const;
	CxXmlHandle Child( int index ) const;
	CxXmlHandle ChildElement( const char* value, int index ) const;
	CxXmlHandle ChildElement( int index ) const;

	CxXmlNode* Node() const			{ return node; } 
	CxXmlElement* Element() const	{ return ( ( node && node->ToElement() ) ? node->ToElement() : 0 ); }
	CxXmlText* Text() const			{ return ( ( node && node->ToText() ) ? node->ToText() : 0 ); }
	CxXmlUnknown* Unknown() const			{ return ( ( node && node->ToUnknown() ) ? node->ToUnknown() : 0 ); }

private:
	CxXmlNode* node;
};

#endif // __X_XML_H__