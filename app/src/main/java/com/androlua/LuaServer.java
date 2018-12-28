package com.androlua;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;


/**
 * Created by Administrator on 2017/10/20 0020.
 */

public class LuaServer implements LuaGcable {
    private ServerSocket mServerSocket;
    private OnReadLineListener mOnReadLineListener;
    private boolean mGc;

    public LuaServer(LuaContext context) {
        context.regGc(this);
    }

    public LuaServer() {
    }

    public boolean start(int port) {
        if(mServerSocket!=null)
            return false;
        try {
            mServerSocket = new ServerSocket(port);
            new ServerThread(mServerSocket).start();
            return true;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    public boolean stop(){
        try {
            mServerSocket.close();
            mServerSocket=null;
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
        if(mServerSocket==null)
            return;
        try {
            mServerSocket.close();
        } catch (Exception e) {
            e.printStackTrace();
        }
        mGc=true;
    }

    @Override
    public boolean isGc() {
        return mGc;
    }

    private class ServerThread extends Thread {
        private final ServerSocket mServer;

        public ServerThread(ServerSocket serverSocket){
            mServer=serverSocket;
        }

        @Override
        public void run() {
            while (true) {
                try {
                    Socket socket = mServerSocket.accept();
                    new SocketThread(socket).start();
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }
    }

    private class SocketThread extends Thread {
        private final Socket mSocket;
        private BufferedWriter out;

        public SocketThread(Socket socket) {
            mSocket = socket;
        }

        @Override
        public void run() {
            try {
                BufferedReader in = new BufferedReader(new InputStreamReader(mSocket.getInputStream()));
                out = new BufferedWriter(new OutputStreamWriter(mSocket.getOutputStream()));
                String line;
                while ((line = in.readLine()) != null) {
                    if (mOnReadLineListener != null)
                        mOnReadLineListener.onReadLine(LuaServer.this, this, line);
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
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
        public void onReadLine(LuaServer server, SocketThread socket, String line);
    }

}
