package com.androlua;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;

/**
 * Create By Qiujuer
 * 2014-07-26
 * <p/>
 * 执行命令行语句静态方法封装
 */
public class ProcessModel {
    //换行符
    private static final String BREAK_LINE;
    //执行退出命令
    private static final byte[] COMMAND_EXIT;
    //错误缓冲
    private static byte[] BUFFER;

    /**
     * 静态变量初始化
     */
    static {
        BREAK_LINE = "\n";
        COMMAND_EXIT = "\nexit\n".getBytes();
        BUFFER = new byte[32];
    }


    /**
     * 执行命令
     *
     * @param params 命令参数
     *               <pre> eg: "/system/bin/ping", "-c", "4", "-s", "100","www.qiujuer.net"</pre>
     * @return 执行结果
     */
    public static String execute(String... params) {
        Process process = null;
        StringBuilder sbReader = null;

        BufferedReader bReader = null;
        InputStreamReader isReader = null;

        InputStream in = null;
        InputStream err = null;
        OutputStream out = null;

        try {
            process = new ProcessBuilder()
				.command(params)
				.start();
            out = process.getOutputStream();
            in = process.getInputStream();
            err = process.getErrorStream();

            out.write(COMMAND_EXIT);
            out.flush();

            process.waitFor();

            isReader = new InputStreamReader(in);
            bReader = new BufferedReader(isReader);

            String s;
            if ((s = bReader.readLine()) != null) {
                sbReader = new StringBuilder();
                sbReader.append(s);
                sbReader.append(BREAK_LINE);
                while ((s = bReader.readLine()) != null) {
                    sbReader.append(s);
                    sbReader.append(BREAK_LINE);
                }
            }

            while ((err.read(BUFFER)) > 0) {
            }
        } catch (IOException e) {
            e.printStackTrace();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            closeAllStream(out, err, in, isReader, bReader);

            if (process != null) {
                processDestroy(process);
                process = null;
            }
        }

        if (sbReader == null)
            return null;
        else
            return sbReader.toString();
    }

    /**
     * 关闭所有流
     *
     * @param out      输出流
     * @param err      错误流
     * @param in       输入流
     * @param isReader 输入流封装
     * @param bReader  输入流封装
     */
    private static void closeAllStream(OutputStream out, InputStream err, InputStream in, InputStreamReader isReader, BufferedReader bReader) {
        if (out != null)
            try {
                out.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        if (err != null)
            try {
                err.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        if (in != null)
            try {
                in.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        if (isReader != null)
            try {
                isReader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        if (bReader != null)
            try {
                bReader.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
    }


    /**
     * 通过Android底层实现进程关闭
     *
     * @param process 进程
     */
    private static void killProcess(Process process) {
        int pid = getProcessId(process);
        if (pid != 0) {
            try {
                //android kill process
                android.os.Process.killProcess(pid);
            } catch (Exception e) {
                try {
                    process.destroy();
                } catch (Exception ex) {
                }
            }
        }
    }

    /**
     * 获取进程的ID
     *
     * @param process 进程
     * @return
     */
    private static int getProcessId(Process process) {
        String str = process.toString();
        try {
            int i = str.indexOf("=") + 1;
            int j = str.indexOf("]");
            str = str.substring(i, j);
            return Integer.parseInt(str);
        } catch (Exception e) {
            return 0;
        }
    }

    /**
     * 销毁进程
     *
     * @param process 进程
     */
    private static void processDestroy(Process process) {
        if (process != null) {
            try {
                //判断是否正常退出
                if (process.exitValue() != 0) {
                    killProcess(process);
                }
            } catch (IllegalThreadStateException e) {
                killProcess(process);
            }
        }
    }
}

