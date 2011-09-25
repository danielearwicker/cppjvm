#ifndef UTIL_UTF8CONV
#define UTIL_UTF8CONV

#include <limits.h>
#include <string>

#include <util/utilstringbuffer.h>

/*! \file include/wax/waxutf8conv.h
	\brief Conversion between UTF-8 and wide characters.

	UTF-8 is a must these days for transmitting Unicode text, 
	but	it isn't a guaranteed capability on every platform. 
	Fortunately it's a simple translation to implement. Most
	of the functions here are just variations to combine 
	convenience and flexibility.

	The basic operations CharToUTF8 and CharFromUTF8 are 
	lifted from the Linux NTFS driver, which in turn got them 
	from Python's wstrop module. They support UCS-16, the same 
	range of characters as Windows.
*/

namespace Util
{
	/*! \brief Converts a 16-bit wide character to UTF8.
		\param c The character to convert
		\param sbuf The buffer to write the UTF8 representation to
		\return The number of characters written to the buffer, which will be between 0 and 3.
	*/
	inline int CharToUTF8(unsigned short c, char *sbuf = NULL)
	{
		unsigned char *buf = (unsigned char *)sbuf;

		if (c == 0)
			return 0; // No support for embedded 0 runes.

		if (c < 0x80) 
		{
			if (buf)
				buf[0] = (char)c;
			
			return 1;
		}

		if (c < 0x800) 
		{
			if (buf) 
			{
				buf[0] = 0xc0 | (c >> 6);
				buf[1] = 0x80 | (c & 0x3f);
			}

			return 2;
		}

		// c < 0x10000
		if (buf) 
		{
			buf[0] = 0xe0 | (c >> 12);
			buf[1] = 0x80 | ((c >> 6) & 0x3f);
			buf[2] = 0x80 | (c & 0x3f);
		}

		return 3;
	}
 
	/*! \brief Converts a UTF8 sequence into a single 16-bit wide character.
		\param sstr Pointer to the UTF8 sequence to convert
		\param c Pointer to the variable that will receive the result
		\return The number of bytes consumed, which will be zero if the sequence is invalid
	*/
	inline int CharFromUTF8(const char *sstr, unsigned short *c)
	{
		const unsigned char *str = (const unsigned char *)sstr;
		int l = 0, i;

		if (*str < 0x80) 
		{
			 *c = *str;
			 return 1;
		}
		
		if (*str < 0xc0)        /* Lead byte must not be 10xxxxxx. */
			 return 0;       /* Is c0 a possible lead byte? */
		
		if (*str < 0xe0) 
		{              
			/* 110xxxxx */
			*c = *str & 0x1f;
			l = 2;
		}
		else if (*str < 0xf0) 
		{   
			/* 1110xxxx */
			*c = *str & 0xf;
			l = 3;
		} 
		else if (*str < 0xf8) 
		{   
			/* 11110xxx */
			*c = *str & 7;
			l = 4;
		} 
		else /* We don't support characters above 0xFFFF in NTFS. */
			return 0;

		for (i = 1; i < l; i++) 
		{
			/* All other bytes must be 10xxxxxx. */
			if ((str[i] & 0xc0) != 0x80)
				return 0;

			*c <<= 6;
			*c |= str[i] & 0x3f;
		}

		return l;
	}

	/*! \brief Returns the number of characters in the UTF-8 string.
		\param pStr The string to examine
		\param nLength The number of bytes to examine (default is whole string)
		\return The number of characters
	*/
	inline int CharCountUTF8(const char *pStr, int nLength = -1)
	{
		if (nLength < 0)
			nLength = INT_MAX;

		unsigned short junk;
		int nCount = 0;

		while ((nLength > 0) && (*pStr != '\0'))
		{
			int nConsumed = CharFromUTF8(pStr, &junk);
			if (nConsumed == 0)
				break;

			if (nConsumed > nLength)
				break;

			nLength -= nConsumed;
			pStr += nConsumed;
			nCount++;
		}

		return nCount;
	}

	/*! \brief Convert a UTF8 string to a wide character buffer.
		\param pStr The string to convert
		\param nLength The number of bytes to examine (if negative, the whole string)
		\param pOut The buffer to write the output to
		\param nOutLength The maximum number of characters to write to the output buffer (including the NUL terminator)
		\return The number of characters written to the buffer
	*/
	inline int FromUTF8(const char *pStr, int nLength, wchar_t *pOut, int nOutLength)
	{
		if (nLength < 0)
			nLength = INT_MAX;

		wchar_t *pStart = pOut;
		unsigned short wc;

		while ((nLength > 0) && (nOutLength > 0) && (*pStr != '\0'))
		{
			int nConsumed = CharFromUTF8(pStr, &wc);
			if (nConsumed == 0)
				break;

			if (nConsumed > nLength)
				break;

			*pOut = wc;
			pOut++;
			nOutLength--;
			pStr += nConsumed;
			nLength -= nConsumed;
		}

		if (nOutLength > 0)
			*pOut = L'\0';

		return (int)(pOut - pStart);
	}

	/*! \brief Convert a UTF8 string to a wide character string.
		\param pStr The string to convert
		\param nLength The number of bytes to examine (if negative, the whole string)
		\param ws The wide string to store the result in
		\return The number of characters written to the output string
	*/
	inline int FromUTF8(const char *pStr, int nLength, std::wstring &ws)
	{
		int nChars = CharCountUTF8(pStr, nLength);
		return FromUTF8(pStr, nLength, StringBuffer(ws, nChars), nChars);
	}

	/*! \brief Convert a UTF8 string to a wide character string.
		\param pStr The string to convert
		\param ws The wide string to store the result in
		\return The number of characters written to the output string
	*/
	inline int FromUTF8(const char *pStr, std::wstring &ws)
		{ return FromUTF8(pStr, -1, ws); }

	/*! \brief Convert a UTF8 string to a wide character string.
		\param pStr The string to convert
		\param nLength The number of bytes to examine (if negative, the whole string)
		\return The resulting wide-character string
	*/
	inline std::wstring FromUTF8(const char *pStr, int nLength = -1)
	{
		std::wstring ws;
		FromUTF8(pStr, nLength, ws);
		return ws;
	}

	inline std::wstring FromUTF8(const std::string &s, int nLength = -1)
		{ return FromUTF8(s.c_str(), nLength); }

	/*! \brief Returns the number of bytes needed to represent the specified string in UTF-8.
		\param pStr The string to examine
		\param nChars The number of characters to examine (default is whole string)
		\return The number of bytes needed
	*/
	inline int ByteCountUTF8(const wchar_t *pStr, int nChars = -1)
	{
		if (nChars < 0)
			nChars = INT_MAX;

		int nCount = 0;
		while ((nChars > 0) && (*pStr != L'\0'))
		{
			nCount += CharToUTF8((unsigned short)*pStr);
			pStr++;
			nChars--;
		}

		return nCount;
	}

	/*! \brief Converts a wide character string into a UTF-8 buffer
		\param pStr The source string
		\param nChars The number of characters to examine (if negative, the whole string)
		\param pOut The buffer to write to
		\param The maximum number of bytes to write to the output buffer
		\return The number of bytes written
	*/
	inline int ToUTF8(const wchar_t *pStr, int nChars, char *pOut, int nOutLength)
	{
		if (nChars < 0)
			nChars = INT_MAX;

		char ch[8];
		int nCount = 0;

		while ((nChars > 0) && (nOutLength > 0) && (*pStr != L'\0'))
		{
			int nBytes = CharToUTF8((unsigned short)*pStr, ch);
			if (nBytes > nOutLength)
				break;

			nCount += nBytes;
			nOutLength -= nBytes;

			for (int n=0; n<nBytes; n++)
				pOut[n] = ch[n];

			pStr++;
			pOut += nBytes;
			nChars--;
		}

		if (nOutLength > 0)
			*pOut = '\0';

		return nCount;
	}
	
	/*! \brief Converts a wide character string into a UTF-8 string
		\param pStr The source string
		\param nChars The number of characters to examine (if negative, the whole string)
		\param s The string to write to
		\return The number of bytes in the output string
	*/
	inline int ToUTF8(const wchar_t *pStr, int nChars, std::string &s)
	{
		int nBytes = ByteCountUTF8(pStr, nChars);
		return ToUTF8(pStr, nChars, StringBuffer(s, nBytes), nBytes);
	}

	/*! \brief Converts a wide character string into a UTF-8 string
		\param pStr The source string
		\param s The string to write to
		\return The number of bytes in the output string
	*/
	inline int ToUTF8(const wchar_t *pStr, std::string &s)
		{ return ToUTF8(pStr, -1, s); }

	/*! \brief Converts a wide character string into a UTF-8 string
		\param pStr The source string
		\param nChars The number of characters to examine (default is whole string)
		\return The resulting UTF-8 string
	*/
	inline std::string ToUTF8(const wchar_t *pStr, int nChars = -1)
	{
		std::string s;
		ToUTF8(pStr, nChars, s);
		return s;
	}

	inline std::string ToUTF8(const std::wstring &s, int nChars = -1)
		{ return ToUTF8(s.c_str(), nChars); }
}

#endif
