
package com.earwicker.cppjvm.cppwrap;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

public class ImplementationGenerator extends SourceGenerator {

    int suffix = 0; // in case I ever want multiple definitions per file

    public void generate() throws Exception {
        include(cls()); // include our own header, obviously
        includeRequiredTypes();
        beginNamespace(cls());
        classInfo();
        defineConstructors();
        defineConversions();
        defineMethods();
        defineFields();
        endNamespace(cls());
    }

    protected void classInfo() throws Exception {
        out().println("static jclass cached_class" + suffix + " = 0;");
        out().println("jclass " + cls().getSimpleName() + "::get_class()");
        out().println("{");
        out().println("    if (cached_class" + suffix + " == 0)");
        out().println("    {");
        out().print("        cached_class" + suffix + " = ::jvm::global_vm().env()->FindClass(\"");
        out().print(cls().getName().replace('.', '/')); // what about nested classes?
        out().println("\");");
        out().println("        cached_class" + suffix + " = (jclass)::jvm::global_vm().env()->NewGlobalRef(cached_class" + suffix + ");");
        out().println("    }");
        out().println("    return cached_class" + suffix + ";");
        out().println("}");

        out().println("static jfieldID cached_fields" + suffix + "[" + (cls().getFields().length + 1) + "];");
        out().println("static jmethodID cached_constructors" + suffix + "[" + (cls().getConstructors().length + 1) + "];");
        out().println("static jmethodID cached_methods" + suffix + "[" + (cls().getMethods().length + 1) + "];");
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

            out().println("    jmethodID i = cached_constructors" + suffix + "[" + pos + "];");
            out().println("    if (i == 0)");
            out().println("    {");
            out().println("        i = env->GetMethodID(get_class(), \"<init>\", \"" + 
                          Signature.generate(ctor) + "\");");
            out().println("        cached_constructors" + suffix + "[" + pos + "] = i;");
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

            out().println("    jmethodID i = cached_methods" + suffix + "[" + pos + "];");
            out().println("    if (i == 0)");
            out().println("    {");
            out().println("        i = env->Get" +
                (isStatic ? "Static" : "") +
                "MethodID(get_class(), \"" + m.getName() + 
                "\", \"" + Signature.generate(m) + "\");");
            out().println("        cached_methods" + suffix + "[" + pos + "] = i;");
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
    
    void defineFields() throws Exception {
        int pos = 0;
        for (Field f : cls().getFields()) {
            if (isFieldHidden(f))
                continue;
                
            boolean isStatic = Modifier.isStatic(f.getModifiers());

            out().println("static jfieldID getFieldId_" + f.getName() + "(JNIEnv *env)");
            out().println("{");
            out().println("    jfieldID i = cached_fields" + suffix + "[" + pos + "];");
            out().println("    if (i == 0)");
            out().println("    {");
            out().println("        i = env->Get" +
                (isStatic ? "Static" : "") +
                "FieldID(" + cls().getSimpleName() + 
                "::get_class(), \"" + f.getName() + 
                "\", \"" + Signature.generate(f.getType()) + "\");");
            out().println("        cached_fields" + suffix + "[" + pos + "] = i;");
            out().println("    }");
            out().println("    return i;");
            out().println("}");

            String fieldFlavour = f.getType().isPrimitive() 
                ? (Character.toUpperCase(f.getType().toString().charAt(0)) + 
                   f.getType().toString().substring(1))
                : "Object";
        
            out().println(
                CppWrap.cppType(f.getType()) + " " +                 
                cls().getSimpleName() + "::get_" + 
                CppWrap.fixName(f.getName()) + "()" +
                (isStatic ? "" : "const")
            );
            out().println("{");
            out().println("    JNIEnv *env = ::jvm::global_vm().env();");
            out().println("    " + 
                CppWrap.cppType(f.getType()) + " ret" + 
                (CppWrap.isWrapped(f.getType()) ? "(" : " = ") + "env->Get" + 
                (isStatic ? "Static" : "") + fieldFlavour + 
                (isStatic ? "Field(get_class" : "Field(::jvm::object::get_impl") +
                "(), getFieldId_" + f.getName() + "(env)" + 
                (CppWrap.isWrapped(f.getType()) ? "));" : ");"));

            out().println(CppWrap.isWrapped(f.getType()) 
                ? "    return ret;"
                : "    return " + CppWrap.cppType(f.getType()) + "(ret);");
            out().println("}");
            
            out().print("void " +
                cls().getSimpleName() + "::set_" + 
                CppWrap.fixName(f.getName()) + "(");
            listParameters(new Class<?>[] { f.getType() }, DECLARE_TYPES);
            out().println(isStatic ? ")" : ") const");
            out().println("{");
            out().println("    JNIEnv *env = ::jvm::global_vm().env();");
            out().print("    env->Set" + 
                (isStatic ? "Static" : "") + fieldFlavour + 
                (isStatic ? "Field(get_class" : "Field(::jvm::object::get_impl") +
                "(), getFieldId_" + f.getName() + "(env), ");
            listParameters(new Class<?>[] { f.getType() }, CALL_UNWRAPPED);
            out().println(");");
            out().println("}");

            pos++;
        }
    }
}


