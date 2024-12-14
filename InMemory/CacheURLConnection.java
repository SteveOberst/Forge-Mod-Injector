package net.sxlver;

import java.io.ByteArrayInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.util.Map;

public class CacheURLConnection extends URLConnection {

    private final CacheClassLoader classLoader;
    private final Map<String, byte[]> cache;


    public CacheURLConnection(CacheClassLoader classLoader, URL url, boolean resourceCache) {
        super(url);
        this.classLoader = classLoader;
        this.cache = resourceCache ? classLoader.RESOURCE_CACHE : classLoader.CACHE;
    }

    @Override
    public void connect() throws IOException {
    }

    @Override
    public InputStream getInputStream() throws IOException {
        String file_name = url.getFile();


        byte[] data = cache.get(file_name);

        if (classLoader.loadAllJar != true) {
            if (data == null) {
                classLoader.addCode(file_name);
            }

            data = cache.get(file_name);
        }

        if (data == null) {
            throw new FileNotFoundException(file_name);
        }

        return new ByteArrayInputStream(data);
    }

}
