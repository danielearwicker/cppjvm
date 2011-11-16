#include <impl/class_cache.hpp>
#include <jvm/virtual_machine.hpp>

/*
 * class_cache implementation
 */
 
void cppjvm::impl::class_cache::alloc() {
#ifdef ARRAY_BASED_CLASS_CACHE
	if (m_constructors)
		return;
	
	m_constructors = new jmethodID[m_constructorCount];
	m_methods = new jmethodID[m_methodCount];
	
	memset(m_constructors, 0, sizeof(jmethodID) * m_constructorCount);
	memset(m_methods, 0, sizeof(jmethodID) * m_methodCount);
#endif
}
cppjvm::impl::class_cache::class_cache(const char* className, int constructorCount, int methodCount) :
#ifdef ARRAY_BASED_CLASS_CACHE
	m_methodCount(methodCount), m_constructorCount(constructorCount),
	m_methods(NULL), m_constructors(NULL),
#endif
	m_className(className),
	m_class(NULL)
{
}
cppjvm::impl::class_cache::~class_cache() {
	JNIEnv* env = ::jvm::global_vm().env();
	if (env && m_class)
		env->DeleteGlobalRef(m_class);
	
#ifdef ARRAY_BASED_CLASS_CACHE
	delete[] m_constructors;
	delete[] m_methods;
#endif
}
jclass cppjvm::impl::class_cache::get_class(JNIEnv* env) {
	if (!m_class) {
		jclass cls;
		if (!env)
			env = ::jvm::global_vm().env();
		cls = env->FindClass(m_className);
		if (!cls)
			throw std::logic_error("Unknown class !");
		m_class = (jclass)env->NewGlobalRef(cls);
	}
	return m_class;
}
jmethodID cppjvm::impl::class_cache::getMethod(int index, MethodCacheType& cache/*, int cacheSize*/, const char* name, const char* signature, JNIEnv* env, bool isStatic)
{
	jmethodID m;
	
	//if (index < 0 || index >= cacheSize)
	//	throw std::logic_error("Invalid index in jmethodID cache array !");
	
	if (!(m = cache[index]))
	{
		jclass cls;
		if (!env)
			env = ::jvm::global_vm().env();
		cls = get_class(env);
		cache[index] = m = (isStatic ?
			env->GetStaticMethodID(cls, name, signature) :
			env->GetMethodID(cls, name, signature)
		);
	}
	if (!m)
		throw std::logic_error("Unknown method !");
	return m;
}

jmethodID cppjvm::impl::class_cache::getConstructor(int index, const char* signature, JNIEnv* env)
{
	alloc();
	return getMethod(index, m_constructors/*, m_constructorCount*/, "<init>", signature, env, false);
}
jmethodID cppjvm::impl::class_cache::getMethod(int index, const char* name, const char* signature, JNIEnv* env, bool isStatic)
{
	alloc();
	return getMethod(index, m_methods/*, m_methodCount*/, name, signature, env, isStatic);
}
jmethodID cppjvm::impl::class_cache::getStaticMethod(int index, const char* name, const char* signature, JNIEnv* env)
{
	return getMethod(index, name, signature, env, true);
}

#define CALL_IMPL(jtype, typeName) \
	jtype 	::cppjvm::impl::class_cache::Call ## typeName ## Method(int index, const char* name, const char* signature, jobject instance, ...) \
	{ \
		JNIEnv *env = ::jvm::global_vm().env(); \
		jmethodID m = getMethod(index, name, signature, env); \
		jtype ret; \
		va_list args; \
		va_start(args, instance); \
		ret = env->Call ## typeName ## MethodV(instance, m, args); \
		va_end(args); \
		::jvm::global_vm().check_exception(env); \
		return ret; \
	}
	
#define STATIC_CALL_IMPL(jtype, typeName) \
	jtype 	::cppjvm::impl::class_cache::CallStatic ## typeName ## Method(int index, const char* name, const char* signature, ...) \
	{ \
		JNIEnv *env = ::jvm::global_vm().env(); \
		jmethodID m = getStaticMethod(index, name, signature, env); \
		jclass c = get_class(env); \
		jtype ret; \
		va_list args; \
		va_start(args, signature); \
		ret = env->CallStatic ## typeName ## MethodV(c, m, args); \
		va_end(args); \
		::jvm::global_vm().check_exception(env); \
		return ret; \
	}
	
#define CALL_IMPLS(jtype, typeName) \
	CALL_IMPL(jtype, typeName); \
	STATIC_CALL_IMPL(jtype, typeName);
    
CALL_IMPLS(	jint 	,	Int      );
CALL_IMPLS(	jshort 	,	Short    );
CALL_IMPLS(	jlong 	,	Long     );
CALL_IMPLS(	jbyte 	,	Byte     );
CALL_IMPLS(	jchar 	,	Char     );
CALL_IMPLS(	jdouble	,	Double   );
CALL_IMPLS(	jfloat 	,	Float    );
CALL_IMPLS(	jobject	,	Object   );
CALL_IMPLS(	jboolean	,	Boolean	 );

/*
jobject 	::cppjvm::impl::class_cache::CallStaticObjectMethod(int index, const char* name, const char* signature, ...)
{
	JNIEnv *env = ::jvm::global_vm().env();
	jmethodID m = getStaticMethod(index, name, signature, env);
	jclass c = get_class(env);
	jobject ret;
	va_list args;
	va_start(args, signature);
	ret = env->CallStaticObjectMethodV(c, m, args);
	va_end(args);
	::jvm::global_vm().check_exception(env);
	return ret;
}

jobject 	::cppjvm::impl::class_cache::CallObjectMethod(int index, const char* name, const char* signature, jobject instance, ...)
{
	JNIEnv *env = ::jvm::global_vm().env();
	jmethodID m = getMethod(index, name, signature, env);
	jobject ret;
	va_list args;
	va_start(args, signature);
	ret = env->CallObjectMethodV(instance, m, args);
	va_end(args);
	::jvm::global_vm().check_exception(env);
	return ret;
}
*/
void 	::cppjvm::impl::class_cache::CallVoidMethod(int index, const char* name, const char* signature, jobject instance, ...)
{
	JNIEnv *env = ::jvm::global_vm().env();
	jmethodID m = getMethod(index, name, signature, env);
	va_list args;
	va_start(args, instance);
	env->CallVoidMethodV(instance, m, args);
	va_end(args);
	::jvm::global_vm().check_exception(env);
}

void 	::cppjvm::impl::class_cache::CallStaticVoidMethod(int index, const char* name, const char* signature, ...)
{
	JNIEnv *env = ::jvm::global_vm().env();
	jmethodID m = getStaticMethod(index, name, signature, env);
	jclass c = get_class(env);
	va_list args;
	va_start(args, signature);
	env->CallStaticVoidMethodV(c, m, args);
	va_end(args);
	::jvm::global_vm().check_exception(env);
}
       
jobject ::cppjvm::impl::class_cache::NewObject(int index, const char* signature, ...)
{
	JNIEnv *env = ::jvm::global_vm().env();
	jmethodID m = getConstructor(index, signature, env);
	jclass c = get_class(env);
	jobject ret;
	va_list args;
	va_start(args, signature);
	ret = env->NewObject(c, m, args);
	va_end(args);
	::jvm::global_vm().check_exception(env);
	return ret;
}
