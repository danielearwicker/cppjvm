#ifndef JVM_LOCAL_FRAME_INCLUDED
#define JVM_LOCAL_FRAME_INCLUDED

#include <jvm/virtual_machine.hpp>

namespace jvm
{
	/*	A local_frame should be declared at the start of any function, or in any
		loop body that acquires references to java objects. Unless this is done,
		those objects will not be eligible for garbage collection.

		Of course this also means that it is not safe to allow ordinary references
		to live longer than the local_frame in which they were acquired, as they
		may then be collected at an unpredictable time.

		To preserve references outside of the local_frame in which they are
		acquired, use the jvm::global<T> wrapper.
	*/
	class local_frame
	{
		JNIEnv *e;
		jobject r;

	public:
		local_frame(jint capacity = 16, JNIEnv *env = 0)
			: r(0)
		{
			e = env;
			if (e == 0)
				e = global_vm().env();

			e->PushLocalFrame(capacity);
		}

		local_frame &operator=(jobject result)
			{ r = result; }

		~local_frame()
			{ e->PopLocalFrame(r); }
	};
}

#endif
