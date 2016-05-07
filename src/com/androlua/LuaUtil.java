package com.androlua;

import android.app.*;
import android.content.*;
import android.content.res.*;
import android.graphics.*;
import android.util.*;
import android.view.*;
import java.io.*;

public class LuaUtil
{
	/**
	 * 截屏
	 * @param activity
	 * @return
	 */
	public static Bitmap captureScreen(Activity activity)
	{
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
		try
		{
			Runtime.getRuntime().exec(
				new String[] { "/system/bin/su", "-c",
					"chmod 777 /dev/graphics/fb0" });
		}
		catch (IOException e)
		{
			e.printStackTrace();
		}
		try
		{
// 获取fb0数据输入流
			InputStream stream = new FileInputStream(new File(
														 "/dev/graphics/fb0"));
			DataInputStream dStream = new DataInputStream(stream);
			dStream.readFully(piex);
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
// 保存图片
		int[] colors = new int[height * width];
		for (int m = 0; m < colors.length; m++)
		{
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

	public static byte[] readAsset(Context context, String name) throws IOException 
	{
		AssetManager am = context.getAssets();
		InputStream is = am.open(name);
		byte[] ret= readAll(is);
		is.close();
		//am.close();
		return ret;
	}

	private static byte[] readAll(InputStream input) throws IOException 
	{
		ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
		byte[] buffer = new byte[4096];
		int n = 0;
		while (-1 != (n = input.read(buffer)))
		{
			output.write(buffer, 0, n);
		}
		byte[] ret= output.toByteArray();
		output.close();
		return ret;
	}

//复制asset文件到sd卡
	public static void assetsToSD(Context context, String InFileName, String OutFileName) throws IOException 
	{  
		InputStream myInput;  
		OutputStream myOutput = new FileOutputStream(OutFileName);  
		myInput = context.getAssets().open(InFileName);  
		byte[] buffer = new byte[8192];  
		int length = myInput.read(buffer);
        while (length > 0)
        {
			myOutput.write(buffer, 0, length); 
			length = myInput.read(buffer);
		}

        myOutput.flush();  
		myInput.close();  
		myOutput.close();        
	}  

	public static void copyFile(String oldPath, String newPath)
	{ 
		try
		{ 
			int bytesum = 0; 
			int byteread = 0; 
			File oldfile = new File(oldPath); 
			if (oldfile.exists())
			{ //文件存在时 
				InputStream inStream = new FileInputStream(oldPath); //读入原文件 
				FileOutputStream fs = new FileOutputStream(newPath); 
				byte[] buffer = new byte[2 ^ 32]; 
				int length; 
				while ((byteread = inStream.read(buffer)) != -1)
				{ 
					bytesum += byteread; //字节数 文件大小 
					System.out.println(bytesum); 
					fs.write(buffer, 0, byteread); 
				} 
				inStream.close(); 
			} 
		} 
		catch (Exception e)
		{ 
			System.out.println("复制文件操作出错"); 
			e.printStackTrace(); 

		} 

	} 

	public static void copyFile(InputStream inStream, OutputStream fs)
	{ 
		try
		{ 
			int bytesum = 0; 
			int byteread = 0; 
			byte[] buffer = new byte[4096]; 
			while ((byteread = inStream.read(buffer)) != -1)
			{ 
				fs.write(buffer, 0, byteread); 
			} 
			inStream.close(); 
		} 
		catch (Exception e)
		{ 
			System.out.println("复制文件操作出错"); 
			e.printStackTrace(); 

		} 

	} 

	public static boolean rmDir(File dir)
	{
		File[] fs=dir.listFiles();
		if (fs != null)
			for (File f:fs)
				if (f.isDirectory())
					rmDir(f);
				else
					f.delete();

		return dir.delete();
	}

	public static void rmDir(File dir, String ext)
	{
		File[] fs=dir.listFiles();
		for (File f:fs)
		{
			if (f.isDirectory())
				rmDir(f);
			else
			if (f.getName().endsWith(ext))
				f.delete();
		}
		//dir.delete();
	}

}
