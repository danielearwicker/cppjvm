#include <jvm/virtual_machine.hpp>
#include <jvm/object.hpp>
#include <jvm/local_frame.hpp>

#include <util/utilutf8.h>
#include <util/utilwide.h>

#include <vector>
#include <algorithm>

void jvm::virtual_machine::create(const std::string &classPath)
{
	JavaVMInitArgs args;
	
	args.version = JNI_VERSION_1_2;
	
	std::string o("-Djava.class.path=");

	std::string fixedPath(classPath);
	for (size_t n = 0; n < fixedPath.size(); n++)
	{
		if (fixedPath[n] == '/' || fixedPath[n] == '\\')
		{
#ifdef CONFIG_VARIANT_windows
			#define realpath(N,R) _fullpath((R),(N),1024)
			fixedPath[n] = '\\';
#else
			fixedPath[n] = '/';
#endif
		}
	}

	char chFull[1024];
	realpath(fixedPath.c_str(), chFull);
	o += chFull;

	JavaVMOption options[1];
	options[0].optionString = const_cast<char *>(o.c_str());
	//options[1].optionString = "-verbose:jni";

	args.nOptions = sizeof(options) / sizeof(JavaVMOption);
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

	local_frame lf(16, e);

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

static std::vector<jvm::global_init_enlist_base *> *g_inits = 0;
static std::vector<jvm::global_init_enlist_base *> &global_inits()
{
    if (g_inits == 0)
        g_inits = new std::vector<jvm::global_init_enlist_base *>;

    return *g_inits;
}

void global_init_startup()
{
    for (int n = 0; n < global_inits().size(); n++)
        global_inits()[n]->startup();
}

// declared after g_vm_default, so will destruct before
struct global_init_cleanup
{
    ~global_init_cleanup()
    {
        // each will delist itself
        while (!global_inits().empty())
            global_inits()[0]->cleanup();
    }
}
global_init_cleanup;

void jvm::enlist(global_init_enlist_base *init)
{
    global_inits().push_back(init);
}

void jvm::delist(global_init_enlist_base *init)
{
    global_inits().erase(
        std::remove(
            global_inits().begin(), 
            global_inits().end(), init),
        global_inits().end());
}

void jvm::create_global_vm(const std::string &classPath)
{
	if (g_vm != 0)
		throw std::logic_error("Global virtual machine already initialized");

	g_vm_default.create(classPath);
	g_vm = &g_vm_default;
	
	global_init_startup();
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
