package com.luajava;

/**
 * Created by nirenr on 2018/12/17.
 */

public class LuaString implements CharSequence {

    private  byte[] mByte=new byte[0];

    public LuaString(String string) {
        mByte=string.getBytes();
    }

    public LuaString(byte[] string) {
        mByte=string;
    }

    public byte[] toByteArray() {
        return mByte;
    }

    @Override
    public int length() {
        return mByte.length;
    }

    @Override
    public char charAt(int index) {
        return (char) mByte[index];
    }

    @Override
    public CharSequence subSequence(int start, int end) {
        return new String(mByte,start,end);
    }

    @Override
    public String toString() {
        return new String(mByte);
    }
}
