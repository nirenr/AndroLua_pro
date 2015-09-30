package com.androlua;


import android.content.*;
import android.content.res.*;
import android.graphics.*;
import java.io.*;
import java.net.*;

public class LuaBitmap
{
	public static Bitmap getLoacalBitmap(String url) throws FileNotFoundException, IOException
	{

		FileInputStream fis = new FileInputStream(url);
		Bitmap bitmap= BitmapFactory.decodeStream(fis);
		fis.close();
		return bitmap;
	}

	public static Bitmap getHttpBitmap(String url) throws IOException
	{
		//Log.d(TAG, url);
		URL myFileUrl = new URL(url);
		HttpURLConnection conn = (HttpURLConnection) myFileUrl.openConnection();
		//conn.setConnectTimeout(0);
		conn.setDoInput(true);
		conn.connect();
		InputStream is = conn.getInputStream();
		Bitmap bitmap = BitmapFactory.decodeStream(is);
		///Bitmap bitmap =decodeScale(is);
		is.close();
		return bitmap;
	}

	public static Bitmap getAssetBitmap(Context context, String name) throws IOException 
	{
		AssetManager am = context.getAssets();
		InputStream is = am.open(name);
		Bitmap bitmap= BitmapFactory.decodeStream(is);
		is.close();
		return bitmap;
	}



	private static Bitmap decodeScale(InputStream fis)
	{ 
		Bitmap b = null; 

		BitmapFactory.Options o = new BitmapFactory.Options();
		o.inJustDecodeBounds = true; 
		BitmapFactory.decodeStream(fis, null, o); 
		int scale = 1; 
		int IMAGE_MAX_SIZE = 720;
		if (o.outHeight > IMAGE_MAX_SIZE || o.outWidth > IMAGE_MAX_SIZE)
		{ 
			scale = (int)Math.pow(2, (int) Math.round(Math.log(IMAGE_MAX_SIZE / (double) Math.max(o.outHeight, o.outWidth)) / Math.log(0.5)));
		} 
		BitmapFactory.Options o2 = new BitmapFactory.Options(); 
		o2.inSampleSize = scale;

		b = BitmapFactory.decodeStream(fis, null, o2); 


		return b; 
	}
	
	public static Bitmap getImageFromPath( String filePath )
	{  

		Bitmap bitmap = null;  
		BitmapFactory.Options opts = new BitmapFactory.Options( );  
		opts.inJustDecodeBounds = true;  
		BitmapFactory.decodeFile( filePath,opts );  

		//缩放图片，避免内存不足
		opts.inSampleSize = computeSampleSize( opts,-1,250 * 250 );  
		opts.inJustDecodeBounds = false;  

		try
		{  
			bitmap = BitmapFactory.decodeFile( filePath,opts );  
		}
		catch (Exception e)
		{  
			// TODO: handle exception  
		}  
		return bitmap;  
	}  

	//缩放图片算法
	private static int computeSampleSize( BitmapFactory.Options options,int minSideLength,int maxNumOfPixels )
	{
		int initialSize = computeInitialSampleSize( options,minSideLength,maxNumOfPixels );
		int roundedSize;
		if (initialSize <= 8)
		{
			roundedSize = 1;
			while (roundedSize < initialSize)
			{
				roundedSize <<= 1;
			}
		}
		else
		{
			roundedSize = (initialSize + 7) / 8 * 8;
		}
		return roundedSize;
	}

	private static int computeInitialSampleSize( BitmapFactory.Options options,int minSideLength,int maxNumOfPixels )
	{
		double w = options.outWidth;
		double h = options.outHeight;
		int lowerBound = (maxNumOfPixels == -1) ? 1 : (int) Math.ceil( Math.sqrt( w * h / maxNumOfPixels ) );
		int upperBound = (minSideLength == -1) ? 128 : (int) Math.min( Math.floor( w / minSideLength ),Math.floor( h / minSideLength ) );
		if (upperBound < lowerBound)
		{
			// return the larger one when there is no overlapping zone.
			return lowerBound;
		}
		if ((maxNumOfPixels == -1) && (minSideLength == -1))
		{
			return 1;
		}
		else if (minSideLength == -1)
		{
			return lowerBound;
		}
		else
		{
			return upperBound;
		}
	} 
}
