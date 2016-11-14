package com.androlua.util;
import android.util.*;
import java.io.*;

	/**
	 * root权限工具类
	 * 
	 * 
	 */
	public class RootUtil {

		private static final String TAG = "linin.root";
		private static boolean mHaveRoot = false;

		/**
		 * 判断机器Android是否已经root，即是否获取root权限
		 */
		public static boolean haveRoot() {
			if (!mHaveRoot) {
				int ret = execRootCmdSilent("echo test"); // 通过执行测试命令来检测
				if (ret != -1) {
					Log.i(TAG, "have root!");
					mHaveRoot = true;
				} else {
					Log.i(TAG, "not root!");
				}
			} else {
				Log.i(TAG, "mHaveRoot = true, have root!");
			}
			return mHaveRoot;
		}

		/** 获取root权限 */
		public static boolean root() {
			try {
				Runtime.getRuntime().exec(
					new String[] { "/system/bin/su", "-c",
						"chmod 777 /dev/graphics/fb0" });
			} catch (IOException e) {
				e.printStackTrace();
				Log.i(TAG, "root fail!");
				return false;
			}
			Log.i(TAG, "root success!");
			return true;
		}

		/**
		 * 执行命令并且输出结果
		 */
		public static String execRootCmd(String cmd) {
			String result = "";
			DataOutputStream dos = null;
			DataInputStream dis = null;

			try {
				Process p = Runtime.getRuntime().exec("su");// 经过Root处理的android系统即有su命令
				dos = new DataOutputStream(p.getOutputStream());
				dis = new DataInputStream(p.getInputStream());

				Log.i(TAG, cmd);
				dos.writeBytes(cmd + "\n");
				dos.flush();
				dos.writeBytes("exit\n");
				dos.flush();
				String line = null;
				while ((line = dis.readLine()) != null) {
					result += line+"\n";
				}
				p.waitFor();
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				if (dos != null) {
					try {
						dos.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
				if (dis != null) {
					try {
						dis.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
			return result;
		}

		/**
		 * 执行命令但不关注结果输出
		 */
		public static int execRootCmdSilent(String cmd) {
			int result = -1;
			DataOutputStream dos = null;

			try {
				Process p = Runtime.getRuntime().exec("su");
				dos = new DataOutputStream(p.getOutputStream());

				Log.i(TAG, cmd);
				dos.writeBytes(cmd + "\n");
				dos.flush();
				dos.writeBytes("exit\n");
				dos.flush();
				p.waitFor();
				result = p.exitValue();
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				if (dos != null) {
					try {
						dos.close();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
			return result;
		}

	
}
