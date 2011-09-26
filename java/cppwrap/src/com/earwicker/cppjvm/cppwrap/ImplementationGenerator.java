
package com.earwicker.cppjvm.cppwrap;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

public class ImplementationGenerator extends SourceGenerator {

    public void generate() throws Exception {
        include(cls()); // include our own header, obviously
        includeRequiredTypes();
        beginNamespace(cls());
        classInfo();
        defineConstructors();
        defineConversions();
        defineMethods();
        endNamespace(cls());
    }

    protected void includeRequiredTypes() throws Exception {
        for (Class<?> required : CppWrap.getDirectlyRequiredTypes(cls())) {
            include(required);
        }
    }

    protected void classInfo() throws Exception {
        out().println("static jclass cached_class = 0;");
        out().println("jclass " + cls().getSimpleName() + "::get_class()");
        out().println("{");
        out().println("    if (cached_class == 0)");
        out().println("    {");
        out().print("        cached_class = ::jvm::global_vm().env()->FindClass(\"");
        out().print(cls().getName().replace('.', '/'));
        out().println("\");");
        out().println("        cached_class = (jclass)::jvm::global_vm().env()->NewGlobalRef(cached_class);");
        out().println("    }");
        out().println("    return cached_class;");
        out().println("}");
        
        out().println("static jmethodID cached_constructors[" + (cls().getConstructors().length + 1) + "];");
        out().println("static jmethodID cached_methods[" + (cls().getMethods().length + 1) + "];");
    }

    protected void defineConstructors() throws Exception {
        int pos = 0;
        for (Constructor<?> ctor : cls().getConstructors()) {
            // void ClassName::new_(params...)
            out().print("void " + cls().getSimpleName() + "::new_(");
            listParameters(ctor.getParameterTypes(), DECLARE_TYPES);
            out().println(")");

            out().println("{");
            out().println("    JNIEnv *env = ::jvm::global_vm().env();");

            out().println("    jmethodID i = cached_constructors[" + pos + "];");
            out().println("    if (i == 0)");
            out().println("    {");
            out().println("        i = env->GetMethodID(get_class(), \"<init>\", \"" + 
                          Signature.generate(ctor) + "\");");
            out().println("        cached_constructors[" + pos + "] = i;");
            out().println("    }");
            out().print(
                "    ::jvm::object::put_impl(env->NewObject(get_class(), i" + 
                (ctor.getParameterTypes().length != 0 ? ", " : "")
            );
            listParameters(ctor.getParameterTypes(), CALL_UNWRAPPED);
            out().println("));");
            out().println("    ::jvm::global_vm().check_exception(env);");
            out().println("}");
            
            pos++;
        }
    }

    protected void defineConversions() throws Exception {
        for (Class<?> st : CppWrap.getSuperTypes(cls())) {
            out().println(cls().getSimpleName() + "::operator " + CppWrap.cppType(st) + "() const");
        	out().println("{");
        	out().println("    return " + CppWrap.cppType(st) + "(get_impl());");
        	out().println("}");
        }
    }

    protected void defineMethods() throws Exception {
        int pos = 0;
        for (Method m : cls().getMethods()) {
            if (m.isSynthetic())
                continue;

            Class<?> returns = m.getReturnType();
            boolean returnsVoid = returns.equals(void.class); 
            boolean isStatic = Modifier.isStatic(m.getModifiers());
            Class<?>[] params = m.getParameterTypes();

            // return-type ClassName::methodName(params...) [const]
            out().print(CppWrap.cppType(returns) + " " + 
                cls().getSimpleName() + "::" +
                CppWrap.fixName(m.getName()) + "(");
            listParameters(params, DECLARE_TYPES);
            out().println(isStatic ? ")" : ") const");

            out().println("{");
            out().println("    JNIEnv *env = ::jvm::global_vm().env();");

            out().println("    jmethodID i = cached_methods[" + pos + "];");
            out().println("    if (i == 0)");
            out().println("    {");
            out().println("        i = env->Get" +
                (isStatic ? "Static" : "") +
                "MethodID(get_class(), \"" + m.getName() + 
                "\", \"" + Signature.generate(m) + "\");");
            out().println("        cached_methods[" + pos + "] = i;");
            out().println("    }");

            String returnFlavour = returns.isPrimitive() 
                ? (Character.toUpperCase(returns.toString().charAt(0)) + 
                   returns.toString().substring(1))
                : "Object";

            // [cpp-type ret = ]env->Call[Static]return-flavourMethod(...        
            out().print("    " + 
                (returnsVoid 
                    ? "" 
                    : (CppWrap.cppType(returns) + " ret" + (CppWrap.isWrapped(returns) ? "(" : " = "))
                ) +
                "env->Call" + 
                (isStatic ? "Static" : "") + returnFlavour + 
                (isStatic ? "Method(get_class(), i" : "Method(::jvm::object::get_impl(), i") + 
                (params.length != 0 ? ", " : "")
            );
            listParameters(params, CALL_UNWRAPPED);
            out().println(CppWrap.isWrapped(returns) ? "));" : ");");
            out().println("    ::jvm::global_vm().check_exception(env);");

            if (!returnsVoid) {
                out().println(CppWrap.isWrapped(returns) 
                    ? "    return ret;"
                    : "    return " + CppWrap.cppType(returns) + "(ret);");
            }

            out().println("}");
            pos++;
        }
    }
}


