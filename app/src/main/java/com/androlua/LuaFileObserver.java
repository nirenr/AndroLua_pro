package com.androlua;

import android.os.FileObserver;

/**
 * Created by Administrator on 2017/11/08 0008.
 */

public class LuaFileObserver extends FileObserver {

    private OnEventListener mOnEventListener;

    public LuaFileObserver(String path) {
        super(path);
    }

    public LuaFileObserver(String path, int mask) {
        super(path, mask);
    }

    public void setOnEventListener(OnEventListener listener){
        mOnEventListener=listener;
    }

    @Override
    public void onEvent(int event, String path) {
        if(mOnEventListener!=null)
            mOnEventListener.onEvent(event, path);
    }

    public interface OnEventListener{
        public void onEvent(int event, String path);
    }
}
