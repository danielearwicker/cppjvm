
package com.earwicker.cppjvm.cppwrap;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

public class CppWrap {
    private static java.util.HashMap<Class<?>, String> primitives;
    private static java.util.HashSet<String> reserved;

    static {
        reserved = new java.util.HashSet<String>();
        reserved.add("delete");
        reserved.add("union");

        primitives = new java.util.HashMap<Class<?>, String>();
        primitives.put(void.class, "void");
        primitives.put(boolean.class, "jboolean");
        primitives.put(int.class, "jint");
        primitives.put(byte.class, "jbyte");
        primitives.put(short.class, "jshort");
        primitives.put(long.class, "jlong");
        primitives.put(double.class, "jdouble");
        primitives.put(float.class, "jfloat");
        primitives.put(char.class, "jchar");
    }

    
    public static String cppType(Class<?> j) throws Exception {
        if (j == null)
            return "jobject";

        if (j.isPrimitive()) {
            String g = primitives.get(j);
            return g == null ? "jobject" : g;
        }

        if (j.isArray())
            return "::jvm::array< " + cppType(j.getComponentType()) + " >";

        // very poor support for nested classes!
        if (j.getDeclaringClass() != null)
            return "jobject";

        return "::" + j.getName().replaceAll("\\.", "::");
    }

    public static boolean isWrapped(Class<?> cls) {
        // Can't wrap nested classes yet...
        return !cls.isPrimitive() && cls.getDeclaringClass() == null;
    }

    public static String fixName(String name) {
        return reserved.contains(name) ? name + "_" : name;
    }

    public static void save(String path, String content) throws Exception {
        new File(path).getParentFile().mkdirs();
        PrintWriter writer = new PrintWriter(path);
        writer.print(content);
        writer.close();
    }

    public static String load(String path) throws Exception {
        if (!new File(path).exists())
            return "$$$empty$$$";
        StringBuilder fileData = new StringBuilder();
        BufferedReader reader = new BufferedReader(new FileReader(path));
        char[] buf = new char[1024];
        int numRead=0;
        while ((numRead = reader.read(buf)) != -1)
            fileData.append(buf, 0, numRead);
        reader.close();
        return fileData.toString();
    }

    public static int saveIfDifferent(String path, String content) throws Exception {
        String oldContent = load(path);
        if (!oldContent.equals(content)) {
            System.out.println("Saving new version: " + path);
            save(path, content);
            return 1;
        }
        return 0;
    }

    public static int generate(Class<?> cls, File out) throws Exception
    {
    	int generated = 0;
        if (!isWrapped(cls))
            return generated;

        char sl = File.separatorChar;

        generated += saveIfDifferent(out.getPath() + sl + "include" + sl + cls.getName().replace('.', sl) + ".hpp",
            new HeaderGenerator().toString(cls));

        generated += saveIfDifferent(out.getPath() + sl + cls.getName().replace('.', '_') + ".cpp",
            new ImplementationGenerator().toString(cls));
        
        return generated;
    }

    // Returns a list of types that the given type is assignment-compatible with
    public static Iterable<Class<?>> getSuperTypes(Class<?> cls) {
        List<Class<?>> compatibleTypes = new ArrayList<Class<?>>();

        for (Class<?> i : cls.getInterfaces())
            	compatibleTypes.add(i);

        for (Class<?> superCls = cls.getSuperclass(); 
             superCls != null; superCls = superCls.getSuperclass()) 
        	compatibleTypes.add(superCls);

        return compatibleTypes;
    }

    // Recursively builds the set of types that are referred to in the definition of
    // the given type.
    public static void getRequiredTypes(Class<?> cls, Set<Class<?>> required, int depth) {            
        if (cls.isArray()) {
            getRequiredTypes(cls.getComponentType(), required, depth); // same depth
            return;
        }
        if ((depth < 0) || cls.isPrimitive() || required.contains(cls))
            return;
        required.add(cls);
        for (Class<?> st : getSuperTypes(cls))
            getRequiredTypes(st, required, depth - 1);
        
        for (Constructor<?> ctor : cls.getConstructors()) {
            for (Class<?> p : ctor.getParameterTypes())
                getRequiredTypes(p, required, depth - 1);
        }

        for (Method m : cls.getMethods()) {
            getRequiredTypes(m.getReturnType(), required, depth - 1);
            for (Class<?> p : m.getParameterTypes())
                getRequiredTypes(p, required, depth - 1);
        }
    }

    public static Iterable<Class<?>> getRequiredTypes(Class<?> cls, int depth) {
        HashSet<Class<?>> req = new HashSet<Class<?>>();
        getRequiredTypes(cls, req, depth);
        return sortClasses(req);        
    }

    public static Iterable<Class<?>> getDirectlyRequiredTypes(Class<?> cls) {
        return getRequiredTypes(cls, 1);
    }

    public static void getAllRequiredTypes(Class<?> cls, Set<Class<?>> required) {
        getRequiredTypes(cls, required, Integer.MAX_VALUE);
    }

    public static List<Class<?>> sortClasses(Collection<Class<?>> classes) {
    	List<Class<?>> sorted = new ArrayList<Class<?>>(classes);
        Collections.sort(sorted, new Comparator<Class<?>>() {
			public int compare(Class<?> arg0, Class<?> arg1) {
				return arg0.getName().compareTo(arg1.getName());
			}
        });
        return sorted;
    }
    
    public static void main(String[] args) throws Exception
    {
        if (args.length < 2)
            System.out.print("Please specify output-path and one or more Java class names");
        else
        {
            Set<Class<?>> required = new HashSet<Class<?>>();
            for (int a = 1; a < args.length; a++)
                getAllRequiredTypes(Class.forName(args[a]), required);
            
            int count = 0;
            for (Class<?> cls : sortClasses(required)) 
                count += generate(cls, new File(args[0]));
            
            if (count == 0)
            	System.out.println("All wrapped Java classes were already up to date");
        }
    }
}

