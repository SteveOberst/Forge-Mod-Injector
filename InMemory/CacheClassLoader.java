/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License") +  you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package net.sxlver;

import java.io.*;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;

/**
 * The CacheClassLoader class implements a class loader that loads classes from
 * jar file, class file java file, directories with jar files and stores these
 * in memory cache.
 * <p>
 * The code of individual classes is stored in the RAM cache and it is possible
 * to exchange source files without having to restart the entire application.
 * Simply change the appropriate file. Creates a new classloader that loads
 * these new resources and uses them later. The old one can either be forgotten
 * or used with the original code. Supported code sources:
 * <p>
 * - jar file
 * <p>
 * - directory (The directory is crawled recursively)
 * <p>
 * - .class file (byte code)
 * <p>
 * - .java file (source code)
 * <p>
 * Examples of usage:
 * <pre>
 * CacheClassLoader childClassLoader = new CacheClassLoader(Thread.currentThread().getContextClassLoader());
 *
 * String file_name = "/tmp/Test3.jar";
 * System.out.println("addJAR = " + file_name);
 * childClassLoader.addJAR(file_name);
 *
 * file_name = "/tmp/Test.jar";
 * childClassLoader.addJAR(file_name);
 * System.out.println("addJAR = " + file_name);
 *
 * final Class&#60;?&#62; test = Class.forName("cz.b2b.jcl.RAM.resource.jar.Test3", true, childClassLoader);
 * Object o = test.getDeclaredConstructor(new Class[]{}).newInstance(new Object[]{});
 *
 * Method print = o.getClass().getMethod("print", String.class);
 * System.out.println("class = " + o.getClass().getCanonicalName());
 * print.invoke(o, "JAR");
 *
 * </pre>
 * <pre>
 * String path = "/tmp/class";
 * String packageName = "cz.b2b.jcl.RAM.resource";
 * String className = "Test";
 * String fullClassName = packageName + "." + className;
 * System.out.println("addClass, path = " + path + ", package =  " + packageName + ", class = " + className);
 * CacheClassLoader childClassLoader = new CacheClassLoader(Thread.currentThread().getContextClassLoader());
 * childClassLoader.addClass(path, packageName, className);
 * final Class&#60;?&#62; test = Class.forName(fullClassName, true, childClassLoader);
 * Object o = test.getDeclaredConstructor(new Class[]{}).newInstance(new Object[]{});
 *
 * Method print = o.getClass().getMethod("print", String.class);
 * System.out.println("class = " + o.getClass().getCanonicalName());
 * print.invoke(o, "CLASS");
 * </pre>
 *
 * @author Richard Kotal &#60;richard.kotal@b2b.cz&#620;
 */
public class CacheClassLoader extends URLClassLoader {

    public static final int BUFFER_SIZE = 8192;
    public final static String host = null;
    public final static int port = -1;
    public final static String baseURI = "/";

    private final static String protocol = "x-mem-cache";

    public final Map<String, byte[]> CACHE;
    public final Map<String, byte[]> RESOURCE_CACHE;
    private URL cacheURL = null;
    private final List<String> jars = new CopyOnWriteArrayList<>();

    public boolean loadAllJar;
    private final ClassLoader launchClassLoader;

    /**
     * Constructs a new CacheClassLoader for the given URLs of URLClassLoader
     * and the MEM cache stream protocol handler.
     * <p>
     * The url for the MEM cache stream protocol handler is added to the others
     * when the constructor is created.
     *
     * @param parent the parent class loader for delegation
     * internally. If equal -1 all references are still held internally.
     * @param loadAllJar Allows loading of the entire spring content. Otherwise,
     * only the required class is loaded. Reduces memory requirements, can
     * significantly reduce loading speed.
     * @throws MalformedURLException Thrown to indicate that a malformed URL has
     * occurred. Either no legal protocol could be found in a specification
     * string or the string could not be parsed.
     */
    public CacheClassLoader(ClassLoader parent, boolean loadAllJar) throws MalformedURLException {
        super(new URL[]{}, parent);
        this.launchClassLoader = parent;

        CACHE = new ConcurrentHashMap<>();
        RESOURCE_CACHE = new ConcurrentHashMap<>();
        this.loadAllJar = loadAllJar;
        cacheURL = new URL(protocol, host, port, baseURI, new CacheURLStreamHandler(this, false));

        super.addURL(cacheURL);
    }


    /**
     * Constructs a new CacheClassLoader for the MEM cache stream protocol
     * handler.
     * <p>
     * The url for the MEM cache stream protocol handler is added when the
     * constructor is created.
     *
     * @param parent the parent class loader for delegation
     * @throws MalformedURLException Thrown to indicate that a malformed URL has
     * occurred. Either no legal protocol could be found in a specification
     * string or the string could not be parsed.
     */
    public CacheClassLoader(ClassLoader parent) throws MalformedURLException {
        this(parent, true);
    }

    @Override
    public void close() throws IOException {

        CACHE.clear();
        jars.clear();
        super.close();

    }

    public void addJar(String jarFileName, UnsafeBufferInputStream stream) throws IOException {
        if (loadAllJar == true) {
            addCode(jarFileName, stream);
        } else {
            jars.add(jarFileName);
        }
    }

    public boolean addCode(String fileName, String jar) throws IOException {
        try(final FileInputStream inputStream = new FileInputStream(jar)) {
            return addCode(fileName, inputStream);
        }
    }

    public boolean addCode(String fileName, InputStream inputStream) throws IOException {
        BufferedInputStream bis = null;
        JarInputStream jis = null;
        ByteArrayOutputStream out;
        String name;
        byte[] b = new byte[BUFFER_SIZE];
        int len = 0;

        try {
            bis = new BufferedInputStream(inputStream);
            jis = new JarInputStream(bis);

            JarEntry jarEntry;
            while ((jarEntry = jis.getNextJarEntry()) != null) {
                name = baseURI + jarEntry.getName();

                if (jarEntry.isDirectory()) {
                    continue;
                }

                if (CACHE.containsKey(name)) {
                    continue;
                }

                
                
                if (loadAllJar != true && fileName.equals(name) == false) {
                    continue;
                }

                out = new ByteArrayOutputStream();

                while ((len = jis.read(b)) > 0) {
                    out.write(b, 0, len);
                }

                CACHE.put(name, out.toByteArray());

                if(!name.endsWith(".class")) {
                    RESOURCE_CACHE.put(name, out.toByteArray());
                }

                out.close();
                if (loadAllJar != true) {
                    return true;
                }

            }
        } finally {
            if (jis != null) {
                jis.close();
            }
            if (bis != null) {
                bis.close();
            }
        }
        return false;
    }

    public void addCode(String fileName) throws IOException {
        for (String jar : jars) {
            if (jar == null) {
                continue;
            }
            if (addCode(jar, fileName) == true) {
                return;
            }
        }
    }

    @Override
    public InputStream getResourceAsStream(String name) {
        final byte[] data = CACHE.get(name);
        if(data == null) return null;
        return new BufferedInputStream(new ByteArrayInputStream(data));
    }
}
