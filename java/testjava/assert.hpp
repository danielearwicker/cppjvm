#ifndef TEST_JAVA_ASSERT_INCLUDED
#define TEST_JAVA_ASSERT_INCLUDED

#include <java/lang/Object.hpp>
#include <java/lang/String.hpp>

#include <sstream>
#include <iostream>
#include <vector>

template <class T>
std::string toString(const T &v)
{
	std::ostringstream o;
	o << v;
	return o.str();
}

inline std::string toString(java::lang::Object obj)
{
	return obj.toString();
}

template <class T>
void assert_equal(const T &expected, const T &actual, const char *description)
{
	if (expected != actual) 
	{
		std::ostringstream o;
		o << "Assertion failed: " << description << std::endl
		  << "Unexpected value: " << toString(actual);
		throw std::logic_error(o.str());
	}
}

#define AS_STR(l) #l
#define ASSERT_EQUAL(t, exp, act) assert_equal<t>((exp), (act), #exp " == " #act " " __FILE__ "(" AS_STR(__LINE__) ")")

typedef void test_func();

struct test
{
	const char *descr;
	test_func *func;
};

class test_registration
{
public:
	static std::vector<test> &list()
	{
		static std::vector<test> l;
		return l;
	}

	test_registration(const char *descr, test_func *func)
	{
		test t;
		t.descr = descr;
		t.func = func;
		list().push_back(t);
	}
};

#define REGISTER_TEST(testname) namespace { test_registration tr_(#testname, testname); }

#endif
