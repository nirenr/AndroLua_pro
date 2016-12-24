package com.androlua;

import android.app.*;
import android.content.*;
import android.content.res.*;
import android.graphics.*;
import android.util.*;
import android.view.*;
import android.widget.*;
import java.io.*;
import java.math.*;
import java.security.*;
import java.util.*;
import java.util.zip.*;

public class LuaUtil {
	/**
	 * 截屏
	 * @param activity
	 * @return
	 */
	public static Bitmap captureScreen(Activity activity) {
// 获取屏幕大小：
		DisplayMetrics metrics = new DisplayMetrics();
		WindowManager WM = (WindowManager) activity
			.getSystemService(Context.WINDOW_SERVICE);
		Display display = WM.getDefaultDisplay();
		display.getMetrics(metrics);
		int height = metrics.heightPixels; // 屏幕高
		int width = metrics.widthPixels; // 屏幕的宽
// 获取显示方式
		int pixelformat = display.getPixelFormat();
		PixelFormat localPixelFormat1 = new PixelFormat();
		PixelFormat.getPixelFormatInfo(pixelformat, localPixelFormat1);
		int deepth = localPixelFormat1.bytesPerPixel;// 位深
		byte[] piex = new byte[height * width * deepth];
		try {
			Runtime.getRuntime().exec(
				new String[] { "/system/bin/su", "-c",
					"chmod 777 /dev/graphics/fb0" });
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		try {
// 获取fb0数据输入流
			InputStream stream = new FileInputStream(new File(
														 "/dev/graphics/fb0"));
			DataInputStream dStream = new DataInputStream(stream);
			dStream.readFully(piex);
		}
		catch (Exception e) {
			e.printStackTrace();
		}
// 保存图片
		int[] colors = new int[height * width];
		for (int m = 0; m < colors.length; m++) {
			int r = (piex[m * 4] & 0xFF);
			int g = (piex[m * 4 + 1] & 0xFF);
			int b = (piex[m * 4 + 2] & 0xFF);
			int a = (piex[m * 4 + 3] & 0xFF);
			colors[m] = (a << 24) + (r << 16) + (g << 8) + b;
		}
// piex生成Bitmap
		Bitmap bitmap = Bitmap.createBitmap(colors, width, height,
											Bitmap.Config.ARGB_8888);
		return bitmap;
	}

	//读取asset文件

	public static byte[] readAsset(Context context, String name) throws IOException {
		AssetManager am = context.getAssets();
		InputStream is = am.open(name);
		byte[] ret= readAll(is);
		is.close();
		//am.close();
		return ret;
	}

	public static byte[] readAll(InputStream input) throws IOException {
		ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
		byte[] buffer = new byte[2 ^ 32];
		int n = 0;
		while (-1 != (n = input.read(buffer))) {
			output.write(buffer, 0, n);
		}
		byte[] ret= output.toByteArray();
		output.close();
		return ret;
	}

//复制asset文件到sd卡
	public static void assetsToSD(Context context, String InFileName, String OutFileName) throws IOException {  
		InputStream myInput;  
		OutputStream myOutput = new FileOutputStream(OutFileName);  
		myInput = context.getAssets().open(InFileName);  
		byte[] buffer = new byte[8192];  
		int length = myInput.read(buffer);
        while (length > 0) {
			myOutput.write(buffer, 0, length); 
			length = myInput.read(buffer);
		}

        myOutput.flush();  
		myInput.close();  
		myOutput.close();        
	}  

	public static void copyFile(String from, String to) { 
		try {
			copyFile(new FileInputStream(from), new FileOutputStream(to));
		}
		catch (IOException e) {
			Log.d("lua",e.getMessage()); 
		}
	} 

	public static boolean copyFile(InputStream in, OutputStream out) { 
		try { 
			int byteread = 0; 
			byte[] buffer = new byte[1024 * 1024]; 
			while ((byteread = in.read(buffer)) != -1) { 
				out.write(buffer, 0, byteread); 
			} 
			in.close(); 
			out.close();
		} 
		catch (Exception e) { 
			Log.d("lua",e.getMessage());
			return false;
		} 
		return true;
	} 
	
	public static boolean copyDir(String from, String to) {
		return copyDir(new File(from), new File(to));
	}

	public static boolean copyDir(File from, File to) {
		boolean ret=true;
		File p=to.getParentFile();
		if (!p.exists())
			p.mkdirs();
		if (from.isDirectory()) {
			File[] fs=from.listFiles();
			if (fs != null && fs.length != 0) {
				for (File f:fs)
					ret=copyDir(f, new File(to, f.getName()));
			}
			else {
				if (!to.exists())
					ret=to.mkdirs();
			}
		}
		else {
			try {
				if (!to.exists())
					to.createNewFile();
				ret=copyFile(new FileInputStream(from), new FileOutputStream(to));
			}
			catch (IOException e) {
				Log.d("lua",e.getMessage());
				ret=false;
			}
		}
		return ret;
	}

	public static boolean rmDir(File dir) {
		if(dir.isDirectory()){
			File[] fs=dir.listFiles();
			for (File f:fs)
				rmDir(f);
		}
		return dir.delete();
	}

	public static void rmDir(File dir, String ext) {
		if(dir.isDirectory()){
			File[] fs=dir.listFiles();
			for (File f:fs)
				rmDir(f,ext);
			dir.delete();
		}
		if (dir.getName().endsWith(ext))
			dir.delete();
	}

	public static byte[] readZip(String zippath, String filepath) throws IOException {
		ZipFile zip=new ZipFile(zippath);
		ZipEntry entey=zip.getEntry(filepath);
		InputStream is=zip.getInputStream(entey);
		return readAll(is);
	}

// 计算文件的 MD5 值

	public static String getFileMD5(File file) {
		try {
			return getFileMD5(new FileInputStream(file));
		}
		catch (FileNotFoundException e) {
			return null;
		}
	}
	public static String getFileMD5(InputStream in) {
		byte buffer[] = new byte[8192];
		int len;
		try {
			MessageDigest digest =MessageDigest.getInstance("MD5");
			while ((len = in.read(buffer)) != -1) {
				digest.update(buffer, 0, len);
			}
			BigInteger bigInt = new BigInteger(1, digest.digest());
			return bigInt.toString(16);
		}
		catch (Exception e) {
			e.printStackTrace();
			return null;
		}
		finally {
			try {
				in.close();
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}

	}

// 计算文件的 SHA-1 值
	public static String getFileSha1(InputStream in) {
		byte buffer[] = new byte[8192];
		int len;
		try {
			MessageDigest digest =MessageDigest.getInstance("SHA-1");
			while ((len = in.read(buffer)) != -1) {
				digest.update(buffer, 0, len);
			}
			BigInteger bigInt = new BigInteger(1, digest.digest());
			return bigInt.toString(16);
		}
		catch (Exception e) {
			e.printStackTrace();
			return null;
		}
		finally {
			try {
				in.close();
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	public static void unZip(String SourceDir) throws IOException {
		unZip(SourceDir,new File(SourceDir).getParent(), "");
	}

	public static void unZip(String SourceDir, String extDir) throws IOException {
		unZip(SourceDir,extDir, "");
	}

	public static void unZip(String SourceDir, String extDir,String fileExt) throws IOException {
		ZipFile zip=new ZipFile(SourceDir);
		Enumeration<? extends ZipEntry> entries=zip.entries();
		while (entries.hasMoreElements()) {
			ZipEntry entry=entries.nextElement();
			String name=entry.getName();
			if(!name.startsWith(fileExt))
				continue;
			String path=name;
			if (entry.isDirectory()) {
				File f=new File(extDir + File.separator + path);
				if (!f.exists())
					f.mkdirs();
			}
			else {
				String fname=extDir + File.separator + path;
				File temp=new File(fname).getParentFile();
				if (!temp.exists()) {
					if (!temp.mkdirs()) {
						throw new RuntimeException("create file " + temp.getName() + " fail");
					}
				}

				FileOutputStream out=new FileOutputStream(extDir + File.separator + path);
				InputStream in=zip.getInputStream(entry);
				byte[] buf=new byte[2 ^ 16];
				int count=0;
				while ((count = in.read(buf)) != -1) {
					out.write(buf, 0, count);
				}
				out.close();
				in.close();
			}
		}
		zip.close();
	}


	private static final byte[] BUFFER = new byte[1024 * 1024]; 

	public static boolean zip(String sourceFilePath) {
		return zip(sourceFilePath,new File(sourceFilePath).getParent());
	}
	
	public static boolean zip(String sourceFilePath, String zipFilePath) {
		File f=new File(sourceFilePath);
		return zip(sourceFilePath, f.getParent(),f.getName()+".zip");
	}
	
	public static boolean zip(String sourceFilePath, String zipFilePath,String zipFileName) {
		boolean result=false;
		File source=new File(sourceFilePath);
		File zipFile=new File(zipFilePath , zipFileName);
		if (!zipFile.getParentFile().exists()) {
			if (!zipFile.getParentFile().mkdirs()) {
				return result;
			}
		}
		if (zipFile.exists()) {
			try {
				zipFile.createNewFile();
			}
			catch (IOException e) {
				return result;
			}
		}

		FileOutputStream dest=null;
		ZipOutputStream out =null;
		try {
			dest = new FileOutputStream(zipFile);
			CheckedOutputStream checksum = new CheckedOutputStream(dest, new Adler32());
			out = new ZipOutputStream(new BufferedOutputStream(checksum));
			out.setMethod(ZipOutputStream.DEFLATED);
			compress(source, out, "/");
			result = true;
		}
		catch (FileNotFoundException e) {
			e.printStackTrace();
		}
		finally {
			if (out != null) {
				try {
					out.closeEntry();
				}
				catch (IOException e) {
					e.printStackTrace();
				}
				try {
					out.close();
				}
				catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
		return result;
	}

	private static void compress(File file, ZipOutputStream out, String mainFileName) {
		if (file.isFile()) {
			FileInputStream fi= null;
			BufferedInputStream origin=null;
			try {
				fi = new FileInputStream(file);
				origin = new BufferedInputStream(fi, BUFFER.length);
				//int index=file.getAbsolutePath().indexOf(mainFileName);
				String entryName=mainFileName + file.getName();
				System.out.println(entryName);
				ZipEntry entry = new ZipEntry(entryName);
				out.putNextEntry(entry);
				//			byte[] data = new byte[BUFFER];
				int count;
				while ((count = origin.read(BUFFER, 0, BUFFER.length)) != -1) {
					out.write(BUFFER, 0, count);
				}
			}
			catch (FileNotFoundException e) {
				e.printStackTrace();
			}
			catch (IOException e) {
				e.printStackTrace();
			}
			finally {
				if (origin != null) {
					try {
						origin.close();
					}
					catch (IOException e) {
						e.printStackTrace();
					}
				}
			}
		}
		else if (file.isDirectory()) {
			File[] fs=file.listFiles();
			if (fs != null && fs.length > 0) {
				for (File f:fs) {
					if (f.isFile())
						compress(f, out, mainFileName);
					else
						compress(f, out, mainFileName + f.getName() + "/");

				}
			}
		}
	}


}
