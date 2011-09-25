#include <jvm/object.hpp>
#include <jvm/virtual_machine.hpp>

void jvm::object::make_global()
{
	jobject g = global_vm().env()->NewGlobalRef(m_object);
	global_vm().check_exception();
	if (g == 0)
		throw std::logic_error("Could not allocate global reference");

	m_object = g;
}

void jvm::object::delete_global()
{
	if (m_object != 0)
	{
		global_vm().env()->DeleteGlobalRef(m_object);
		m_object = 0;
	}
}
