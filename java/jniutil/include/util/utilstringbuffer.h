#ifndef UTIL_STRINGBUFFER
#define UTIL_STRINGBUFFER

#include <string>
#include <vector>

/*
	\file include\util\utilstringbuffer.h
	A useful class for wrapping a std::basic_string<> when passing it into 
	a function that expects a writeable buffer of characters.
*/

namespace Util
{
	template <class C>
	class _StringBuffer
	{
		typename std::basic_string<C> &m_str;
		typename std::vector<C> m_buffer;

	public:
		_StringBuffer(typename std::basic_string<C> &str, size_t nSize)
			: m_str(str), m_buffer(nSize + 1) { get()[nSize] = (C)0; }

		~_StringBuffer()
			{ commit(); }

		C *get()
			{ return &(m_buffer[0]); }

		operator C *()
			{ return get(); }

		void commit()
		{
			if (m_buffer.size() != 0)
			{
				size_t l = std::char_traits<C>::length(get());

				m_str.assign(get(), l);

				m_buffer.resize(0);
			}
		}

		void abort()
			{ m_buffer.resize(0); }
	};

	template <class C>
	inline _StringBuffer<C> StringBuffer(typename std::basic_string<C> &str, size_t nSize)
		{ return _StringBuffer<C>(str, nSize); }
}

#endif
