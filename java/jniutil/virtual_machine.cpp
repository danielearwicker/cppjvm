#include <jvm/virtual_machine.hpp>
#include <jvm/object.hpp>

#include <util/utilutf8.h>
#include <util/utilwide.h>

void jvm::virtual_machine::create(const std::string &classPath)
{
	JavaVMInitArgs args;
	
	args.version = JNI_VERSION_1_2;
	args.nOptions = 1;

	std::string o("-Djava.class.path=");
	o += classPath;

	JavaVMOption options[1];
	options[0].optionString = const_cast<char *>(o.c_str());

	args.options = options;
	args.ignoreUnrecognized = JNI_FALSE;

	JNIEnv *junk;
	if (JNI_CreateJavaVM(&m_jvm, (void **)&junk, &args) < 0)
		throw std::logic_error("Could not create JavaVM");
}

void jvm::virtual_machine::destroy()
{
	if (m_jvm != 0)
	{
		m_jvm->DestroyJavaVM();
		m_jvm = 0;
	}
}

JNIEnv *jvm::virtual_machine::env(JNIEnv *e) const
{
	if (e == 0)
	{
	    if (m_jvm->AttachCurrentThread((void **)&e, 0) < 0)
		    throw std::logic_error("Could not attach thread to JavaVM");
	}
	return e;
}

void jvm::virtual_machine::check_exception(JNIEnv *e) const
{
    e = env(e);

	jthrowable x = e->ExceptionOccurred();
	if (x != 0)
	{
		e->ExceptionDescribe();
		e->ExceptionClear();

		std::string msg("(Exception message not available)");

        jmethodID getMessage = e->GetMethodID(e->GetObjectClass(x), "getMessage", "()Ljava/lang/String;");
		e->ExceptionClear();

		if (getMessage != 0)
		{
			jobject s = e->CallObjectMethod(x, getMessage);
			e->ExceptionClear();

			if (s != 0)
				msg = string((jstring)s);
		}

		throw std::logic_error(msg);
	}
}

void jvm::virtual_machine::throw_exception(const std::string &msg, JNIEnv *e) const
{
	static jclass excls = env(e)->FindClass("java/lang/Exception");
	env(e)->ThrowNew(excls, msg.c_str());
}

void jvm::virtual_machine::throw_exception(const std::wstring &msg, JNIEnv *e) const
{
	throw_exception(Util::ToMultiByte(msg), e);
}

std::wstring jvm::virtual_machine::wstring(jstring v, JNIEnv *e) const
{
	jboolean c = 0;
	const char *b = env(e)->GetStringUTFChars(v, &c);
	std::wstring r(Util::FromUTF8(b));
	env(e)->ReleaseStringUTFChars(v, b);
	return r;
}

std::string jvm::virtual_machine::string(jstring v, JNIEnv *e) const
{
	jboolean c = 0;
	const char *b = env(e)->GetStringUTFChars(v, &c);
	std::string r(b);
	env(e)->ReleaseStringUTFChars(v, b);
	return r;
}

jstring jvm::virtual_machine::string(const std::string &v, JNIEnv *e) const
{
	return string(Util::ToWideChar(v), e);
}

jstring jvm::virtual_machine::string(const std::wstring &v, JNIEnv *e) const
{
	return env(e)->NewStringUTF(Util::ToUTF8(v).c_str());
}

jvm::virtual_machine *g_vm = 0;
jvm::virtual_machine g_vm_default;

void jvm::create_global_vm(const std::string &classPath)
{
	if (g_vm != 0)
		throw std::logic_error("Global virtual machine already initialized");

	g_vm_default.create(classPath);
	g_vm = &g_vm_default;
}

jvm::virtual_machine &jvm::global_vm()
{
	if (g_vm == 0)
		throw std::logic_error("No virtual machine available");

	return *g_vm;
}

jvm::virtual_machine *jvm::swap_global_vm(jvm::virtual_machine *vm)
{
	jvm::virtual_machine *o = g_vm;
	g_vm = vm;
	return o;
}

bool jvm::global_vm_available()
{
	return (g_vm != 0);
}
