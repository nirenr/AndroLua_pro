package com.androlua;

import android.util.Log;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.Socket;

/**
 * Created by Administrator on 2017/10/20 0020.
 */

public class LuaClient implements LuaGcable {
    private OnReadLineListener mOnReadLineListener;
    private Socket mSocket;
    private BufferedReader in;
    private BufferedWriter out;
    private boolean mGc;

    public LuaClient(LuaContext context) {
        context.regGc(this);
    }

    public LuaClient() {
    }

    public boolean start(String dstName, int dstPort) {
        if(mSocket!=null)
            return false;

        try {
            mSocket = new Socket(dstName, dstPort);
            in = new BufferedReader(new InputStreamReader(mSocket.getInputStream()));
            out = new BufferedWriter(new OutputStreamWriter(mSocket.getOutputStream()));
            new SocketThread(mSocket).start();
            return true;
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean stop() {
        if(mSocket==null)
            return false;
        try {
            mSocket.close();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public void setOnReadLineListener(OnReadLineListener listener) {
        mOnReadLineListener = listener;
    }

    @Override
    public void gc() {
        stop();
        mGc=true;
    }

    @Override
    public boolean isGc() {
        return mGc;
    }

    public boolean write(String text) {
        try {
            out.write(text);
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean flush() {
        try {
            out.flush();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean newLine() {
        try {
            out.newLine();
            out.flush();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private class SocketThread extends Thread {
        private final Socket mSocket;

        public SocketThread(Socket socket) {
            mSocket = socket;
        }

        @Override
        public void run() {
            try {
                String line;
                while ((line = in.readLine()) != null) {
                    if (mOnReadLineListener != null)
                        mOnReadLineListener.onReadLine(LuaClient.this, this, line);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        public boolean write(String text) {
            try {
                Log.i("lua",text);
                out.write(text);
                return true;
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }
        public boolean flush() {
            try {
                out.flush();
                return true;
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        public boolean newLine() {
            try {
                out.newLine();
                out.flush();
                return true;
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }

        public boolean close() {
            try {
                mSocket.close();
                return true;
            } catch (Exception e) {
                e.printStackTrace();
                return false;
            }
        }
    }

    public interface OnReadLineListener {
        public void onReadLine(LuaClient server, SocketThread socket, String line);
    }

}
