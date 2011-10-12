#ifndef JVM_GLOBAL_INCLUDED
#define JVM_GLOBAL_INCLUDED

#include <jvm/virtual_machine.hpp>

namespace jvm
{
	/*	Whenever local references to objects are exposed through JNI, they
		are added to a list associated with the top-most JVM stack frame.

		When execution exists that stack frame, all local references on the
		list are deleted, meaning that the objects they referred to can now
		be garbage collected (though not necessarily immediately).

		If you need an object to stay alive regardless, you should store
		it in a reference wrapped with global<T>, e.g. 

			jvm::global<InputStream>

		Use the local_frame class to create your own JVM stack frames.

		For example:

			jvm::global<InputStream> i;

			{
				jvm::local_frame lf;
				i = (obtain input stream somehow)
			}

			// i can still be used safely here

		Global references work somewhat like ref-counted smart pointers.
	*/
	template <class T>
	class global : public T
	{
	public:
		global() { }

		global(const global<T> &o)
			{ put_impl(o.get_impl()); }

		explicit global(const T &o)
			{ put_impl(o.get_impl()); }

		// Destructor deletes the reference
		~global()
			{ T::delete_global(); }

		global<T> &operator=(const T &o)
		{ 
			put_impl(o.get_impl());
			return *this;
		}

		global<T> &operator=(const global<T> &o)
		{ 
			put_impl(o.get_impl());
			return *this;
		}

        void set_null()
            { put_impl(0); }

		jobject get_impl() const
			{ return global_vm().env()->NewLocalRef(T::get_impl()); }

		// however a reference is stored, it goes via this,
		// ensuring it gets traded for a unique global reference
		void put_impl(jobject o)
		{
			T::delete_global();
			T::put_impl(o);
			T::make_global();
		}
	};
}

#endif
