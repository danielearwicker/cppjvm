#ifndef JVM_SYNCHRONIZED_INCLUDED
#define JVM_SYNCHRONIZED_INCLUDED

#include <jvm/virtual_machine.hpp>

namespace jvm
{
    class synchronized
	{
		JNIEnv *e;
		jobject l;

	public:
		synchronized(jobject lock, JNIEnv *env = 0)
			: e(global_vm().env(env)), l(lock)
		{
			if (e->MonitorEnter(l) != JNI_OK)
			    throw std::logic_error("MonitorEnter failed");
		}

		~synchronized()
			{ e->MonitorExit(l); }
	};
}

#endif
