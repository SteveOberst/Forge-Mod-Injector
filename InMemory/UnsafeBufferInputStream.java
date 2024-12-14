package net.sxlver;

import sun.misc.Unsafe;

import java.io.IOException;
import java.io.InputStream;

public class UnsafeBufferInputStream extends InputStream {
    private Unsafe unsafe;
    private long address;
    private long remainingBytes;
    private static final int BUFFER_SIZE = 1024; // Adjust buffer size as needed

    public UnsafeBufferInputStream(long startAddr, long bufLen) {
        this.unsafe = getUnsafe();
        this.address = startAddr;
        this.remainingBytes = bufLen;
    }

    private Unsafe getUnsafe() {
        try {
            java.lang.reflect.Field field = Unsafe.class.getDeclaredField("theUnsafe");
            field.setAccessible(true);
            return (Unsafe) field.get(null);
        } catch (Exception e) {
            throw new RuntimeException("Unsafe access error: " + e.getMessage());
        }
    }

    @Override
    public int read() throws IOException {
        if (remainingBytes <= 0) {
            return -1;
        }
        byte value = unsafe.getByte(address++);
        remainingBytes--;
        return value & 0xFF;
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        if (remainingBytes <= 0) {
            return -1;
        }
        int bytesRead = (int) Math.min(len, remainingBytes);
        for (int i = 0; i < bytesRead; i++) {
            b[off + i] = unsafe.getByte(address++);
        }
        remainingBytes -= bytesRead;
        return bytesRead;
    }

    @Override
    public void close() {
    }
}
