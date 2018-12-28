package com.androlua;

import android.content.*;
import android.graphics.*;
import android.hardware.*;
import android.util.*;
import android.view.*;
import java.io.*;

import android.hardware.Camera;

class LuaCameraView extends SurfaceView
{ 
	private SurfaceHolder holder = null;
	//构造函数	
	public LuaCameraView(Context context) 
	{ 
		super(context);
		// 操作surface的holder	
		holder = this.getHolder();
		// 创建SurfaceHolder.Callback对象
		holder.addCallback(new SurfaceHolder.Callback() 
			{

				private Camera mCamera; 
				@Override public void surfaceDestroyed(SurfaceHolder holder)
				{ 
					// 停止预览	
					mCamera.stopPreview();
					// 释放相机资源并置空
					mCamera.release();
					mCamera = null;
				} 	
				@Override	
				public void surfaceCreated(SurfaceHolder holder)
				{	
					//当预览视图创建的时候开启相机	
					mCamera = Camera.open();
					try
					{	
						//设置预览
						mCamera.setPreviewDisplay(holder);
					}
					catch (IOException e)
					{	 // 释放相机资源并置空	 mCamera.release();
						mCamera = null;
					} 	
				} 	 
				//当surface视图数据发生变化时，处理预览信息
				@Override	
				public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
				{ 	 
					//获得相机参数对象	
					Camera.Parameters parameters = mCamera.getParameters();
					//设置格式 
					parameters.setPictureFormat(PixelFormat.JPEG);
					//设置预览大小，这里我的测试机是Milsstone所以设置的是854x480
					parameters.setPreviewSize(854, 480);
					//设置自动对焦
					parameters.setFocusMode("auto");
					//设置图片保存时的分辨率大小
					parameters.setPictureSize(2592, 1456);
					//给相机对象设置刚才设定的参数
					//mCamera.setParameters(parameters);
					//开始预览	
					mCamera.startPreview();	
				}	
			}
		);	 // 设置Push缓冲类型，说明surface数据由其他来源提供，而不是用自己的Canvas来绘图，在这里是由摄像头来提供数据 
		holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);	
	}
}
