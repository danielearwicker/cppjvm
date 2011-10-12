#include <java/io/ByteArrayOutputStream.hpp>

#include <jvm/array.hpp>

#include "assert.hpp"

#include <string.h>
#include <vector>

void test_bytearray()
{
	using namespace java::io;
	using namespace jvm;

	ByteArrayOutputStream stream = jnew<ByteArrayOutputStream>();

	array<jbyte> b1(6);
	b1[0] = 'H';
	b1[1] = 'e';
	b1[2] = 'l';
	b1[3] = 'l';
	b1[4] = 'o';
	b1[5] = ' ';

	// demonstrate bulk operation on array using raw buffers
	array<jbyte> b2(6);
	b2.put(0, reinterpret_cast<const jbyte *>("world."), 6);

	stream.write(b1);
	stream.write(b2);

	array<jbyte> b3 = stream.toByteArray();

	std::vector<jbyte> result(b3.length());
	b3.get(0, &result[0], result.size());

	std::cout.write(reinterpret_cast<const char *>(&result[0]), result.size());
	std::cout << std::endl;

	ASSERT_EQUAL(int, 0, memcmp("Hello world.", &result[0], result.size()));
}

REGISTER_TEST(test_bytearray);
