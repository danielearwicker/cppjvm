
package com.earwicker.cppjvm.cppwrap;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

public class ImplementationGenerator extends SourceGenerator {

    public void generate() throws Exception {
        include(cls()); // include our own header, obviously
        if (putDefinitionsInHeaders)
        		out().println("#include <impl/class_cache.hpp>");
        
        includeRequiredTypes();
        beginNamespace(cls());
        classInfo();
        if (!putDefinitionsInHeaders) {
			defineConstructors();
			defineMethods();
		}
		defineConversions();
        endNamespace(cls());
    }

    protected void includeRequiredTypes() throws Exception {
        for (Class<?> required : CppWrap.getDirectlyRequiredTypes(cls())) {
            include(required);
        }
    }

    protected void classInfo() throws Exception {
    		Class c = cls();
    			
    		if (putDefinitionsInHeaders)
    			out().print(c.getSimpleName() + "::");
    		else
    			out().print("static cppjvm::impl::class_cache ");
    		out().println("s_impl_cache(" +
    			"\"" + c.getName().replace('.', '/') + "\", " + 
    			c.getConstructors().length + ", " + 
    			c.getMethods().length + 
		");");
        out().println("jclass " + c.getSimpleName() + "::get_class()");
        out().println("{");
        out().println("    return s_impl_cache.get_class();");
        out().println("}");
    }

    protected void defineConstructors() throws Exception {
        int pos = 0;
        for (Constructor<?> ctor : cls().getConstructors()) {
            // void ClassName::new_(params...)
            out().print("void " + (isInHeader() ? "" : cls().getSimpleName() + "::") + "new_(");
            listParameters(ctor.getParameterTypes(), DECLARE_TYPES);
            out().println(")");

            out().println("{");
            out().print(
                "    ::jvm::object::put_impl(s_impl_cache.NewObject(" + pos + ", \"" + Signature.generate(ctor) + "\"" + 
                (ctor.getParameterTypes().length != 0 ? ", " : "")
            );
            listParameters(ctor.getParameterTypes(), CALL_UNWRAPPED);
            out().println("));");
            //out().println("    ::jvm::global_vm().check_exception(env);");
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

    protected boolean isInHeader() {
    		return false;
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
                (isInHeader() ? "" : cls().getSimpleName() + "::") +
                CppWrap.fixName(m.getName()) + "(");
            listParameters(params, DECLARE_TYPES);
            out().println(isStatic ? ")" : ") const");

            out().println("{");
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
                "s_impl_cache.Call" +
                (isStatic ? "Static" : "") + returnFlavour + "Method(" + 
					pos + ", " +
				"\"" + m.getName() + "\", " +
				"\"" + Signature.generate(m) + "\"" + 
				(isStatic ? "" : ", ::jvm::object::get_impl()") +
                (params.length != 0 ? ", " : "")
            );
            listParameters(params, CALL_UNWRAPPED);
            out().println(CppWrap.isWrapped(returns) ? "));" : ");");
            
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


