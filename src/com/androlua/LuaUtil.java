package com.androlua;

import android.app.*;
import android.content.*;
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
}
