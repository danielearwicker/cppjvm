#include "assert.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	try
	{
		jvm::create_global_vm("");
	}
	catch (const std::exception &e)
	{
		std::cout << "Initialising JVM: " << e.what() << std::endl;
	}

	return test_registration::run();
}
