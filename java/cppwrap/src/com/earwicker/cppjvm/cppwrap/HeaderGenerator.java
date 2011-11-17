
package com.earwicker.cppjvm.cppwrap;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

public class HeaderGenerator extends SourceGenerator {
	public void generate() throws Exception {
        beginIncludeGuard();
        globalIncludes();
        includeParentType();
        forwardDeclareRequiredTypes();
        beginNamespace(cls());
        beginClass();
        declareConstructors();
        declareConversions();
        declareMethods();
        declareSpecialStringFeatures();
        endClass();
        endNamespace(cls());
        endIncludeGuard();
    }

    protected void forwardDeclare(Class<?> cls) {
        if (!CppWrap.shouldGenerate(cls) || !CppWrap.isWrapped(cls))
            return;

        beginNamespace(cls);
        out().print(" class " + cls.getSimpleName() + ";");
        endNamespace(cls);
        out().println();
    }

    protected void beginIncludeGuard() {
        String guard = "INCLUDED_CPPJVM_" + cls().getName().replace('.', '_').toUpperCase();
        out().println("#ifndef " + guard);
        out().println("#define " + guard);
    }

    protected void endIncludeGuard() {
        out().println("#endif");
    }

    protected void globalIncludes() {
        out().println("#include <jvm/virtual_machine.hpp>");
        out().println("#include <jvm/object.hpp>");
        out().println("#include <jvm/array.hpp>");
        if (!putDefinitionsInHeaders)
        		out().println("#include <impl/class_cache.hpp>");
    }
    
    protected void includeParentType() {
    		Class superclass = cls().getSuperclass();
    		if (superclass == null)
    			return;
    		
    		out().println("#include <" + superclass.getName().replace('.', '/') + ".hpp>");
    }

    protected void forwardDeclareRequiredTypes() throws Exception {
        for (Class<?> required : CppWrap.getDirectlyRequiredTypes(cls())) {
            forwardDeclare(required);
        }
    }

    protected void beginClass() {
        out().println();
        Class superclass = cls().getSuperclass();
        String superclassCppName;
        if (superclass == null) {
        		superclassCppName = "::jvm::object";
        } else {
        		superclassCppName = CppWrap.cppType(superclass);//"::" + superclass.getName().replaceAll("\\.", "::");
        }
        out().println("class " + cls().getSimpleName() + " : public " + superclassCppName);
        out().println("{");
        if (putDefinitionsInHeaders)
        		out().println("    static cppjvm::impl::class_cache s_impl_cache;");
        out().println("public:");

        // Can construct from a jobject, but not implicitly to avoid accidental unsafe conversion
        out().println("    explicit " + cls().getSimpleName() + "(jobject jobj) : " + superclassCppName + "(jobj) {}");

        out().println("    static jclass get_class();");

        // Default constructor: null reference
        out().println("    " + cls().getSimpleName() + "() { }");

        // Copy constructor: copy reference
        out().println("    " + cls().getSimpleName() + "(const " + cls().getSimpleName() + " &other)");
        out().println("        : " + superclassCppName + "(other.get_impl()) {}");
    }

    protected void endClass() {
        out().println("};");
    }

    protected void declareConstructors() throws Exception {
        if (putDefinitionsInHeaders) {
			out().println(indent(new ImplementationGenerator() {
				@Override
				public void generate() throws Exception {
					defineConstructors();
				}
				@Override
				protected boolean isInHeader() {
					return true;
				}
				@Override
				protected void editWarning() {}
			}.toString(cls())));
		} else {
            	for (Constructor<?> ctor : cls().getConstructors()) {
				Class<?>[] params = ctor.getParameterTypes();
	
				// void new_(params...);
					out().print("    void new_(");
				listParameters(params, DECLARE_TYPES);
				out().println(");");
			
				// For non-default and non-copy constructors only:
				if ((params.length > 1) || 
					((params.length == 1) && !params[0].equals(cls()))) {
	
					// Make an actual C++ constructor
					out().print("    explicit " + cls().getSimpleName() + "(");
					listParameters(params, DECLARE_TYPES);
					out().println(")");
					out().println("    {");
					out().print("        new_(");
					listParameters(params, CALL_WRAPPED);
					out().println(");");
					out().println("    }");
				}
			}
		}
    }

    protected void declareConversions() throws Exception {
    		String dummy = CppWrap.cppType(Object.class);
        for (Class<?> st : CppWrap.getSuperTypes(cls())) {
        		String conv = CppWrap.cppType(st);
        		if (conv.equals(dummy))
        			continue;
			
        		out().println("    operator " + CppWrap.cppType(st) + "() const;");
        }
    }

    protected void declareMethods() throws Exception {
    		if (putDefinitionsInHeaders)
    			out().println(indent(new ImplementationGenerator() {
				@Override
				public void generate() throws Exception {
					defineMethods();
				}
				@Override
				protected boolean isInHeader() {
					return true;
				}
				@Override
				protected void editWarning() {}
			}.toString(cls())));
    		else {
			for (Method m : methods()) {
				if (m.isSynthetic())
					continue;
	
				// [static] return-type methodName(params...) [const];
				out().print("    " + 
					(Modifier.isStatic(m.getModifiers()) ? "static " : ""/*"virtual "*/) + 
					CppWrap.cppType(m.getReturnType()) + " " + 
					CppWrap.fixName(m.getName()) + "(");
				listParameters(m.getParameterTypes(), DECLARE_TYPES);
				out().println(Modifier.isStatic(m.getModifiers()) ? ");" : ") const;");
			}
		}
    }
    
    protected void declareSpecialStringFeatures() {
        if (!cls().equals(String.class))
            return;

        out().println("    String(const ::std::string &src)");
        out().println("        { put_impl(::jvm::global_vm().string(src)); }");
        out().println("    String(const ::std::wstring &src)");
        out().println("        { put_impl(::jvm::global_vm().string(src)); }");
        out().println("    String(const char *src)");
        out().println("        { put_impl(::jvm::global_vm().string(src)); }");
        out().println("    String(const wchar_t *src)");
        out().println("        { put_impl(::jvm::global_vm().string(src)); }");
        out().println("    ::std::string str()");
        out().println("        { return ::jvm::global_vm().string((jstring)get_impl()); }");
        out().println("    ::std::wstring wstr()");
        out().println("        { return ::jvm::global_vm().wstring((jstring)get_impl()); }");
        out().println("    operator ::std::string()");
        out().println("        { return ::jvm::global_vm().string((jstring)get_impl()); }");
        out().println("    operator ::std::wstring()");
        out().println("        { return ::jvm::global_vm().wstring((jstring)get_impl()); }");
    }
}

