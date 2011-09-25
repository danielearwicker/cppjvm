#include "assert.hpp"

#include <java/lang/System.hpp>
#include <jvm/local_frame.hpp>
#include <jvm/global.hpp>

#define QBF "The quick brown fox jumped over the lazy dog."

void test_globals()
{
	using namespace java::lang;
	using namespace jvm;

	/*	If the declaration of s1 doesn't use java::global (as in
		the commented out version) then the assertion at the bottom
		MAY cause an access violation. Unfortunately this depends
		on the behaviour of the GC so it's unpredictable, making
		bugs hard to catch. To see it here you might also need to
		increase the number of loops to 100000 or more.

		The rule is: if a reference needs to survive outside of 
		a local_frame, it must be wrapped with global<T>.

        It should be noted that you cannot avoid this problem by
        the clever idea of simply never declaring any local_frame 
        objects. Without them, local references never die and so
        the objects referred to are never collected.
	*/

	global<String> s1;
	// String s1; // substitute this "naked" version to trigger problems

	const int loops = 1000; // May need to add some zeros
	for (int n = 0; n < loops; n++)
	{
		{
			// comment out the next line and memory consumption will explode
			local_frame lf;
			
			String s2(QBF QBF QBF QBF QBF);
			String s3(QBF QBF QBF QBF QBF);

			if (s1.is_null()) // store the first concatenation in s1
				s1 = s2.concat(s3);
		}
	}

	// By this point, the GC may have collected s1 if it is not global.
	ASSERT_EQUAL(std::string, QBF QBF QBF QBF QBF QBF QBF QBF QBF QBF, s1);
}

REGISTER_TEST(test_globals);
