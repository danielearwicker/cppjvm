
package com.earwicker.cppjvm.cppwrap;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

public class HeaderGenerator extends SourceGenerator {

    public void generate() throws Exception {
        beginIncludeGuard();
        globalIncludes();
        includeRequiredTypes();
        forwardDeclareRequiredTypes(); // some circular includes will not have worked
        beginNamespace(cls());
        
        beginClass();
        declareConstructors();
        declareConversions();
        declareMethods();
        declareFields();
        declareSpecialStringFeatures();
        endClass();
        
        endNamespace(cls());
        endIncludeGuard();
    }

    protected void forwardDeclare(Class<?> cls) {
        if (!CppWrap.isWrapped(cls))
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
    }

    protected void forwardDeclareRequiredTypes() throws Exception {
        for (Class<?> required : CppWrap.getDirectlyRequiredTypes(cls())) {
            forwardDeclare(required);
        }
    }

    protected void beginClass() {
        out().println();
        out().println("class " + cls().getSimpleName() + " : public ::jvm::object");
        out().println("{");
        out().println("public:");

        // Can construct from a jobject, but not implicitly to avoid accidental unsafe conversion
        out().println("    explicit " + cls().getSimpleName() + "(jobject jobj) : object(jobj) {}");

        out().println("    static jclass get_class();");

        // Default constructor: null reference
        out().println("    " + cls().getSimpleName() + "() { }");

        // Copy constructor: copy reference
        out().println("    " + cls().getSimpleName() + "(const " + cls().getSimpleName() + " &other)");
        out().println("        : object(other.get_impl()) {}");
    }

    protected void endClass() {
        out().println("};");
    }

    protected void declareConstructors() throws Exception {
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

    protected void declareConversions() throws Exception {
        for (Class<?> st : CppWrap.getSuperTypes(cls())) {
            out().println("    operator " + CppWrap.cppType(st) + "() const;");
        }
    }

    protected void declareMethods() throws Exception {
        for (Method m : cls().getMethods()) {
            if (m.isSynthetic())
                continue;

            // [static] return-type methodName(params...) [const];
            out().print("    " + 
                (Modifier.isStatic(m.getModifiers()) ? "static " : "") + 
                CppWrap.cppType(m.getReturnType()) + " " + 
                CppWrap.fixName(m.getName()) + "(");
            listParameters(m.getParameterTypes(), DECLARE_TYPES);
            out().println(Modifier.isStatic(m.getModifiers()) ? ");" : ") const;");
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

    void declareFields() throws Exception {
        for (Field f : cls().getFields()) {
            if (isFieldHidden(f))
                continue;

            if (f.getType().isPrimitive() &&
                !f.getType().equals(Double.TYPE) &&
                !f.getType().equals(Float.TYPE) &&
                Modifier.isStatic(f.getModifiers()) &&
                Modifier.isFinal(f.getModifiers())) {

                String val = f.get(null).toString();
                
                if (f.getType().equals(Character.TYPE)) {
                    val = "" + (int)val.charAt(0);
                }
                
                out().print("    static const " + 
                    CppWrap.cppType(f.getType()) + " " + 
                    CppWrap.fixName(f.getName()) + " = " +
                    val);
                
                if(f.getType().equals(Long.TYPE)) {
                	out().print("LL");
                }
                
                out().println(";");
            }
            out().println("    " + 
                (Modifier.isStatic(f.getModifiers()) ? "static " : "") + 
                CppWrap.cppType(f.getType()) + " get_" + 
                CppWrap.fixName(f.getName()) + "()" +
                (Modifier.isStatic(f.getModifiers()) ? ";" : "const;")
            );
            out().print("    " + 
                (Modifier.isStatic(f.getModifiers()) ? "static void set_" : "void set_") + 
                CppWrap.fixName(f.getName()) + "(");
            listParameters(new Class<?>[] { f.getType() }, DECLARE_TYPES);
            out().println(Modifier.isStatic(f.getModifiers()) ? ");" : ") const;");
        }
    }    
}

