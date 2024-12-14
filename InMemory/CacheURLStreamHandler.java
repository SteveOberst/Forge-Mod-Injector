package net.sxlver;

import java.io.IOException;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLStreamHandler;

/**
 * Special MEM cache stream protocol handler knows how to make a connection
 * for the protocol type x-mem-cache.
 * <p>
 * This handler loads jar, class, java files and jar files contained in
 * directories into the RAM cache.
 */
public class CacheURLStreamHandler extends URLStreamHandler {

    private final CacheClassLoader classLoader;
    private final boolean resources;

    public CacheURLStreamHandler(Object classLoader, final boolean resources) {
        this.classLoader = (CacheClassLoader) classLoader;
        this.resources = resources;
    }

    @Override
    protected URLConnection openConnection(URL url) throws IOException {
        return new CacheURLConnection(classLoader, url, resources);
    }

}
