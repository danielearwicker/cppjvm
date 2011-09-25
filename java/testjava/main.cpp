#include "assert.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	int failed = 0;

	try
	{
		jvm::create_global_vm("");
	
		std::vector<test>::iterator i;

		std::cout << "Running " << test_registration::list().size() << " tests..." << std::endl;

		for (i = test_registration::list().begin(); i != test_registration::list().end(); ++i)
		{
			try
			{
				std::cout << "Running " << i->descr << "..." << std::endl;
				i->func();
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

	return failed;
}
