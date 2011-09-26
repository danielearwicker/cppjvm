#ifndef JVM_ARRAY_INCLUDED
#define JVM_ARRAY_INCLUDED

#include <jvm/virtual_machine.hpp>
#include <jvm/object.hpp>

namespace jvm
{
	/* A traits class allows us to abstract over the inconsistencies in the
	   JNI APIs. The general traits for any type T are assumed to work on
	   the CppWrap-generated wrappers for any Java class, and so they use
	   the APIs for dealing with Object[] arrays.
	*/
	template <class T>
	struct array_traits
	{
		typedef jobjectArray array_type; // The JNI array type
		typedef T accessor_base_type; // Base for type to return from operator[]

		// Allocate an array (we assume we can get the jclass from T)
		static array_type alloc(JNIEnv *e, jsize n)
			{ return e->NewObjectArray(e, n, T::get_class(), 0); }

		// We assume a method T::get_impl() to obtain local reference jobjects
		static void put(JNIEnv *e, array_type a, jsize p, const T *b, jsize c)
		{
			// There are no APIs for bulk operations on object arrays, so we simulate them
			for (jsize n = 0; n < c; n++)
				e->SetObjectArrayElement(a, p + n, b[n].get_impl());
		}

		// We assume a method T::put_impl() to store the references
		static void get(JNIEnv *e, array_type a, jsize p, T *b, jsize c)
		{
			for (jsize n = 0; n < c; n++)
				b[n].put_impl(e->GetObjectArrayElement(a, p + n));
		}
	};

	/* Then we define specialised traits for the primitive types, which are 
	   somewhat simpler. They all follow an identical pattern so we can use
	   a macro to declare them.
	*/
	template <class T>
	class primitive_accessor
	{
		T val;

	public:
		void put_impl(T o) 
			{ val = o; }

		operator T()
			{ return val; }
	};

#define DECLARE_JARRAY_TRAITS(ELEM, FNAME) \
	template <> \
	struct array_traits<ELEM> \
	{ \
		typedef ELEM ## Array array_type; \
		typedef primitive_accessor<ELEM> accessor_base_type; \
		static array_type alloc(JNIEnv *e, jsize n) \
			{ return e->New ## FNAME ## Array(n); } \
		static void put(JNIEnv *e, array_type a, jsize p, const ELEM *b, jsize c) \
			{ e->Set ## FNAME ## ArrayRegion(a, p, c, b); } \
		static void get(JNIEnv *e, array_type a, jsize p, ELEM *b, jsize c) \
			{ e->Get ## FNAME ## ArrayRegion(a, p, c, b); } \
	}

	DECLARE_JARRAY_TRAITS(jbyte, Byte);
	DECLARE_JARRAY_TRAITS(jchar, Char);
	DECLARE_JARRAY_TRAITS(jboolean, Boolean);
	DECLARE_JARRAY_TRAITS(jshort, Short);
	DECLARE_JARRAY_TRAITS(jint, Int);
	DECLARE_JARRAY_TRAITS(jlong, Long);
	DECLARE_JARRAY_TRAITS(jfloat, Float);
	DECLARE_JARRAY_TRAITS(jdouble, Double);

	template <class T>
	class array : public object
	{
	public:
		typedef typename array_traits<T>::array_type array_type;
		typedef typename array_traits<T>::accessor_base_type accessor_base_type;

		array() {}

		explicit array(jobject ar)
			: object(ar) {}

		explicit array(jsize length)
			{ new_(length); }

		void new_(jsize length)
			{ put_impl(array_traits<T>::alloc(global_vm().env(), length)); }

		jsize length() const
			{ return global_vm().env()->GetArrayLength((array_type)get_impl()); }

		void put(int p, const T *v, size_t c = 1) const
			{ array_traits<T>::put(global_vm().env(), (array_type)get_impl(), p, v, c); }

		void get(int p, T *v, size_t c = 1) const
			{ array_traits<T>::get(global_vm().env(), (array_type)get_impl(), p, v, c); }

		class accessor_ : public accessor_base_type
		{
			const array<T> &data_;
			jsize index_;

		public:
			accessor_(const array<T> &array, jsize index)
				: data_(array), index_(index) 
			{
				T v;
				array.get(index, &v);
				accessor_base_type::put_impl(v);
			}

			accessor_ &operator=(const T &v)
			{
				accessor_base_type::put_impl(v);
				data_.put(index_, &v);
				return *this;
			}
		};

		accessor_ operator[](jsize index) const
			{ return accessor_(*this, index); }
		accessor_ operator[](int index) const // VC++2008 needs this when using int literals
			{ return accessor_(*this, index); }
	};
}

#endif
