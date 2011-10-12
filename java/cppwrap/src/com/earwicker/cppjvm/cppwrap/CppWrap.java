
package com.earwicker.cppjvm.cppwrap;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

public class CppWrap {
    private static java.util.HashMap<Class<?>, String> primitives;
    private static java.util.HashSet<String> reserved;

    private static PrintWriter log;
    private static void println(String str) {
        if (log != null)
            log.println(str);
    }

    static {
        reserved = new java.util.HashSet<String>();
        reserved.add("delete");
        reserved.add("union");
        reserved.add("and");
        reserved.add("or");
        reserved.add("xor");
        reserved.add("not");
        reserved.add("NULL");
        reserved.add("register");
        
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

    public static String nestedName(Class<?> cls, boolean namespace) {
        
        String name = cls.getSimpleName();

        while (cls.getDeclaringClass() != null) {
            cls = cls.getDeclaringClass();
            name = cls.getSimpleName() + "_n::" + name;
        }

        if (namespace) {
            String packageName = cls.getPackage().getName();
            name = packageName.replaceAll("\\.", "::") + "::" + name;
        }
        
        return name;
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

        return "::" + nestedName(j, true);
    }

    public static boolean isWrapped(Class<?> cls) {
        return !cls.isPrimitive();
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
            println("Saving new version: " + path);
            save(path, content);
            return 1;
        }
        return 0;
    }

    public static int generate(Class<?> cls, File out, 
        List<String> files, boolean generating) throws Exception
    {
    	int generated = 0;
        if (!isWrapped(cls))
            return generated;

        char sl = File.separatorChar;

        String headerName = out.getPath() + sl + "include" + sl + 
            cls.getName().replace('.', sl).replace('$', sl) + ".hpp";

        if (generating) {
            generated += saveIfDifferent(headerName,
                new HeaderGenerator().toString(cls));
        }
            
        String implName = out.getPath() + sl + 
            cls.getName().replace('.', '_').replace('$', '_') + ".cpp";

        if (generating) {
            generated += saveIfDifferent(implName,
                new ImplementationGenerator().toString(cls));
        }

        files.add(implName);

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
    public static void getRequiredTypes(
            Class<?> cls, Set<Class<?>> required, 
            Map<Class<?>, Integer> minDepths, int currentDepth) {

        if (cls.isArray()) {
            getRequiredTypes(cls.getComponentType(), required, minDepths, currentDepth); // same depth
            return;
        }

        if (minDepths != null) {
            Integer oldDepth = minDepths.get(cls);
            if ((oldDepth == null) || (oldDepth > currentDepth)) {
                minDepths.put(cls, currentDepth);
            }
        }

        println("Considering: " + cls.getName());

        if (cls.isPrimitive() || required.contains(cls))
        {
            if (cls.isPrimitive())
                println("Ignoring primitive: " + cls.getName());

            if (required.contains(cls))
                println("Ignoring already found: " + cls.getName());    

            return;
        }
        
        println("Requires: " + cls.getName());

        required.add(cls);
        println("Supertypes of: " + cls.getName());
        for (Class<?> st : getSuperTypes(cls))        
            getRequiredTypes(st, required, minDepths, currentDepth + 1);

        println("Constructors of: " + cls.getName());
        for (Constructor<?> ctor : cls.getConstructors()) {
            for (Class<?> p : ctor.getParameterTypes())
                getRequiredTypes(p, required, minDepths, currentDepth + 1);
        }

        println("Methods of: " + cls.getName());
        for (Method m : cls.getMethods()) {
            println("Method " + cls.getName() + "." + m.getName());
            getRequiredTypes(m.getReturnType(), required, minDepths, currentDepth + 1);
            for (Class<?> p : m.getParameterTypes())
                getRequiredTypes(p, required, minDepths, currentDepth + 1);
        }

        for (Class<?> c : cls.getClasses()) {
            println("Nested classes of: " + cls.getName());
            getRequiredTypes(c, required, minDepths, currentDepth); // same depth
        }
    }

    public static void getAllRequiredTypes(Class<?> cls, Set<Class<?>> required) {
        getRequiredTypes(cls, required, null, 0);
    }

    public static Iterable<Class<?>> getDirectlyRequiredTypes(Class<?> cls) {
    
        println("Directly required types for: " + cls.getName());
    
        HashSet<Class<?>> req = new HashSet<Class<?>>();
        Map<Class<?>, Integer> minDepths = new HashMap<Class<?>, Integer>();
        getRequiredTypes(cls, req, minDepths, 0);

        List<Class<?>> pruned = new ArrayList<Class<?>>();
        for (Class<?> c : req) {
            int minDepth = minDepths.get(c);
            println("    " + minDepth + ": " + c.getName());
            if (minDepth <= 2) {
                pruned.add(c);
            }
        }

        return sortClasses(pruned);        
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
    
    public static void main(String[] args) throws Exception {
        if (args.length < 2)
            System.err.print("Please specify output-path and one or more Java class names");
        else
        {
            File outDir = new File(args[0]);
    
            boolean generating = true;
    
            Set<Class<?>> required = new HashSet<Class<?>>();
            for (int a = 1; a < args.length; a++) {
                if (args[a].equals("--log")) {
                    if (log == null) {
                        outDir.mkdirs();
                        log = new PrintWriter(new FileWriter(new File(outDir, "CppWrapLog.txt")));
                    }
                } else if (args[a].equals("--list")) {
                    generating = false;     
                } else if (args[a].equals("--generate")) {
                    generating = true;
                } else {
                    getAllRequiredTypes(Class.forName(args[a]), required);
                }
            }

            int count = 0;
            List<String> files = new ArrayList<String>();
            for (Class<?> cls : sortClasses(required))
                count += generate(cls, outDir, files, generating);

            if (generating) {
                if (count == 0)
                    println("All wrapped Java classes were already up to date");
            } else {
                for (String file : files) {
                    System.out.print(file.replace(File.separatorChar, '/') + " ");
                }    
            }

            if (log != null)
                log.close();
        }
    }
}

