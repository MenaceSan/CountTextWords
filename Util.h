#pragma once
#include "pch.h"

namespace Util
{
	// Things not well supported by STL.

	static inline int StrCmpI(const char* a, const char* b)
	{
		// strcasecmp = Linux/POSIX . #include <strings.h>

#if false // def _MSC_VER
		return ::_strcmpi(a, b);
#else
		while (true)
		{
			char cha = std::toupper(*a);
			char chb = std::toupper(*b);
			if (cha != chb)
			{
				return cha < chb ? -1 : 1;
			}
			if (cha == '\0')
				return 0;	// equal.
			++a; ++b;
		}
#endif
	}

	static inline std::string CvtA(std::string s)
	{
		// To utf8. No change
		return s;
	}
	static inline std::string CvtA(std::wstring s)
	{
		// To utf8
		return std::string(s.begin(), s.end());
	}

	static inline std::wstring CvtW(std::string s)
	{
		// To unicode
		return std::wstring(s.begin(), s.end());
	}
	static inline std::wstring CvtW(std::wstring s)
	{
		// To unicode. No change
		return s;
	}

	// TODO template Cvt ?
	// template<typedef _Elem> static inline basic_string<_Elem, char_traits<_Elem>, allocator<_Elem>> Cvt(std::wstring s);
}
