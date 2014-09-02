/*
www.sourceforge.net/projects/tinyxml
Original code by Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "stdafx.h"
#include <XUtil/Xml/xXml.h>
#include <XUtil/String/xString.h>
#include <stdio.h>
#include <stdlib.h>
#include <atlconv.h>

//////////////////////////////////////////////////////////////////////////
// CxXmlBase
BOOL CxXmlBase::condenseWhiteSpace = TRUE;

const char* CxXmlBase::errorString[ XXML_ERROR_STRING_COUNT ] =
{
	"No error",
	"Error",
	"Failed to open file",
	"Memory allocation failed.",
	"Error parsing Element.",
	"Failed to read Element name",
	"Error reading Element value.",
	"Error reading Attributes.",
	"Error: empty tag.",
	"Error reading end tag.",
	"Error parsing Unknown.",
	"Error parsing Comment.",
	"Error parsing Declaration.",
	"Error document empty.",
	"Error null (0) or unexpected EOF found in input stream.",
};

CxXmlBase::Entity CxXmlBase::entity[ NUM_ENTITY ] = 
{
	{ "&amp;",  5, '&' },
	{ "&lt;",   4, '<' },
	{ "&gt;",   4, '>' },
	{ "&quot;", 6, '\"' },
	{ "&apos;", 6, '\'' }
};

const char XXML_UTF_LEAD_0 = (const char)0xef;
const char XXML_UTF_LEAD_1 = (const char)0xbb;
const char XXML_UTF_LEAD_2 = (const char)0xbf;

const int CxXmlBase::utf8ByteTable[256] = 
{
	//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x00
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x10
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x20
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x30
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x40
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x50
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x60
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x70	End of ASCII range
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x80 0x80 to 0xc1 invalid
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x90 
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xa0 
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xb0 
		1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xc0 0xc2 to 0xdf 2 byte
		2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xd0
		3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	// 0xe0 0xe0 to 0xef 3 byte
		4,	4,	4,	4,	4,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1	// 0xf0 0xf0 to 0xf4 4 byte, 0xf5 and higher invalid
};

void CxXmlBase::ConvertUTF32ToUTF8( unsigned long input, char* output, int* length )
{
	const unsigned long BYTE_MASK = 0xBF;
	const unsigned long BYTE_MARK = 0x80;
	const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

	if (input < 0x80) 
		*length = 1;
	else if ( input < 0x800 )
		*length = 2;
	else if ( input < 0x10000 )
		*length = 3;
	else if ( input < 0x200000 )
		*length = 4;
	else
		{ *length = 0; return; }	// This code won't covert this correctly anyway.

	output += *length;

	// Scary scary fall throughs.
	switch (*length) 
	{
		case 4:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 3:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 2:
			--output; 
			*output = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 1:
			--output; 
			*output = (char)(input | FIRST_BYTE_MARK[*length]);
	}
}


int CxXmlBase::IsAlpha( unsigned char anyByte, CxXmlEncoding )
{
	if ( anyByte < 127 )
		return isalpha( anyByte );
	else
		return 1;
}


int CxXmlBase::IsAlphaNum( unsigned char anyByte, CxXmlEncoding )
{
	if ( anyByte < 127 )
		return isalnum( anyByte );
	else
		return 1;
}

//////////////////////////////////////////////////////////////////////////
// InnerClass - CxXmlParsingData
class CxXmlParsingData
{
	friend class CxXmlDocument;
  public:
	void Stamp( const char* now, CxXmlEncoding encoding );

	const CxXmlCursor& Cursor()	{ return cursor; }

  private:
	// Only used by the document!
	CxXmlParsingData( const char* start, int _tabsize, int row, int col )
	{
		XASSERT( start );
		stamp = start;
		tabsize = _tabsize;
		cursor.row = row;
		cursor.col = col;
	}

	CxXmlCursor		cursor;
	const char*		stamp;
	int				tabsize;
};


void CxXmlParsingData::Stamp( const char* now, CxXmlEncoding encoding )
{
	XASSERT( now );

	// Do nothing if the tabsize is 0.
	if ( tabsize < 1 )
	{
		return;
	}

	// Get the current row, column.
	int row = cursor.row;
	int col = cursor.col;
	const char* p = stamp;
	XASSERT( p );

	while ( p < now )
	{
		// Code contributed by Fletcher Dunn: (modified by lee)
		switch (*p) {
			case 0:
				// We *should* never get here, but in case we do, don't
				// advance past the terminating null character, ever
				return;

			case '\r':
				// bump down to the next line
				++row;
				col = 0;				
				// Eat the character
				++p;

				// Check for \r\n sequence, and treat this as a single character
				if (*p == '\n') {
					++p;
				}
				break;

			case '\n':
				// bump down to the next line
				++row;
				col = 0;

				// Eat the character
				++p;

				// Check for \n\r sequence, and treat this as a single
				// character.  (Yes, this bizarre thing does occur still
				// on some arcane platforms...)
				if (*p == '\r') {
					++p;
				}
				break;

			case '\t':
				// Eat the character
				++p;

				// Skip to next tab stop
				col = (col / tabsize + 1) * tabsize;
				break;

			case XXML_UTF_LEAD_0:
				if ( encoding == XXML_ENCODING_UTF8 )
				{
					if ( *(p+1) && *(p+2) )
					{
						// In these cases, don't advance the column. These are
						// 0-width spaces.
						if ( *(p+1)==XXML_UTF_LEAD_1 && *(p+2)==XXML_UTF_LEAD_2 )
							p += 3;	
						else if ( *(p+1)==(char)(0xbf) && *(p+2)==(char)(0xbe) )
							p += 3;	
						else if ( *(p+1)==(char)(0xbf) && *(p+2)==(char)(0xbf) )
							p += 3;	
						else
							{ p +=3; ++col; }	// A normal character.
					}
				}
				else
				{
					++p;
					++col;
				}
				break;

			default:
				if ( encoding == XXML_ENCODING_UTF8 )
				{
					// Eat the 1 to 4 byte utf8 character.
					int step = CxXmlBase::utf8ByteTable[*((unsigned char*)p)];
					if ( step == 0 )
						step = 1;		// Error case from bad encoding, but handle gracefully.
					p += step;

					// Just advance one column, of course.
					++col;
				}
				else
				{
					++p;
					++col;
				}
				break;
		}
	}
	cursor.row = row;
	cursor.col = col;
	XASSERT( cursor.row >= -1 );
	XASSERT( cursor.col >= -1 );
	stamp = p;
	XASSERT( stamp );
}

const char* CxXmlBase::SkipWhiteSpace( const char* p, CxXmlEncoding encoding )
{
	if ( !p || !*p )
	{
		return 0;
	}
	if ( encoding == XXML_ENCODING_UTF8 )
	{
		while ( *p )
		{
			// Skip the stupid Microsoft UTF-8 Byte order marks
			if (	*(p+0)==XXML_UTF_LEAD_0
				 && *(p+1)==XXML_UTF_LEAD_1 
				 && *(p+2)==XXML_UTF_LEAD_2 )
			{
				p += 3;
				continue;
			}
			else if (*(p+0)==XXML_UTF_LEAD_0
				 && *(p+1)==(const char) 0xbf
				 && *(p+2)==(const char) 0xbe )
			{
				p += 3;
				continue;
			}
			else if (*(p+0)==XXML_UTF_LEAD_0
				 && *(p+1)==(const char) 0xbf
				 && *(p+2)==(const char) 0xbf )
			{
				p += 3;
				continue;
			}

			if ( IsWhiteSpace( *p ) || *p == '\n' || *p =='\r' )		// Still using old rules for white space.
				++p;
			else
				break;
		}
	}
	else
	{
		while ( *p && IsWhiteSpace( *p ) || *p == '\n' || *p =='\r' )
			++p;
	}

	return p;
}

const char* CxXmlBase::ReadName( const char* p, CxStdString * name, CxXmlEncoding encoding )
{
	*name = "";
	XASSERT( p );

	// Names start with letters or underscores.
	// Of course, in unicode, tinyxml has no idea what a letter *is*. The
	// algorithm is generous.
	//
	// After that, they can be letters, underscores, numbers,
	// hyphens, or colons. (Colons are valid ony for namespaces,
	// but tinyxml can't tell namespaces from names.)
	if (    p && *p 
		 && ( IsAlpha( (unsigned char) *p, encoding ) || *p == '_' ) )
	{
		while (		p && *p
				&&	(		IsAlphaNum( (unsigned char ) *p, encoding ) 
						 || *p == '_'
						 || *p == '-'
						 || *p == '.'
						 || *p == ':' ) )
		{
			(*name) += *p;
			++p;
		}
		return p;
	}
	return 0;
}

const char* CxXmlBase::GetEntity( const char* p, char* value, int* length, CxXmlEncoding encoding )
{
	// Presume an entity, and pull it out.
    CxStdString ent;
	int i;
	*length = 0;

	if ( *(p+1) && *(p+1) == '#' && *(p+2) )
	{
		unsigned long ucs = 0;
		//*ME:	warning C4244: convert '__w64 int' to 'unsigned'
		//*ME:	Use size_t instead of unsigned (pointer-arithmetic)
		size_t delta = 0;
		unsigned mult = 1;

		if ( *(p+2) == 'x' )
		{
			// Hexadecimal.
			if ( !*(p+3) ) return 0;

			const char* q = p+3;
			q = strchr( q, ';' );

			if ( !q || !*q ) return 0;

			delta = q-p;
			--q;

			while ( *q != 'x' )
			{
				if ( *q >= '0' && *q <= '9' )
					ucs += mult * (*q - '0');
				else if ( *q >= 'a' && *q <= 'f' )
					ucs += mult * (*q - 'a' + 10);
				else if ( *q >= 'A' && *q <= 'F' )
					ucs += mult * (*q - 'A' + 10 );
				else 
					return 0;
				mult *= 16;
				--q;
			}
		}
		else
		{
			// Decimal.
			if ( !*(p+2) ) return 0;

			const char* q = p+2;
			q = strchr( q, ';' );

			if ( !q || !*q ) return 0;

			delta = q-p;
			--q;

			while ( *q != '#' )
			{
				if ( *q >= '0' && *q <= '9' )
					ucs += mult * (*q - '0');
				else 
					return 0;
				mult *= 10;
				--q;
			}
		}
		if ( encoding == XXML_ENCODING_UTF8 )
		{
			// convert the UCS to UTF-8
			ConvertUTF32ToUTF8( ucs, value, length );
		}
		else
		{
			*value = (char)ucs;
			*length = 1;
		}
		return p + delta + 1;
	}

	// Now try to match it.
	for ( i=0; i<NUM_ENTITY; ++i )
	{
		if ( strncmp( entity[i].str, p, entity[i].strLength ) == 0 )
		{
			XASSERT( strlen( entity[i].str ) == entity[i].strLength );
			*value = entity[i].chr;
			*length = 1;
			return ( p + entity[i].strLength );
		}
	}

	// So it wasn't an entity, its unrecognized, or something like that.
	*value = *p;	// Don't put back the last one, since we return it!
	return p+1;
}


BOOL CxXmlBase::StringEqual( const char* p,
							 const char* tag,
							 BOOL ignoreCase,
							 CxXmlEncoding encoding )
{
	XASSERT( p );
	XASSERT( tag );
	if ( !p || !*p )
	{
		XASSERT( 0 );
		return FALSE;
	}

	const char* q = p;

	if ( ignoreCase )
	{
		while ( *q && *tag && ToLower( *q, encoding ) == ToLower( *tag, encoding ) )
		{
			++q;
			++tag;
		}

		if ( *tag == 0 )
			return TRUE;
	}
	else
	{
		while ( *q && *tag && *q == *tag )
		{
			++q;
			++tag;
		}

		if ( *tag == 0 )		// Have we found the end of the tag, and everything equal?
			return TRUE;
	}
	return FALSE;
}

const char* CxXmlBase::ReadText(	const char* p, 
									CxStdString * text, 
									BOOL trimWhiteSpace, 
									const char* endTag, 
									BOOL caseInsensitive,
									CxXmlEncoding encoding )
{
    *text = "";
	if (    !trimWhiteSpace			// certain tags always keep whitespace
		 || !condenseWhiteSpace )	// if TRUE, whitespace is always kept
	{
		// Keep all the white space.
		while (	   p && *p
				&& !StringEqual( p, endTag, caseInsensitive, encoding )
			  )
		{
			int len;
			char cArr[4] = { 0, 0, 0, 0 };
			p = GetChar( p, cArr, &len, encoding );
			text->append( cArr, len );
		}
	}
	else
	{
		BOOL whitespace = FALSE;

		// Remove leading white space:
		p = SkipWhiteSpace( p, encoding );
		while (	   p && *p
				&& !StringEqual( p, endTag, caseInsensitive, encoding ) )
		{
			if ( *p == '\r' || *p == '\n' )
			{
				whitespace = TRUE;
				++p;
			}
			else if ( IsWhiteSpace( *p ) )
			{
				whitespace = TRUE;
				++p;
			}
			else
			{
				// If we've found whitespace, add it before the
				// new character. Any whitespace just becomes a space.
				if ( whitespace )
				{
					(*text) += ' ';
					whitespace = FALSE;
				}
				int len;
				char cArr[4] = { 0, 0, 0, 0 };
				p = GetChar( p, cArr, &len, encoding );
				if ( len == 1 )
					(*text) += cArr[0];	// more efficient
				else
					text->append( cArr, len );
			}
		}
	}
	return p + strlen( endTag );
}

void CxXmlBase::PutString( const CxStdString& str, CxOStream* stream )
{
	CxStdString buffer;
	PutString( str, &buffer );
	(*stream) << buffer;
}

void CxXmlBase::PutString( const CxStdString& str, CxStdString* outString )
{
	int i=0;

	while ( i<(int)str.length() )
	{
		unsigned char c = (unsigned char) str[i];

		if (    c == '&' 
		     && i < ( (int)str.length() - 2 )
			 && str[i+1] == '#'
			 && str[i+2] == 'x' )
		{
			// Hexadecimal character reference.
			// Pass through unchanged.
			// &#xA9;	-- copyright symbol, for example.
			//
			// The -1 is a bug fix from Rob Laveaux. It keeps
			// an overflow from happening if there is no ';'.
			// There are actually 2 ways to exit this loop -
			// while fails (error case) and break (semicolon found).
			// However, there is no mechanism (currently) for
			// this function to return an error.
			while ( i<(int)str.length()-1 )
			{
				outString->append( str.c_str() + i, 1 );
				++i;
				if ( str[i] == ';' )
					break;
			}
		}
		else if ( c == '&' )
		{
			outString->append( entity[0].str, entity[0].strLength );
			++i;
		}
		else if ( c == '<' )
		{
			outString->append( entity[1].str, entity[1].strLength );
			++i;
		}
		else if ( c == '>' )
		{
			outString->append( entity[2].str, entity[2].strLength );
			++i;
		}
		else if ( c == '\"' )
		{
			outString->append( entity[3].str, entity[3].strLength );
			++i;
		}
		else if ( c == '\'' )
		{
			outString->append( entity[4].str, entity[4].strLength );
			++i;
		}
		else if ( c < 32 )
		{
			// Easy pass at non-alpha/numeric/symbol
			// Below 32 is symbolic.
			char buf[ 32 ];
			sprintf( buf, "&#x%02X;", (unsigned) ( c & 0xff ) );
			//*ME:	warning C4267: convert 'size_t' to 'int'
			//*ME:	Int-Cast to make compiler happy ...
			outString->append( buf, (int)strlen( buf ) );
			++i;
		}
		else
		{
			//char realc = (char) c;
			//outString->append( &realc, 1 );
			*outString += (char) c;	// somewhat more efficient function call.
			++i;
		}
	}
}


// <-- Strange class for a bug fix. Search for STL_STRING_BUG
CxXmlBase::StringToBuffer::StringToBuffer( const CxStdString& str )
{
	buffer = new char[ str.length()+1 ];
	if ( buffer )
	{
		strcpy( buffer, str.c_str() );
	}
}


CxXmlBase::StringToBuffer::~StringToBuffer()
{
	delete [] buffer;
}
// End strange bug fix. -->


//////////////////////////////////////////////////////////////////////////
// CxXmlNode
CxXmlNode::CxXmlNode( NodeType _type ) : CxXmlBase()
{
	parent = 0;
	type = _type;
	firstChild = 0;
	lastChild = 0;
	prev = 0;
	next = 0;
}

CxXmlNode::~CxXmlNode()
{
	CxXmlNode* node = firstChild;
	CxXmlNode* temp = 0;

	while ( node )
	{
		temp = node;
		node = node->next;
		delete temp;
	}	
}

CxXmlNode* CxXmlNode::Identify( const char* p, CxXmlEncoding encoding )
{
	CxXmlNode* returnNode = 0;

	p = SkipWhiteSpace( p, encoding );
	if ( !p || !*p || *p != '<' )
	{
		return 0;
	}

	CxXmlDocument* doc = GetDocument();
	p = SkipWhiteSpace( p, encoding );

	if ( !p || !*p )
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start with a letter or underscore, but xml is reserved.
	// - Comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown to tinyxml.
	//

	const char* xmlHeader = { "<?xml" };
	const char* commentHeader = { "<!--" };
	const char* dtdHeader = { "<!" };

	if ( StringEqual( p, xmlHeader, TRUE, encoding ) )
	{
		//XTRACE( "XML parsing Declaration\n" );
		returnNode = new CxXmlDeclaration();
	}
	else if ( StringEqual( p, commentHeader, FALSE, encoding ) )
	{
		//XTRACE( "XML parsing Comment\n" );
		returnNode = new CxXmlComment();
	}
	else if ( StringEqual( p, dtdHeader, FALSE, encoding ) )
	{
		//XTRACE( "XML parsing Unknown(1)\n" );
		returnNode = new CxXmlUnknown();
	}
	else if (    IsAlpha( *(p+1), encoding )
			  || *(p+1) == '_' )
	{
		//XTRACE( "XML parsing Element\n" );
		returnNode = new CxXmlElement( "" );
	}
	else
	{
		//XTRACE( "XML parsing Unknown(2)\n" );
		returnNode = new CxXmlUnknown();
	}

	if ( returnNode )
	{
		// Set the parent, so it can report errors
		returnNode->parent = this;
	}
	else
	{
		if ( doc )
			doc->SetError( XXML_ERROR_OUT_OF_MEMORY, 0, 0, XXML_ENCODING_UNKNOWN );
	}
	return returnNode;
}

void CxXmlNode::CopyTo( CxXmlNode* target ) const
{
	target->SetValue (value.c_str() );
	target->userData = userData; 
}


void CxXmlNode::Clear()
{
	CxXmlNode* node = firstChild;
	CxXmlNode* temp = 0;

	while ( node )
	{
		temp = node;
		node = node->next;
		delete temp;
	}	

	firstChild = 0;
	lastChild = 0;
}


CxXmlNode* CxXmlNode::LinkEndChild( CxXmlNode* node )
{
	node->parent = this;

	node->prev = lastChild;
	node->next = 0;

	if ( lastChild )
		lastChild->next = node;
	else
		firstChild = node;			// it was an empty list.

	lastChild = node;
	return node;
}


CxXmlNode* CxXmlNode::InsertEndChild( const CxXmlNode& addThis )
{
	CxXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;

	return LinkEndChild( node );
}


CxXmlNode* CxXmlNode::InsertBeforeChild( CxXmlNode* beforeThis, const CxXmlNode& addThis )
{	
	if ( !beforeThis || beforeThis->parent != this )
		return 0;

	CxXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;
	node->parent = this;

	node->next = beforeThis;
	node->prev = beforeThis->prev;
	if ( beforeThis->prev )
	{
		beforeThis->prev->next = node;
	}
	else
	{
		XASSERT( firstChild == beforeThis );
		firstChild = node;
	}
	beforeThis->prev = node;
	return node;
}


CxXmlNode* CxXmlNode::InsertAfterChild( CxXmlNode* afterThis, const CxXmlNode& addThis )
{
	if ( !afterThis || afterThis->parent != this )
		return 0;

	CxXmlNode* node = addThis.Clone();
	if ( !node )
		return 0;
	node->parent = this;

	node->prev = afterThis;
	node->next = afterThis->next;
	if ( afterThis->next )
	{
		afterThis->next->prev = node;
	}
	else
	{
		XASSERT( lastChild == afterThis );
		lastChild = node;
	}
	afterThis->next = node;
	return node;
}


CxXmlNode* CxXmlNode::ReplaceChild( CxXmlNode* replaceThis, const CxXmlNode& withThis )
{
	if ( replaceThis->parent != this )
		return 0;

	CxXmlNode* node = withThis.Clone();
	if ( !node )
		return 0;

	node->next = replaceThis->next;
	node->prev = replaceThis->prev;

	if ( replaceThis->next )
		replaceThis->next->prev = node;
	else
		lastChild = node;

	if ( replaceThis->prev )
		replaceThis->prev->next = node;
	else
		firstChild = node;

	delete replaceThis;
	node->parent = this;
	return node;
}


BOOL CxXmlNode::RemoveChild( CxXmlNode* removeThis )
{
	if ( removeThis->parent != this )
	{	
		XASSERT( 0 );
		return FALSE;
	}

	if ( removeThis->next )
		removeThis->next->prev = removeThis->prev;
	else
		lastChild = removeThis->prev;

	if ( removeThis->prev )
		removeThis->prev->next = removeThis->next;
	else
		firstChild = removeThis->next;

	delete removeThis;
	return TRUE;
}

const CxXmlNode* CxXmlNode::FirstChild( const char * _value ) const
{
	const CxXmlNode* node;
	for ( node = firstChild; node; node = node->next )
	{
		if ( node->SValue() == CxStdString( _value ))
			return node;
	}
	return 0;
}


CxXmlNode* CxXmlNode::FirstChild( const char * _value )
{
	CxXmlNode* node;
	for ( node = firstChild; node; node = node->next )
	{
		if ( node->SValue() == CxStdString( _value ))
			return node;
	}
	return 0;
}


const CxXmlNode* CxXmlNode::LastChild( const char * _value ) const
{
	const CxXmlNode* node;
	for ( node = lastChild; node; node = node->prev )
	{
		if ( node->SValue() == CxStdString (_value))
			return node;
	}
	return 0;
}

CxXmlNode* CxXmlNode::LastChild( const char * _value )
{
	CxXmlNode* node;
	for ( node = lastChild; node; node = node->prev )
	{
		if ( node->SValue() == CxStdString (_value))
			return node;
	}
	return 0;
}

const CxXmlNode* CxXmlNode::IterateChildren( CxXmlNode* previous ) const
{
	if ( !previous )
	{
		return FirstChild();
	}
	else
	{
		XASSERT( previous->parent == this );
		return previous->NextSibling();
	}
}

CxXmlNode* CxXmlNode::IterateChildren( CxXmlNode* previous )
{
	if ( !previous )
	{
		return FirstChild();
	}
	else
	{
		XASSERT( previous->parent == this );
		return previous->NextSibling();
	}
}

const CxXmlNode* CxXmlNode::IterateChildren( const char * val, CxXmlNode* previous ) const
{
	if ( !previous )
	{
		return FirstChild( val );
	}
	else
	{
		XASSERT( previous->parent == this );
		return previous->NextSibling( val );
	}
}

CxXmlNode* CxXmlNode::IterateChildren( const char * val, CxXmlNode* previous )
{
	if ( !previous )
	{
		return FirstChild( val );
	}
	else
	{
		XASSERT( previous->parent == this );
		return previous->NextSibling( val );
	}
}

const CxXmlNode* CxXmlNode::NextSibling( const char * _value ) const 
{
	const CxXmlNode* node;
	for ( node = next; node; node = node->next )
	{
		if ( node->SValue() == CxStdString (_value))
			return node;
	}
	return 0;
}

CxXmlNode* CxXmlNode::NextSibling( const char * _value )
{
	CxXmlNode* node;
	for ( node = next; node; node = node->next )
	{
		if ( node->SValue() == CxStdString (_value))
			return node;
	}
	return 0;
}

const CxXmlNode* CxXmlNode::PreviousSibling( const char * _value ) const
{
	const CxXmlNode* node;
	for ( node = prev; node; node = node->prev )
	{
		if ( node->SValue() == CxStdString (_value))
			return node;
	}
	return 0;
}

CxXmlNode* CxXmlNode::PreviousSibling( const char * _value )
{
	CxXmlNode* node;
	for ( node = prev; node; node = node->prev )
	{
		if ( node->SValue() == CxStdString (_value))
			return node;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CxXmlElement
void CxXmlElement::RemoveAttribute( const char * name )
{
	CxXmlAttribute* node = attributeSet.Find( name );
	if ( node )
	{
		attributeSet.Remove( node );
		delete node;
	}
}

const char* CxXmlElement::Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding )
{
	p = SkipWhiteSpace( p, encoding );
	CxXmlDocument* document = GetDocument();

	if ( !p || !*p )
	{
		if ( document ) document->SetError( XXML_ERROR_PARSING_ELEMENT, 0, 0, encoding );
		return 0;
	}

//	CxXmlParsingData data( p, prevData );
	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}

	if ( *p != '<' )
	{
		if ( document ) document->SetError( XXML_ERROR_PARSING_ELEMENT, p, data, encoding );
		return 0;
	}

	p = SkipWhiteSpace( p+1, encoding );

	// Read the name.
	const char* pErr = p;

    p = ReadName( p, &value, encoding );
	if ( !p || !*p )
	{
		if ( document )	document->SetError( XXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, encoding );
		return 0;
	}

    CxStdString endTag ("</");
	endTag += value;
	endTag += ">";

	// Check for and read attributes. Also look for an empty
	// tag or an end tag.
	while ( p && *p )
	{
		pErr = p;
		p = SkipWhiteSpace( p, encoding );
		if ( !p || !*p )
		{
			if ( document ) document->SetError( XXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding );
			return 0;
		}
		if ( *p == '/' )
		{
			++p;
			// Empty tag.
			if ( *p  != '>' )
			{
				if ( document ) document->SetError( XXML_ERROR_PARSING_EMPTY, p, data, encoding );		
				return 0;
			}
			return (p+1);
		}
		else if ( *p == '>' )
		{
			// Done with attributes (if there were any.)
			// Read the value -- which can include other
			// elements -- read the end tag, and return.
			++p;
			p = ReadValue( p, data, encoding );		// Note this is an Element method, and will set the error if one happens.
			if ( !p || !*p )
				return 0;

			// We should find the end tag now
			if ( StringEqual( p, endTag.c_str(), FALSE, encoding ) )
			{
				p += endTag.length();
				return p;
			}
			else
			{
				if ( document ) document->SetError( XXML_ERROR_READING_END_TAG, p, data, encoding );
				return 0;
			}
		}
		else
		{
			// Try to read an attribute:
			CxXmlAttribute* attrib = new CxXmlAttribute();
			if ( !attrib )
			{
				if ( document ) document->SetError( XXML_ERROR_OUT_OF_MEMORY, pErr, data, encoding );
				return 0;
			}

			attrib->SetDocument( document );
			const char* pErr = p;
			p = attrib->Parse( p, data, encoding );

			if ( !p || !*p )
			{
				if ( document ) document->SetError( XXML_ERROR_PARSING_ELEMENT, pErr, data, encoding );
				delete attrib;
				return 0;
			}

			// Handle the strange case of double attributes:
			CxXmlAttribute* node = attributeSet.Find( attrib->Name() );
			if ( node )
			{
				node->SetValue( attrib->Value() );
				delete attrib;
				return 0;
			}

			attributeSet.Add( attrib );
		}
	}
	return p;
}


const char* CxXmlElement::ReadValue( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding )
{
	CxXmlDocument* document = GetDocument();

	const char* pWithWhiteSpace = p;
	// Read in text and elements in any order.
	p = SkipWhiteSpace( p, encoding );
	while ( p && *p )
	{
		if ( *p != '<' )
		{
			// Take what we have, make a text element.
			CxXmlText* textNode = new CxXmlText( "" );

			if ( !textNode )
			{
				if ( document ) document->SetError( XXML_ERROR_OUT_OF_MEMORY, 0, 0, encoding );
				    return 0;
			}

			if ( CxXmlBase::IsWhiteSpaceCondensed() )
			{
				p = textNode->Parse( p, data, encoding );
			}
			else
			{
				// Special case: we want to keep the white space
				// so that leading spaces aren't removed.
				p = textNode->Parse( pWithWhiteSpace, data, encoding );
			}

			if ( !textNode->Blank() )
				LinkEndChild( textNode );
			else
				delete textNode;
		} 
		else 
		{
			// We hit a '<'
			// Have we hit a new element or an end tag?
			if ( StringEqual( p, "</", FALSE, encoding ) )
			{
				return p;
			}
			else
			{
				CxXmlNode* node = Identify( p, encoding );
				if ( node )
				{
					p = node->Parse( p, data, encoding );
					LinkEndChild( node );
				}				
				else
				{
					return 0;
				}
			}
		}
		p = SkipWhiteSpace( p, encoding );
	}

	if ( !p )
	{
		if ( document ) document->SetError( XXML_ERROR_READING_ELEMENT_VALUE, 0, 0, encoding );
	}	
	return p;
}

const CxXmlElement* CxXmlNode::FirstChildElement() const
{
	const CxXmlNode* node;

	for (	node = FirstChild();
			node;
			node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}

CxXmlElement* CxXmlNode::FirstChildElement()
{
	CxXmlNode* node;

	for (	node = FirstChild();
			node;
			node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}

const CxXmlElement* CxXmlNode::FirstChildElement( const char * _value ) const
{
	const CxXmlNode* node;

	for (	node = FirstChild( _value );
			node;
			node = node->NextSibling( _value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}

CxXmlElement* CxXmlNode::FirstChildElement( const char * _value )
{
	CxXmlNode* node;

	for (	node = FirstChild( _value );
			node;
			node = node->NextSibling( _value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}

const CxXmlElement* CxXmlNode::NextSiblingElement() const
{
	const CxXmlNode* node;

	for (	node = NextSibling();
	node;
	node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}

CxXmlElement* CxXmlNode::NextSiblingElement()
{
	CxXmlNode* node;

	for (	node = NextSibling();
	node;
	node = node->NextSibling() )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}

const CxXmlElement* CxXmlNode::NextSiblingElement( const char * _value ) const
{
	const CxXmlNode* node;

	for (	node = NextSibling( _value );
	node;
	node = node->NextSibling( _value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}

CxXmlElement* CxXmlNode::NextSiblingElement( const char * _value )
{
	CxXmlNode* node;

	for (	node = NextSibling( _value );
	node;
	node = node->NextSibling( _value ) )
	{
		if ( node->ToElement() )
			return node->ToElement();
	}
	return 0;
}


const CxXmlDocument* CxXmlNode::GetDocument() const
{
	const CxXmlNode* node;

	for ( node = this; node; node = node->parent )
	{
		if ( node->ToDocument() )
			return node->ToDocument();
	}
	return 0;
}

CxXmlDocument* CxXmlNode::GetDocument()
{
	CxXmlNode* node;

	for ( node = this; node; node = node->parent )
	{
		if ( node->ToDocument() )
			return node->ToDocument();
	}
	return 0;
}

CxXmlElement::CxXmlElement (const char * _value)
	: CxXmlNode( CxXmlNode::ELEMENT )
{
	firstChild = lastChild = 0;
	value = _value;
}

CxXmlElement::CxXmlElement( const CxXmlElement& copy)
	: CxXmlNode( CxXmlNode::ELEMENT )
{
	firstChild = lastChild = 0;
	copy.CopyTo( this );	
}


void CxXmlElement::operator=( const CxXmlElement& base )
{
	ClearThis();
	base.CopyTo( this );
}


CxXmlElement::~CxXmlElement()
{
	ClearThis();
}


void CxXmlElement::ClearThis()
{
	Clear();
	while ( attributeSet.First() )
	{
		CxXmlAttribute* node = attributeSet.First();
		attributeSet.Remove( node );
		delete node;
	}
}


const char * CxXmlElement::Attribute( const char * name ) const
{
	const CxXmlAttribute* node = attributeSet.Find( name );

	if ( node )
		return node->Value();

	return 0;
}


const char * CxXmlElement::Attribute( const char * name, int* i ) const
{
	const char * s = Attribute( name );
	if ( i )
	{
		if ( s )
			*i = atoi( s );
		else
			*i = 0;
	}
	return s;
}


const char * CxXmlElement::Attribute( const char * name, double* d ) const
{
	const char * s = Attribute( name );
	if ( d )
	{
		if ( s )
			*d = atof( s );
		else
			*d = 0;
	}
	return s;
}


int CxXmlElement::QueryIntAttribute( const char* name, int* ival ) const
{
	const CxXmlAttribute* node = attributeSet.Find( name );
	if ( !node )
		return XXML_NO_ATTRIBUTE;

	return node->QueryIntValue( ival );
}


int CxXmlElement::QueryDoubleAttribute( const char* name, double* dval ) const
{
	const CxXmlAttribute* node = attributeSet.Find( name );
	if ( !node )
		return XXML_NO_ATTRIBUTE;

	return node->QueryDoubleValue( dval );
}


void CxXmlElement::SetAttribute( const char * name, int val )
{	
	char buf[64];
	sprintf( buf, "%d", val );
	SetAttribute( name, buf );
}


void CxXmlElement::SetDoubleAttribute( const char * name, double val )
{	
	char buf[128];
	sprintf( buf, "%f", val );
	SetAttribute( name, buf );
}


void CxXmlElement::SetAttribute( const char * name, const char * _value )
{
	CxXmlAttribute* node = attributeSet.Find( name );
	if ( node )
	{
		node->SetValue( _value );
		return;
	}

	CxXmlAttribute* attrib = new CxXmlAttribute( name, _value );
	if ( attrib )
	{
		attributeSet.Add( attrib );
	}
	else
	{
		CxXmlDocument* document = GetDocument();
		if ( document ) document->SetError( XXML_ERROR_OUT_OF_MEMORY, 0, 0, XXML_ENCODING_UNKNOWN );
	}
}

void CxXmlElement::Print( CxArchive& cfile, int depth ) const
{
	int i;
	CxString strTemp;
	strTemp = _T("    ");
	for ( i=0; i<depth; i++ )
	{
		//////////////////////////////////////////////////////////////////////////
		// yaong                                                                //
		//////////////////////////////////////////////////////////////////////////
		cfile.Write( strTemp, strTemp.GetLength() );
	}

	USES_CONVERSION;

	strTemp.Format( _T("<%s"), A2T((LPSTR)value.c_str()) );
	cfile.Write( strTemp, strTemp.GetLength() );

	strTemp = _T(" ");
	const CxXmlAttribute* attrib;
	for ( attrib = attributeSet.First(); attrib; attrib = attrib->Next() )
	{
		cfile.Write( strTemp, strTemp.GetLength() );
		attrib->Print( cfile, depth );
	}

	// There are 3 different formatting approaches:
	// 1) An element without children is printed as a <foo /> node
	// 2) An element with only a text child is printed as <foo> text </foo>
	// 3) An element with children is printed on multiple lines.
	CxXmlNode* node;
	if ( !firstChild )
	{
		strTemp = _T(" />");
		cfile.Write( strTemp, strTemp.GetLength() );
	}
	else if ( firstChild == lastChild && firstChild->ToText() )
	{
		strTemp = _T(">");
		cfile.Write( strTemp, strTemp.GetLength() );
		firstChild->Print( cfile, depth + 1 );
		strTemp.Format( _T("</%s>"), A2T((LPSTR)value.c_str()) );
		cfile.Write( strTemp, strTemp.GetLength() );
	}
	else
	{
		strTemp = _T(">");
		cfile.Write( strTemp, strTemp.GetLength() );

		for ( node = firstChild; node; node=node->NextSibling() )
		{
			if ( !node->ToText() )
			{
				strTemp = _T("\r\n");
				cfile.Write( strTemp, strTemp.GetLength() );
			}
			node->Print( cfile, depth+1 );
		}
		strTemp = _T("\r\n");
		cfile.Write( strTemp, strTemp.GetLength() );
		for ( i=0; i<depth; ++i )
		{
			strTemp = _T("    ");
			cfile.Write( strTemp, strTemp.GetLength() );
		}
		strTemp.Format( _T("</%s>"), A2T((LPSTR)value.c_str()) );
		cfile.Write( strTemp, strTemp.GetLength() );
	}
}

void CxXmlElement::StreamOut( CxOStream * stream ) const
{
	(*stream) << "<" << value;

	const CxXmlAttribute* attrib;
	for ( attrib = attributeSet.First(); attrib; attrib = attrib->Next() )
	{	
		(*stream) << " ";
		attrib->StreamOut( stream );
	}

	// If this node has children, give it a closing tag. Else
	// make it an empty tag.
	CxXmlNode* node;
	if ( firstChild )
	{ 		
		(*stream) << ">";

		for ( node = firstChild; node; node=node->NextSibling() )
		{
			node->StreamOut( stream );
		}
		(*stream) << "</" << value << ">";
	}
	else
	{
		(*stream) << " />";
	}
}


void CxXmlElement::CopyTo( CxXmlElement* target ) const
{
	// superclass:
	CxXmlNode::CopyTo( target );

	// Element class: 
	// Clone the attributes, then clone the children.
	const CxXmlAttribute* attribute = 0;
	for (	attribute = attributeSet.First();
	attribute;
	attribute = attribute->Next() )
	{
		target->SetAttribute( attribute->Name(), attribute->Value() );
	}

	CxXmlNode* node = 0;
	for ( node = firstChild; node; node = node->NextSibling() )
	{
		target->LinkEndChild( node->Clone() );
	}
}


CxXmlNode* CxXmlElement::Clone() const
{
	CxXmlElement* clone = new CxXmlElement( Value() );
	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}

//////////////////////////////////////////////////////////////////////////
// CxXmlDocument
CxXmlDocument::CxXmlDocument() : CxXmlNode( CxXmlNode::DOCUMENT )
{
	tabsize = 4;
	ClearError();
}

CxXmlDocument::CxXmlDocument( const char * documentName ) : CxXmlNode( CxXmlNode::DOCUMENT )
{
	tabsize = 4;
	value = documentName;
	ClearError();
}

CxXmlDocument::CxXmlDocument( const CxXmlDocument& copy ) : CxXmlNode( CxXmlNode::DOCUMENT )
{
	copy.CopyTo( this );
}


void CxXmlDocument::operator=( const CxXmlDocument& copy )
{
	Clear();
	copy.CopyTo( this );
}

const char* CxXmlDocument::Parse( const char* p, CxXmlParsingData* prevData, CxXmlEncoding encoding )
{
	ClearError();

	// Parse away, at the document level. Since a document
	// contains nothing but other tags, most of what happens
	// here is skipping white space.
	if ( !p || !*p )
	{
		SetError( XXML_ERROR_DOCUMENT_EMPTY, 0, 0, XXML_ENCODING_UNKNOWN );
		return 0;
	}

	// Note that, for a document, this needs to come
	// before the while space skip, so that parsing
	// starts from the pointer we are given.
	location.Clear();
	if ( prevData )
	{
		location.row = prevData->cursor.row;
		location.col = prevData->cursor.col;
	}
	else
	{
		location.row = 0;
		location.col = 0;
	}
	CxXmlParsingData data( p, TabSize(), location.row, location.col );
	location = data.Cursor();

	if ( encoding == XXML_ENCODING_UNKNOWN )
	{
		// Check for the Microsoft UTF-8 lead bytes.
		if (	*(p+0) && *(p+0) == XXML_UTF_LEAD_0
			 && *(p+1) && *(p+1) == XXML_UTF_LEAD_1
			 && *(p+2) && *(p+2) == XXML_UTF_LEAD_2 )
		{
			encoding = XXML_ENCODING_UTF8;
		}
	}

    p = SkipWhiteSpace( p, encoding );
	if ( !p )
	{
		SetError( XXML_ERROR_DOCUMENT_EMPTY, 0, 0, XXML_ENCODING_UNKNOWN );
		return 0;
	}

	while ( p && *p )
	{
		CxXmlNode* node = Identify( p, encoding );
		if ( node )
		{
			p = node->Parse( p, &data, encoding );
			LinkEndChild( node );
		}
		else
		{
			break;
		}

		// Did we get encoding info?
		if (    encoding == XXML_ENCODING_UNKNOWN
			 && node->ToDeclaration() )
		{
			CxXmlDeclaration* dec = node->ToDeclaration();
			const char* enc = dec->Encoding();
			XASSERT( enc );

			if ( *enc == 0 )
				encoding = XXML_ENCODING_UTF8;
			else if ( StringEqual( enc, "UTF-8", TRUE, XXML_ENCODING_UNKNOWN ) )
				encoding = XXML_ENCODING_UTF8;
			else if ( StringEqual( enc, "UTF8", TRUE, XXML_ENCODING_UNKNOWN ) )
				encoding = XXML_ENCODING_UTF8;	// incorrect, but be nice
			else 
				encoding = XXML_ENCODING_LEGACY;
		}

		p = SkipWhiteSpace( p, encoding );
	}

	// Was this empty?
	if ( !firstChild ) {
		SetError( XXML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding );
		return 0;
	}

	// All is well.
	return p;
}

void CxXmlDocument::SetError( int err, const char* pError, CxXmlParsingData* data, CxXmlEncoding encoding )
{	
	// The first error in a chain is more accurate - don't set again!
	if ( error )
		return;

	XASSERT( err > 0 && err < XXML_ERROR_STRING_COUNT );
	error   = TRUE;
	errorId = err;
	errorDesc = errorString[ errorId ];

	errorLocation.Clear();
	if ( pError && data )
	{
		//CxXmlParsingData data( pError, prevData );
		data->Stamp( pError, encoding );
		errorLocation = data->Cursor();
	}
}

BOOL CxXmlDocument::LoadFile( CxXmlEncoding encoding )
{
	// See STL_STRING_BUG below.
	StringToBuffer buf( value );

	if ( buf.buffer && LoadFile( buf.buffer, encoding ) )
		return TRUE;

	return FALSE;
}


BOOL CxXmlDocument::SaveFile() const
{
	// See STL_STRING_BUG below.
	StringToBuffer buf( value );

	if ( buf.buffer && SaveFile( buf.buffer ) )
		return TRUE;

	return FALSE;
}

BOOL CxXmlDocument::LoadFile( const char* filename, CxXmlEncoding encoding )
{
	// Delete the existing data:
	Clear();
	location.Clear();

	// There was a really terrifying little bug here. The code:
	//		value = filename
	// in the STL case, cause the assignment method of the std::string to
	// be called. What is strange, is that the std::string had the same
	// address as it's c_str() method, and so bad things happen. Looks
	// like a bug in the Microsoft STL implementation.
	// See STL_STRING_BUG above.
	// Fixed with the StringToBuffer class.
	value = filename;

	CxFile file;

	USES_CONVERSION;

	if ( file.Open( A2T((LPSTR)filename), CxFile::modeRead ) )
	{
		// Get the file size, so we can pre-allocate the string. HUGE speed impact.
		long length = 0;
		file.Seek( 0, CxFile::end );
		//fseek( file, 0, SEEK_END );
		length = file.GetPosition();
		//length = ftell( file );
		file.Seek( 0, CxFile::begin );
		//fseek( file, 0, SEEK_SET );

		// Strange case, but good to handle up front.
		if ( length == 0 )
		{
			file.Close();
			//fclose( file );
			return FALSE;
		}

		// If we have a file, assume it is all one big XML file, and read it in.
		// The document parser may decide the document ends sooner than the entire file, however.
		CxStdString data;
		data.reserve( length );

		const int BUF_SIZE = 2048;
		char buf[BUF_SIZE];
		buf[BUF_SIZE-1] = 0;

		int nLen;
		while ( (nLen = file.Read( buf, BUF_SIZE-1 )) > 0 )
		{
			buf[nLen] = 0;
			data += buf;
		}
//		while ( fgets( buf, BUF_SIZE, file ) )
//		{
//			data += buf;
//		}
		file.Close();
		//fclose( file );

		Parse( data.c_str(), 0, encoding );

		if (  Error() )
            return FALSE;
        else
			return TRUE;
	}
	SetError( XXML_ERROR_OPENING_FILE, 0, 0, XXML_ENCODING_UNKNOWN );
	return FALSE;
}

BOOL CxXmlDocument::LoadFile( CxFile& archive, CxXmlEncoding encoding )
{
	// Delete the existing data:
	Clear();
	location.Clear();

	// There was a really terrifying little bug here. The code:
	//		value = filename
	// in the STL case, cause the assignment method of the std::string to
	// be called. What is strange, is that the std::string had the same
	// address as it's c_str() method, and so bad things happen. Looks
	// like a bug in the Microsoft STL implementation.
	// See STL_STRING_BUG above.
	// Fixed with the StringToBuffer class.

	if ( TRUE )
	{
		// Get the file size, so we can pre-allocate the string. HUGE speed impact.
		long length = 0;
		archive.Seek( 0, CxFile::end );
		//fseek( file, 0, SEEK_END );
		length = archive.GetPosition();
		//length = ftell( file );
		archive.Seek( 0, CxFile::begin );
		//fseek( file, 0, SEEK_SET );

		// Strange case, but good to handle up front.
		if ( length == 0 )
		{
			//archive.Close();
			//fclose( file );
			return FALSE;
		}

		// If we have a file, assume it is all one big XML file, and read it in.
		// The document parser may decide the document ends sooner than the entire file, however.
		CxStdString data;
		data.reserve( length );

		const int BUF_SIZE = 2048;
		char buf[BUF_SIZE];
		buf[BUF_SIZE-1] = 0;

		int nLen;
		while ( (nLen = archive.Read( buf, BUF_SIZE-1 )) > 0 )
		{
			buf[nLen] = 0;
			data += buf;
		}
//		while ( fgets( buf, BUF_SIZE, file ) )
//		{
//			data += buf;
//		}
		//archive.Close();
		//fclose( file );

		Parse( data.c_str(), 0, encoding );

		if (  Error() )
            return FALSE;
        else
			return TRUE;
	}
	SetError( XXML_ERROR_OPENING_FILE, 0, 0, XXML_ENCODING_UNKNOWN );
	return FALSE;
}

BOOL CxXmlDocument::LoadFile( CxMemFile& archive, CxXmlEncoding encoding/* = XXML_DEFAULT_ENCODING*/ )
{
	// Delete the existing data:
	Clear();
	location.Clear();

	// There was a really terrifying little bug here. The code:
	//		value = filename
	// in the STL case, cause the assignment method of the std::string to
	// be called. What is strange, is that the std::string had the same
	// address as it's c_str() method, and so bad things happen. Looks
	// like a bug in the Microsoft STL implementation.
	// See STL_STRING_BUG above.
	// Fixed with the StringToBuffer class.

	if ( TRUE )
	{
		// Get the file size, so we can pre-allocate the string. HUGE speed impact.
		long length = 0;
		length = archive.GetSize();

		archive.SetCurPos( 0 );

		// Strange case, but good to handle up front.
		if ( length == 0 )
		{
			return FALSE;
		}

		// If we have a file, assume it is all one big XML file, and read it in.
		// The document parser may decide the document ends sooner than the entire file, however.
		CxStdString data;
		data.reserve( length );

		const int BUF_SIZE = 2048;
		char buf[BUF_SIZE];
		buf[BUF_SIZE-1] = 0;

		int nLen;
		while ( (nLen = archive.Read( buf, BUF_SIZE-1 )) > 0 )
		{
			buf[nLen] = 0;
			data += buf;
		}
//		while ( fgets( buf, BUF_SIZE, file ) )
//		{
//			data += buf;
//		}
		//archive.Close();
		//fclose( file );

		Parse( data.c_str(), 0, encoding );

		if (  Error() )
            return FALSE;
        else
			return TRUE;
	}
	SetError( XXML_ERROR_OPENING_FILE, 0, 0, XXML_ENCODING_UNKNOWN );
	return FALSE;
}

BOOL CxXmlDocument::SaveFile( CxArchive& archive ) const
{
	Print( archive, 0 );
//	archive.Close();
	return TRUE;
}

BOOL CxXmlDocument::SaveFile( const char * filename ) const
{
	// The old c stuff lives on...
	CxFile File;
	USES_CONVERSION;
	if ( File.Open( A2T((LPSTR)filename), CxFile::modeWrite|CxFile::modeCreate ) == TRUE )
	{
		Print( File, 0 );
		File.Close();
		return TRUE;
	}
	return FALSE;
}


void CxXmlDocument::CopyTo( CxXmlDocument* target ) const
{
	CxXmlNode::CopyTo( target );

	target->error = error;
	target->errorDesc = errorDesc.c_str ();

	CxXmlNode* node = 0;
	for ( node = firstChild; node; node = node->NextSibling() )
	{
		target->LinkEndChild( node->Clone() );
	}	
}


CxXmlNode* CxXmlDocument::Clone() const
{
	CxXmlDocument* clone = new CxXmlDocument();
	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}


void CxXmlDocument::Print( CxArchive& cfile, int depth ) const
{
	const CxXmlNode* node;
	CxString strCRLF = "\r\n";
	node = FirstChild();
	if ( node->Type() != DECLARATION )
	{
		const char str[] = "<?xml version=\"1.0\" encoding=\"euc-kr\" ?>\r\n";
		cfile.Write( str, sizeof(str)-1 );
	}
	for ( ; node; node=node->NextSibling() )
	{
		node->Print( cfile, depth );
		cfile.Write( strCRLF, strCRLF.GetLength() );
	}
}

void CxXmlDocument::StreamOut( CxOStream * out ) const
{
	const CxXmlNode* node;
	for ( node=FirstChild(); node; node=node->NextSibling() )
	{
		node->StreamOut( out );

		// Special rule for streams: stop after the root element.
		// The stream in code will only read one element, so don't
		// write more than one.
		if ( node->ToElement() )
			break;
	}
}

//////////////////////////////////////////////////////////////////////////
// CxXmlAttribute
const CxXmlAttribute* CxXmlAttribute::Next() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( next->value.empty() && next->name.empty() )
		return 0;
	return next;
}

CxXmlAttribute* CxXmlAttribute::Next()
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( next->value.empty() && next->name.empty() )
		return 0;
	return next;
}

const CxXmlAttribute* CxXmlAttribute::Previous() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( prev->value.empty() && prev->name.empty() )
		return 0;
	return prev;
}

CxXmlAttribute* CxXmlAttribute::Previous()
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value or name.
	if ( prev->value.empty() && prev->name.empty() )
		return 0;
	return prev;
}

void CxXmlAttribute::Print( CxArchive& cfile, int /*depth*/ ) const
{
	CxStdString n, v;

	PutString( name, &n );
	PutString( value, &v );

	CxString strTemp;

	if (value.find ('\"') == CxStdString::npos)
	{
		strTemp.Format( _T("%s=\"%s\""), n.c_str(), v.c_str() );
		cfile.Write( strTemp, strTemp.GetLength() );
	}
	else
	{
		strTemp.Format( _T("%s='%s'"), n.c_str(), v.c_str() );
		cfile.Write( strTemp, strTemp.GetLength() );
	}
}

void CxXmlAttribute::StreamOut( CxOStream * stream ) const
{
	if (value.find( '\"' ) != CxStdString::npos)
	{
		PutString( name, stream );
		(*stream) << "=" << "'";
		PutString( value, stream );
		(*stream) << "'";
	}
	else
	{
		PutString( name, stream );
		(*stream) << "=" << "\"";
		PutString( value, stream );
		(*stream) << "\"";
	}
}

int CxXmlAttribute::QueryIntValue( int* ival ) const
{
	if ( sscanf( value.c_str(), "%d", ival ) == 1 )
		return XXML_SUCCESS;
	return XXML_WRONG_TYPE;
}

int CxXmlAttribute::QueryDoubleValue( double* dval ) const
{
	if ( sscanf( value.c_str(), "%lf", dval ) == 1 )
		return XXML_SUCCESS;
	return XXML_WRONG_TYPE;
}

void CxXmlAttribute::SetIntValue( int _value )
{
	char buf [64];
	sprintf (buf, "%d", _value);
	SetValue (buf);
}

void CxXmlAttribute::SetDoubleValue( double _value )
{
	char buf [64];
	sprintf (buf, "%lf", _value);
	SetValue (buf);
}

const int CxXmlAttribute::IntValue() const
{
	return atoi (value.c_str ());
}

const double  CxXmlAttribute::DoubleValue() const
{
	return atof (value.c_str ());
}

const char* CxXmlAttribute::Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding )
{
	p = SkipWhiteSpace( p, encoding );
	if ( !p || !*p ) return 0;

	int tabsize = 4;
	if ( document )
		tabsize = document->TabSize();

//	TiXmlParsingData data( p, prevData );
	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}
	// Read the name, the '=' and the value.
	const char* pErr = p;
	p = ReadName( p, &name, encoding );
	if ( !p || !*p )
	{
		if ( document ) document->SetError( XXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding );
		return 0;
	}
	p = SkipWhiteSpace( p, encoding );
	if ( !p || !*p || *p != '=' )
	{
		if ( document ) document->SetError( XXML_ERROR_READING_ATTRIBUTES, p, data, encoding );
		return 0;
	}

	++p;	// skip '='
	p = SkipWhiteSpace( p, encoding );
	if ( !p || !*p )
	{
		if ( document ) document->SetError( XXML_ERROR_READING_ATTRIBUTES, p, data, encoding );
		return 0;
	}
	
	const char* end;

	if ( *p == '\'' )
	{
		++p;
		end = "\'";
		p = ReadText( p, &value, FALSE, end, FALSE, encoding );
	}
	else if ( *p == '"' )
	{
		++p;
		end = "\"";
		p = ReadText( p, &value, FALSE, end, FALSE, encoding );
	}
	else
	{
		// All attribute values should be in single or double quotes.
		// But this is such a common error that the parser will try
		// its best, even without them.
		value = "";
		while (    p && *p										// existence
				&& !IsWhiteSpace( *p ) && *p != '\n' && *p != '\r'	// whitespace
				&& *p != '/' && *p != '>' )						// tag end
		{
			value += *p;
			++p;
		}
	}
	return p;
}

//////////////////////////////////////////////////////////////////////////
// CxXmlComment
CxXmlComment::CxXmlComment( const CxXmlComment& copy ) : CxXmlNode( CxXmlNode::COMMENT )
{
	copy.CopyTo( this );
}


void CxXmlComment::operator=( const CxXmlComment& base )
{
	Clear();
	base.CopyTo( this );
}

void CxXmlComment::Print( CxArchive& cfile, int depth ) const
{
	CxString strTemp = _T("    ");
	for ( int i=0; i<depth; i++ )
	{
		cfile.Write( strTemp, strTemp.GetLength() );
		//fputs( "    ", cfile );
	}
	strTemp.Format( _T("<!--%s-->"), value.c_str() );
	cfile.Write( strTemp, strTemp.GetLength() );
}

void CxXmlComment::StreamOut( CxOStream * stream ) const
{
	(*stream) << "<!--";
	//PutString( value, stream );
	(*stream) << value;
	(*stream) << "-->";
}


void CxXmlComment::CopyTo( CxXmlComment* target ) const
{
	CxXmlNode::CopyTo( target );
}


CxXmlNode* CxXmlComment::Clone() const
{
	CxXmlComment* clone = new CxXmlComment();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}

const char* CxXmlComment::Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding )
{
	CxXmlDocument* document = GetDocument();
	value = "";

	p = SkipWhiteSpace( p, encoding );

	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}
	const char* startTag = "<!--";
	const char* endTag   = "-->";

	if ( !StringEqual( p, startTag, FALSE, encoding ) )
	{
		document->SetError( XXML_ERROR_PARSING_COMMENT, p, data, encoding );
		return 0;
	}
	p += strlen( startTag );
	p = ReadText( p, &value, FALSE, endTag, FALSE, encoding );
	return p;
}

//////////////////////////////////////////////////////////////////////////
// CxXmlText
void CxXmlText::Print( CxArchive& cfile, int /*depth*/ ) const
{
	CxStdString buffer;
	PutString( value, &buffer );
	CxString strTemp;
	strTemp.Format( _T("%s"), buffer.c_str() );
	cfile.Write( strTemp, strTemp.GetLength() );
}

void CxXmlText::StreamOut( CxOStream * stream ) const
{
	PutString( value, stream );
}


void CxXmlText::CopyTo( CxXmlText* target ) const
{
	CxXmlNode::CopyTo( target );
}


CxXmlNode* CxXmlText::Clone() const
{	
	CxXmlText* clone = 0;
	clone = new CxXmlText( "" );

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}

const char* CxXmlText::Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding )
{
	value = "";

	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}
	BOOL ignoreWhite = TRUE;

	const char* end = "<";
	p = ReadText( p, &value, ignoreWhite, end, FALSE, encoding );
	if ( p )
		return p-1;	// don't truncate the '<'
	return 0;
}

BOOL CxXmlText::Blank() const
{
	for ( unsigned i=0; i<value.length(); i++ )
		if ( !IsWhiteSpace( value[i] ) )
			return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// CxXmlDeclaration
CxXmlDeclaration::CxXmlDeclaration( const char * _version,
									const char * _encoding,
									const char * _standalone )
	: CxXmlNode( CxXmlNode::DECLARATION )
{
	version = _version;
	encoding = _encoding;
	standalone = _standalone;
}

CxXmlDeclaration::CxXmlDeclaration( const CxXmlDeclaration& copy )
	: CxXmlNode( CxXmlNode::DECLARATION )
{
	copy.CopyTo( this );	
}


void CxXmlDeclaration::operator=( const CxXmlDeclaration& copy )
{
	Clear();
	copy.CopyTo( this );
}


void CxXmlDeclaration::Print( CxArchive& cfile, int /*depth*/ ) const
{
	CxString strTemp;
	strTemp = _T("<?xml ");
	cfile.Write( strTemp, strTemp.GetLength() );

	USES_CONVERSION;

	if ( !version.empty() )
	{
		strTemp.Format( _T("version=\"%s\" "), A2T((LPSTR)version.c_str()) );
		cfile.Write( strTemp, strTemp.GetLength() );
	}
	if ( !encoding.empty() )
	{
		strTemp.Format( _T("encoding=\"%s\" "), A2T((LPSTR)encoding.c_str()) );
		cfile.Write( strTemp, strTemp.GetLength() );
	}
	if ( !standalone.empty() )
	{
		strTemp.Format( _T("standalone=\"%s\" "), A2T((LPSTR)standalone.c_str()) );
		cfile.Write( strTemp, strTemp.GetLength() );
	}
	strTemp = "?>";
	cfile.Write( strTemp, strTemp.GetLength() );
}

void CxXmlDeclaration::StreamOut( CxOStream * stream ) const
{
	(*stream) << "<?xml ";

	if ( !version.empty() )
	{
		(*stream) << "version=\"";
		PutString( version, stream );
		(*stream) << "\" ";
	}
	if ( !encoding.empty() )
	{
		(*stream) << "encoding=\"";
		PutString( encoding, stream );
		(*stream ) << "\" ";
	}
	if ( !standalone.empty() )
	{
		(*stream) << "standalone=\"";
		PutString( standalone, stream );
		(*stream) << "\" ";
	}
	(*stream) << "?>";
}


void CxXmlDeclaration::CopyTo( CxXmlDeclaration* target ) const
{
	CxXmlNode::CopyTo( target );

	target->version = version;
	target->encoding = encoding;
	target->standalone = standalone;
}


CxXmlNode* CxXmlDeclaration::Clone() const
{	
	CxXmlDeclaration* clone = new CxXmlDeclaration();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}

const char* CxXmlDeclaration::Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding _encoding )
{
	p = SkipWhiteSpace( p, _encoding );
	// Find the beginning, find the end, and look for
	// the stuff in-between.
	CxXmlDocument* document = GetDocument();
	if ( !p || !*p || !StringEqual( p, "<?xml", TRUE, _encoding ) )
	{
		if ( document ) document->SetError( XXML_ERROR_PARSING_DECLARATION, 0, 0, _encoding );
		return 0;
	}

	if ( data )
	{
		data->Stamp( p, _encoding );
		location = data->Cursor();
	}
	p += 5;

	version = "";
	encoding = "";
	standalone = "";

	while ( p && *p )
	{
		if ( *p == '>' )
		{
			++p;
			return p;
		}

		p = SkipWhiteSpace( p, _encoding );
		if ( StringEqual( p, "version", TRUE, _encoding ) )
		{
			CxXmlAttribute attrib;
			p = attrib.Parse( p, data, _encoding );		
			version = attrib.Value();
		}
		else if ( StringEqual( p, "encoding", TRUE, _encoding ) )
		{
			CxXmlAttribute attrib;
			p = attrib.Parse( p, data, _encoding );		
			encoding = attrib.Value();
		}
		else if ( StringEqual( p, "standalone", TRUE, _encoding ) )
		{
			CxXmlAttribute attrib;
			p = attrib.Parse( p, data, _encoding );		
			standalone = attrib.Value();
		}
		else
		{
			// Read over whatever it is.
			while ( p && *p && *p != '>' && !IsWhiteSpace( *p ) )
				++p;
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// CxXmlUnknown
void CxXmlUnknown::Print( CxArchive& cfile, int depth ) const
{
	CxString strTemp = _T("    ");
	for ( int i=0; i<depth; i++ )
	{
		cfile.Write( strTemp, strTemp.GetLength() );
	}

	USES_CONVERSION;

	strTemp.Format( _T("<%s>"), A2T((LPSTR)value.c_str()) );
	cfile.Write( strTemp, strTemp.GetLength() );
}

void CxXmlUnknown::StreamOut( CxOStream * stream ) const
{
	(*stream) << "<" << value << ">";		// Don't use entities here! It is unknown.
}


void CxXmlUnknown::CopyTo( CxXmlUnknown* target ) const
{
	CxXmlNode::CopyTo( target );
}

CxXmlNode* CxXmlUnknown::Clone() const
{
	CxXmlUnknown* clone = new CxXmlUnknown();

	if ( !clone )
		return 0;

	CopyTo( clone );
	return clone;
}

const char* CxXmlUnknown::Parse( const char* p, CxXmlParsingData* data, CxXmlEncoding encoding )
{
	CxXmlDocument* document = GetDocument();
	p = SkipWhiteSpace( p, encoding );

//	CxXmlParsingData data( p, prevData );
	if ( data )
	{
		data->Stamp( p, encoding );
		location = data->Cursor();
	}
	if ( !p || !*p || *p != '<' )
	{
		if ( document ) document->SetError( XXML_ERROR_PARSING_UNKNOWN, p, data, encoding );
		return 0;
	}
	++p;
    value = "";

	while ( p && *p && *p != '>' )
	{
		value += *p;
		++p;
	}

	if ( !p )
	{
		if ( document )	document->SetError( XXML_ERROR_PARSING_UNKNOWN, 0, 0, encoding );
	}
	if ( *p == '>' )
		return p+1;
	return p;
}

CxXmlAttributeSet::CxXmlAttributeSet()
{
	sentinel.next = &sentinel;
	sentinel.prev = &sentinel;
}


CxXmlAttributeSet::~CxXmlAttributeSet()
{
	XASSERT( sentinel.next == &sentinel );
	XASSERT( sentinel.prev == &sentinel );
}


void CxXmlAttributeSet::Add( CxXmlAttribute* addMe )
{
	XASSERT( !Find( addMe->Name() ) );	// Shouldn't be multiply adding to the set.

	addMe->next = &sentinel;
	addMe->prev = sentinel.prev;

	sentinel.prev->next = addMe;
	sentinel.prev      = addMe;
}

void CxXmlAttributeSet::Remove( CxXmlAttribute* removeMe )
{
	CxXmlAttribute* node;

	for ( node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( node == removeMe )
		{
			node->prev->next = node->next;
			node->next->prev = node->prev;
			node->next = 0;
			node->prev = 0;
			return;
		}
	}
	XASSERT( 0 );		// we tried to remove a non-linked attribute.
}

const CxXmlAttribute*	CxXmlAttributeSet::Find( const char * name ) const
{
	const CxXmlAttribute* node;

	for ( node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( node->name == name )
			return node;
	}
	return 0;
}

CxXmlAttribute*	CxXmlAttributeSet::Find( const char * name )
{
	CxXmlAttribute* node;

	for ( node = sentinel.next; node != &sentinel; node = node->next )
	{
		if ( node->name == name )
			return node;
	}
	return 0;
}

CxOStream & operator<< (CxOStream & out, const CxXmlNode & base)
{
	base.StreamOut (& out);
	return out;
}

CxXmlHandle CxXmlHandle::FirstChild() const
{
	if ( node )
	{
		CxXmlNode* child = node->FirstChild();
		if ( child )
			return CxXmlHandle( child );
	}
	return CxXmlHandle( 0 );
}


CxXmlHandle CxXmlHandle::FirstChild( const char * value ) const
{
	if ( node )
	{
		CxXmlNode* child = node->FirstChild( value );
		if ( child )
			return CxXmlHandle( child );
	}
	return CxXmlHandle( 0 );
}


CxXmlHandle CxXmlHandle::FirstChildElement() const
{
	if ( node )
	{
		CxXmlElement* child = node->FirstChildElement();
		if ( child )
			return CxXmlHandle( child );
	}
	return CxXmlHandle( 0 );
}


CxXmlHandle CxXmlHandle::FirstChildElement( const char * value ) const
{
	if ( node )
	{
		CxXmlElement* child = node->FirstChildElement( value );
		if ( child )
			return CxXmlHandle( child );
	}
	return CxXmlHandle( 0 );
}


CxXmlHandle CxXmlHandle::Child( int count ) const
{
	if ( node )
	{
		int i;
		CxXmlNode* child = node->FirstChild();
		for (	i=0;
				child && i<count;
				child = child->NextSibling(), ++i )
		{
			// nothing
		}
		if ( child )
			return CxXmlHandle( child );
	}
	return CxXmlHandle( 0 );
}


CxXmlHandle CxXmlHandle::Child( const char* value, int count ) const
{
	if ( node )
	{
		int i;
		CxXmlNode* child = node->FirstChild( value );
		for (	i=0;
				child && i<count;
				child = child->NextSibling( value ), ++i )
		{
			// nothing
		}
		if ( child )
			return CxXmlHandle( child );
	}
	return CxXmlHandle( 0 );
}


CxXmlHandle CxXmlHandle::ChildElement( int count ) const
{
	if ( node )
	{
		int i;
		CxXmlElement* child = node->FirstChildElement();
		for (	i=0;
				child && i<count;
				child = child->NextSiblingElement(), ++i )
		{
			// nothing
		}
		if ( child )
			return CxXmlHandle( child );
	}
	return CxXmlHandle( 0 );
}


CxXmlHandle CxXmlHandle::ChildElement( const char* value, int count ) const
{
	if ( node )
	{
		int i;
		CxXmlElement* child = node->FirstChildElement( value );
		for (	i=0;
				child && i<count;
				child = child->NextSiblingElement( value ), ++i )
		{
			// nothing
		}
		if ( child )
			return CxXmlHandle( child );
	}
	return CxXmlHandle( 0 );
}
