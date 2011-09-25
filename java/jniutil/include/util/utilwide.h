#ifndef UTIL_WIDE
#define UTIL_WIDE

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <stdexcept>

#include <util/utilstringbuffer.h>

/*! \file include/util/utilwide.h
	\brief Functions for converting between wide and non-wide characters
*/

namespace Util
{
	//! Helper class that sets the code page and resets it on destruction
	class CodePageContext
	{
		const char *m_pNew, *m_pOld;

	public:
		//! Sets the locale and remembers the previous setting
		CodePageContext(const char *pLocale)
			: m_pNew(pLocale)
		{
			if (m_pNew != 0)
				m_pOld = setlocale(LC_CTYPE, m_pNew); 
		}

		~CodePageContext()
		{
			if (m_pNew != 0)
				setlocale(LC_CTYPE, m_pOld); 
		}
	};

	inline void ThrowInvalidChar()
		{ throw std::invalid_argument("Invalid character sequence"); }

	inline int GetWideCharCount(const char *pStr, int nLength = -1, const char *pLocale = "")
	{
		CodePageContext cpc(pLocale);

		if (nLength == -1)
			nLength = (int)strlen(pStr);

		int nTotal = 0;
		wchar_t wchJunk;

		while (nLength > 0)
		{
			int nBytes = mbtowc(&wchJunk, pStr, nLength);
			if (nBytes == 0)
				break;

			if (nBytes == -1)
				ThrowInvalidChar();

			pStr += nBytes;
			nLength -= nBytes;
			nTotal++;
		}

		return nTotal;
	}

	inline int GetWideChars(const char *pStr, int nLength, wchar_t *pOut, int nOutLength, const char *pLocale = "")
	{
		CodePageContext cpc(pLocale);

		if (nLength == -1)
			nLength = (int)strlen(pStr);

		int nTotal = 0;

		while ((nLength > 0) && (nOutLength > 0))
		{
			int nBytes = mbtowc(pOut, pStr, nLength);
			if (nBytes == 0)
				break;

			if (nBytes == -1)
				ThrowInvalidChar();

			pOut++;
			nOutLength--;
			pStr += nBytes;
			nLength -= nBytes;
			nTotal++;
		}

		return nTotal;
	}

	inline int ToWideChar(const char *pStr, int nLength, std::wstring &ws, const char *pLocale = "")
	{
		CodePageContext cpc(pLocale);

		int nChars = GetWideCharCount(pStr, nLength, 0);
		GetWideChars(pStr, nLength, StringBuffer(ws, nChars), nChars, 0);
		return nChars;
	}

	inline std::wstring ToWideChar(const char *pStr, int nLength = -1, const char *pLocale = "")
	{
		std::wstring m;
		ToWideChar(pStr, nLength, m, pLocale);
		return m;
	}

	inline std::wstring ToWideChar(const std::string &s, int nLength = -1, const char *pLocale = "")
		{ return ToWideChar(s.c_str(), nLength, pLocale); }

	inline const wchar_t *ToWideChar(const wchar_t *pStr)
		{ return pStr; }

	inline const std::wstring &ToWideChar(const std::wstring &ws)
		{ return ws; }

	inline char GetSubstitute(char chSubst)
	{
		if (chSubst == '\0')
			throw std::invalid_argument("Unicode character cannot be represented in multi-byte encoding");

		return chSubst;
	}

	const char chDefaultSubst = '?';

	inline int GetMultiByteCount(const wchar_t *pStr, int nChars = -1, 
								 char chSubst = chDefaultSubst, const char *pLocale = "")
	{
		CodePageContext cpc(pLocale);

        //VS2005 - wcslen now dies if pStr is Null so just exit as the only answer can be 0 anyway
        if( pStr == NULL )
            return 0;

		if (nChars == -1)
			nChars = (int)wcslen(pStr);

		int nTotal = 0;
		char chJunk[10];

		while (nChars > 0)
		{
			int nBytes = wctomb(chJunk, *pStr);
			if (nBytes == 0)
				break;

			if (nBytes == -1)
			{
				GetSubstitute(chSubst);
				nBytes = 1;
			}

			nTotal += nBytes;
			pStr++;
			nChars--;
		}

		return nTotal;
	}

	inline int GetMultiBytes(const wchar_t *pStr, int nChars, char *pOut, int nOutLength, 
							 char chSubst = chDefaultSubst, const char *pLocale = "")
	{
		CodePageContext cpc(pLocale);

        //VS2005 - wcslen now dies if pStr is Null so just exit as the only answer can be 0 anyway
        if( pStr == NULL )
            return 0;

		if (nChars == -1)
			nChars = (int)wcslen(pStr);

		int nTotal = 0;
		char chTemp[10];

		while ((nChars > 0) && (nOutLength > 0))
		{
			int nBytes = wctomb(chTemp, *pStr);
			if (nBytes == 0)
				break;

			if (nBytes == -1)
			{
				chTemp[0] = GetSubstitute(chSubst);
				nBytes = 1;
			}

			if (nBytes > nOutLength)
				break;

			nTotal += nBytes;

			memcpy(pOut, chTemp, nBytes);

			pOut += nBytes;
			nOutLength -= nBytes;

			pStr++;
			nChars--;
		}

		return nTotal;
	}

	inline int ToMultiByte(const wchar_t *pStr, int nChars, std::string &s, 
						   char chSubst = chDefaultSubst, const char *pLocale = "")
	{
		CodePageContext cpc(pLocale);

		int nBytes = GetMultiByteCount(pStr, nChars, chSubst, 0);
		GetMultiBytes(pStr, nChars, StringBuffer(s, nBytes), nBytes, chSubst, 0);
		return nBytes;
	}

	inline std::string ToMultiByte(const wchar_t *pStr, int nChars = -1, 
								   char chSubst = chDefaultSubst, const char *pLocale = "")
	{
		std::string m;
		ToMultiByte(pStr, nChars, m, chSubst, pLocale);
		return m;
	}

	inline std::string ToMultiByte(const std::wstring &ws, int nChars = -1, 
								   char chSubst = chDefaultSubst, const char *pLocale = "")
		{ return ToMultiByte(ws.c_str(), nChars, chSubst, pLocale); }

	inline const char *ToMultiByte(const char *pStr)
		{ return pStr; }

	inline const std::string &ToMultiByte(const std::string &s)
		{ return s; }

	inline void FromWideChar(const wchar_t *pStr, std::string &s)
		{ s = ToMultiByte(pStr); }

	inline void FromWideChar(const wchar_t *pStr, std::wstring &ws)
		{ ws = pStr; }

	inline void FromMultiByte(const char *pStr, std::string &s)
		{ s = pStr; }

	inline void FromWideChar(const char *pStr, std::wstring &ws)
		{ ws = ToWideChar(pStr); }
}

#endif
