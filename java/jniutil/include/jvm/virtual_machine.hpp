#ifndef JVM_VIRTUAL_MACHINE_INCLUDED
#define JVM_VIRTUAL_MACHINE_INCLUDED

#include <stdexcept>
#include <jni.h>

namespace jvm
{
	class virtual_machine
	{
		JavaVM *m_jvm;

	public:
		virtual_machine()
			: m_jvm(0) {}

		virtual_machine(const std::string &classPath)
			: m_jvm(0) { create(classPath); }

		~virtual_machine()
			{ destroy(); }

		void create(const std::string &classPath);
		void destroy();

		JNIEnv *env(JNIEnv *e = 0) const;

		jstring string(const std::string &v, JNIEnv *e = 0) const;
		jstring string(const std::wstring &v, JNIEnv *e = 0) const;
		std::wstring wstring(jstring v, JNIEnv *e = 0) const;
		std::string string(jstring v, JNIEnv *e = 0) const;

		void check_exception(JNIEnv *e = 0) const;
		void throw_exception(const std::string &msg, JNIEnv *e = 0) const;
		void throw_exception(const std::wstring &msg, JNIEnv *e = 0) const;
	};

	virtual_machine &global_vm();
	void create_global_vm(const std::string &classPath);
	virtual_machine *swap_global_vm(virtual_machine *vm);
	bool global_vm_available();

    struct global_init_enlist_base
    {
    public:
        virtual void startup() = 0;
        virtual void cleanup() = 0;
    };

    template <class T>
	class global_init_enlist : public global_init_enlist_base
	{
	    T *impl_;

	public:
	    global_init_enlist()
	        : impl_(0) { enlist(this); }

	    ~global_init_enlist()
	        { cleanup(); }

	    void startup()
	    {
	        if (impl_ == 0)
	            impl_ = new T();
	    }

	    void cleanup()
	    {
	        if (impl_ != 0)
            {
                T *d = impl_;
                impl_ = 0;
                delist(this);
                try { delete d; } catch (...) { }
            }
	    }
	};
	
	void enlist(global_init_enlist_base *init);
    void delist(global_init_enlist_base *init);
}

#endif
