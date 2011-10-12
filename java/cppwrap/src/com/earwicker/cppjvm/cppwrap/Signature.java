
package com.earwicker.cppjvm.cppwrap;

import java.lang.reflect.*;

public class Signature 
{	
    public static String generate(Class[] p, String r)
    {
        StringBuffer b =  new StringBuffer();

        b.append("(");

        for (int n = 0; n < p.length; n++)
            b.append(generate(p[n]));

        b.append(")");
        b.append(r);
        return b.toString();
    }

    public static String generate(Method m)
    {
        return generate(m.getParameterTypes(), 
                        generate(m.getReturnType()));
    }

    public static String generate(Constructor m)
        { return generate(m.getParameterTypes(), "V"); }

    private static java.util.HashMap<String, String> primitives;

    static
    {
        primitives = new java.util.HashMap<String, String>();
        primitives.put("void", "V");
        primitives.put("boolean", "Z");
        primitives.put("int", "I");
        primitives.put("byte", "B");
        primitives.put("short", "S");
        primitives.put("long", "J");
        primitives.put("double", "D");
        primitives.put("float", "F");
        primitives.put("char", "C");
    }

    public static String generate(Class c)
    {
        if (c.isPrimitive())
            return primitives.get(c.toString());
        else if (c.isArray())
            return "[" + generate(c.getComponentType());
        else
            return "L" + (c.getName()).replace('.', '/') + ";";
    }
}
