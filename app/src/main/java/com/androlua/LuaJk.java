package com.androlua;
import java.io.File;  
import java.util.ArrayList;  
import java.util.List;  
import java.util.Stack;  
  
import android.os.FileObserver;  
import android.util.Log;  
  
@SuppressWarnings(value = { "rawtypes", "unchecked" }) 

public class LuaJk extends FileObserver {  
  
    /*FileObserver的字段集  如：FileObserver.OPEN        */  
    public static int CHANGES_ONLY = OPEN|ATTRIB|MODIFY|ACCESS |CREATE | DELETE | CLOSE_NOWRITE | CLOSE_WRITE| DELETE_SELF | MOVE_SELF | MOVED_FROM | MOVED_TO;  
  
    List mObservers;  
    String mPath;  
    int mMask;  
  
    
    //  创建目录监听器=LuaJK("/sdcard/")
    public LuaJk(String path) {  
        this(path, ALL_EVENTS);  
    }  
  
    public LuaJk(String path, int mask) {  
        super(path, mask);  
        mPath = path;  
        mMask = mask;  
    }  
  
    
    
    // 创建目录监听器.startWatching()
    
    @Override  
    public void startWatching() {  //开始监听
        if (mObservers != null)  
            return;  
  
        mObservers = new ArrayList();  
        Stack stack = new Stack();  
        stack.push(mPath);  
  
        while (!stack.isEmpty()) {  
            String parent = (String)stack.pop();  
            mObservers.add(new 一缕清风自在飞(parent, mMask));  
            File path = new File(parent);  
            File[] files = path.listFiles();  
            if (null == files)  
                continue;  
            for (File f : files) {  
                if (f.isDirectory() && !f.getName().equals(".")  
                        && !f.getName().equals("..")) {  
                    stack.push(f.getPath());  
                }  
            }  
        }  
  
        for (int i = 0; i < mObservers.size(); i++) {  
        	一缕清风自在飞 sfo = (一缕清风自在飞) mObservers.get(i);  
            sfo.startWatching();  
        }  
    };  
  
    @Override  
    public void stopWatching() {  //停止监听
        if (mObservers == null)  
            return;  
  
        for (int i = 0; i < mObservers.size(); i++) {  
        	一缕清风自在飞 sfo = (一缕清风自在飞) mObservers.get(i);  
            sfo.stopWatching();  
        }  
          
        mObservers.clear();  
        mObservers = null;  
    };  
    
    public String 数据(String 返回文本){ //返回给androlua
		return 返回文本;
	}
 
    @Override  
    public void onEvent(int event, String path) {  // onEvent回调函数
        switch (event) {  
        case FileObserver.ACCESS:  
        	数据(path); 
            break;  
        case FileObserver.ATTRIB:  
        	数据(path);  
            break;  
        case FileObserver.CLOSE_NOWRITE:  
        	数据(path);  
            break;  
        case FileObserver.CLOSE_WRITE:  
        	数据(path); 
            break;  
        case FileObserver.CREATE:  
        	数据(path);  
            break;  
        case FileObserver.DELETE:  
        	数据(path);  
            break;  
        case FileObserver.DELETE_SELF:  
        	数据(path);  
            break;  
        case FileObserver.MODIFY:  
        	数据(path);  
            break;  
        case FileObserver.MOVE_SELF:  
        	数据(path);  
            break;  
        case FileObserver.MOVED_FROM:  
        	数据(path);  
            break;  
        case FileObserver.MOVED_TO:  
        	数据(path);  
            break;  
        case FileObserver.OPEN:  
        	数据(path);  
            break;  
        default:  
        	数据(path);  
            break;  
        }  
    }  
  
 
    class 一缕清风自在飞 extends FileObserver {  
        String mPath;  
  
        public 一缕清风自在飞(String path) {  
            this(path, ALL_EVENTS);  
            mPath = path;  
        }  
  
        public 一缕清风自在飞(String path, int mask) {  
            super(path, mask);  
            mPath = path;  
        }  
  
        @Override  
        public void onEvent(int event, String path) {  
            String newPath = mPath + "/" + path;  
            LuaJk.this.onEvent(event, newPath);  
        }  
    }  
}  