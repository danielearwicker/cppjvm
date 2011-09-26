#ifndef TEST_JAVA_ASSERT_INCLUDED
#define TEST_JAVA_ASSERT_INCLUDED

#include <java/lang/Object.hpp>
#include <java/lang/String.hpp>

#include <sstream>
#include <iostream>

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
	test *next;

	test(const char *d,
		test_func *f,
		test *n) 
		: descr(d), 
		  func(f), 
		  next(n) {}
};

class test_registration
{
public:
	static test **head()
	{
		static test *h = 0;
		return &h;
	}

	test_registration(const char *descr, test_func *func)
	{
		*(head()) = new test(descr, func, *(head()));
	}

	static int run()
	{
		int failed = 0;

		try
		{
			std::cout << "Running tests..." << std::endl;

			for (test *t = *(head()); t != 0; t = t->next)
			{
				try
				{
					std::cout << "Running " << t->descr << "..." << std::endl;
					t->func();
				}
				catch (const std::exception &e)
				{
					failed++;
					std::cout << e.what() << std::endl;
				}
			}

			if (failed == 0)
				std::cout << "Perfect :)" << std::endl;
			else
				std::cout << std::endl << failed << " failed :(" << std::endl;
		}
		catch (const std::exception &e)
		{
			std::cout << "General problem starting Java tests" << e.what() << std::endl;
		}

		test *d = *(head()); 
		while (d != 0)
		{
			test *n = d->next;
			delete d;
			d = n;
		}
		*(head()) = 0;

		return failed;
	}
};

#define REGISTER_TEST(testname) namespace { test_registration tr_(#testname, testname); }

#endif
