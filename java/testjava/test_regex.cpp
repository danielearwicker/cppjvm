#include <java/util/regex/Pattern.hpp>

#include "assert.hpp"

void test_regex()
{
	using namespace java::util::regex;
    using namespace java::lang;
    
	Pattern pattern = Pattern::compile("\\{[a-z]*\\}");

	Matcher matcher = pattern.matcher(String("Some of the {words} in this {sentence} are {special}"));

	ASSERT_EQUAL(jboolean, true, matcher.find());
	ASSERT_EQUAL(std::string, "{words}", matcher.group());
	ASSERT_EQUAL(jboolean, true, matcher.find());
	ASSERT_EQUAL(std::string, "{sentence}", matcher.group());
	ASSERT_EQUAL(jboolean, true, matcher.find());
	ASSERT_EQUAL(std::string, "{special}", matcher.group());
}

REGISTER_TEST(test_regex);
