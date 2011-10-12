#include <java/net/Proxy/Type.hpp>
#include <jvm/local_frame.hpp>

#include "assert.hpp"

void test_nested()
{
    jvm::local_frame lf;
    
    // should be able to allocate an array of a nested type
    jvm::array<java::net::Proxy_n::Type> arrayOfProxyTypes(1);
	ASSERT_EQUAL(int, 1, arrayOfProxyTypes.length());

    // Also obtain an array of instances and try using it
    arrayOfProxyTypes = java::net::Proxy_n::Type::values();
    for (int n = 0; n < arrayOfProxyTypes.length(); n++) {
        std::cout << arrayOfProxyTypes[n].name().str() << std::endl;
    }
}

REGISTER_TEST(test_nested);
