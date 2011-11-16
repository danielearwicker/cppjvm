#ifndef CPPJVM_IMPL_CLASS_CACHE_INCLUDED
#define CPPJVM_IMPL_CLASS_CACHE_INCLUDED

//#include <stdexcept>
#include <jni.h>
#include <map>

//#define ARRAY_BASED_CLASS_CACHE

namespace cppjvm
{
namespace impl
{
	class class_cache 
	{
#ifdef ARRAY_BASED_CLASS_CACHE
		typedef jmethodID* MethodCacheType;
        int m_methodCount, m_constructorCount;
#else
		typedef std::map<int, jmethodID> MethodCacheType;
#endif
        MethodCacheType m_constructors, m_methods;
        
        const char* m_className;
        jclass m_class;
        
        void alloc();
        jmethodID getMethod(int index, MethodCacheType& cache/*, int cacheSize*/, const char* name, const char* signature, JNIEnv* env, bool isStatic);
        
        jmethodID getConstructor(int index, const char* signature, JNIEnv* env = NULL);
        jmethodID getMethod(int index, const char* name, const char* signature, JNIEnv* env = NULL, bool isStatic = false);
        jmethodID getStaticMethod(int index, const char* name, const char* signature, JNIEnv* env = NULL);
        
        
    public:
        class_cache(const char* className, int constructorCount, int methodCount);
        ~class_cache();
        
        jclass get_class(JNIEnv* env = NULL);
        
        jobject NewObject(int index, const char* signature, ...);
        
        jint 	CallIntMethod(int index, const char* name, const char* signature, jobject instance, ...);
        jshort 	CallShortMethod(int index, const char* name, const char* signature, jobject instance, ...);
        jlong 	CallLongMethod(int index, const char* name, const char* signature, jobject instance, ...);
        jbyte 	CallByteMethod(int index, const char* name, const char* signature, jobject instance, ...);
        jchar 	CallCharMethod(int index, const char* name, const char* signature, jobject instance, ...);
        jboolean	CallBooleanMethod(int index, const char* name, const char* signature, jobject instance, ...);
        jdouble 	CallDoubleMethod(int index, const char* name, const char* signature, jobject instance, ...);
        jfloat 	CallFloatMethod(int index, const char* name, const char* signature, jobject instance, ...);
        void 	CallVoidMethod(int index, const char* name, const char* signature, jobject instance, ...);
        jobject 	CallObjectMethod(int index, const char* name, const char* signature, jobject instance, ...);
        
        jint 	CallStaticIntMethod(int index, const char* name, const char* signature, ...);
        jshort 	CallStaticShortMethod(int index, const char* name, const char* signature, ...);
        jlong 	CallStaticLongMethod(int index, const char* name, const char* signature, ...);
        jbyte 	CallStaticByteMethod(int index, const char* name, const char* signature, ...);
        jchar 	CallStaticCharMethod(int index, const char* name, const char* signature, ...);
        jboolean	CallStaticBooleanMethod(int index, const char* name, const char* signature, ...);
        jdouble 	CallStaticDoubleMethod(int index, const char* name, const char* signature, ...);
        jfloat 	CallStaticFloatMethod(int index, const char* name, const char* signature, ...);
        void 	CallStaticVoidMethod(int index, const char* name, const char* signature, ...);
        jobject 	CallStaticObjectMethod(int index, const char* name, const char* signature, ...);
	};
}
}

#endif // CPPJVM_IMPL_CLASS_CACHE_INCLUDED
