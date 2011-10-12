#ifndef TEST_JAVA_ASSERT_INCLUDED
#define TEST_JAVA_ASSERT_INCLUDED

#include <testing/assert.hpp>

#include <java/lang/Object.hpp>
#include <java/lang/String.hpp>

inline std::string toString(java::lang::Object obj)
{
	return obj.toString();
}

#endif
