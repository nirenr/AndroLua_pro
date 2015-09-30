package com.androlua;


import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Calendar;
import java.util.Locale;
import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.text.format.DateFormat;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.Window;
import android.view.WindowManager;
import android.widget.FrameLayout;
import android.widget.TextView;
import android.widget.Toast;
public class ca extends Activity
{
	private CameraView cv;
	//准备一个相机对象
	private Camera mCamera = null;
	//准备一个Bitmap对象
	private Bitmap mBitmap = null;
	//准备一个保存图片的PictureCallback对象
	public Camera.PictureCallback pictureCallback = new Camera.PictureCallback()

	{
		public void onPictureTaken(byte[] data, Camera camera)
		{	
			Log.i("yao", "onPictureTaken");
			Toast.makeText(getApplicationContext(), "正在保存……", Toast.LENGTH_LONG).show();

			//用BitmapFactory.decodeByteArray()方法可以把相机传回的裸数据转换成Bitmap对象
			mBitmap = BitmapFactory.decodeByteArray(data, 0, data.length);
			//接下来的工作就是把Bitmap保存成一个存储卡中的文件
			File file = new File("/sdcard/YY" + new DateFormat().format("yyyyMMdd_hhmmss", Calendar.getInstance(Locale.CHINA)) + ".jpg");
			try
			{
				file.createNewFile();
				BufferedOutputStream os = new BufferedOutputStream(new FileOutputStream(file));
				mBitmap.compress(Bitmap.CompressFormat.PNG, 100, os);
				os.flush();
				os.close();
				Toast.makeText(getApplicationContext(), "图片保存完毕，在存储卡的根目录", Toast.LENGTH_LONG).show();
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}	
		}  
	};
 	//Activity的创建方法	
	@Override	public void onCreate(Bundle savedInstanceState)
	{ 
		super.onCreate(savedInstanceState);
		//窗口去掉标题 
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		//窗口设置为全屏 
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		//设置窗口为半透明
		getWindow().setFormat(PixelFormat.TRANSLUCENT);
		//提供一个帧布局

		FrameLayout fl = new FrameLayout(this);
		//创建一个照相预览用的SurfaceView子类，并放在帧布局的底层 
		cv = new CameraView(this);
		fl.addView(cv);
		//创建一个文本框添加在帧布局中，我们可以看到，文字自动出现在了SurfaceView的前面，由此你可以在预览窗口做出各种特殊效果	
		TextView tv = new TextView(this);
		tv.setText("请按\"相机\"按钮拍摄");
		fl.addView(tv);
		//设置Activity的根内容视图	
		setContentView(fl);
 	} 	//相机按键按下的事件处理方法	
	public boolean onKeyDown(int keyCode, KeyEvent event) 
	{	
		Log.i("yao", "MainActivity.onKeyDown");
		if (keyCode == KeyEvent.KEYCODE_CAMERA) 
		{	
			if (mCamera != null)
			{ 
				Log.i("yao", "mCamera.takePicture");
				//当按下相机按钮时，执行相机对象的takePicture()方法,该方法有三个回调对象做入参，不需要的时候可以设null
				mCamera.takePicture(null, null, pictureCallback);
			}	
		}	
		return cv.onKeyDown(keyCode, event);
	} 	
	// 照相视图	
	class CameraView extends SurfaceView
	{ 
		private SurfaceHolder holder = null;
		//构造函数	
		public CameraView(Context context) 
		{ 
			super(context);
			Log.i("yao", "CameraView");
			// 操作surface的holder	
			holder = this.getHolder();
			// 创建SurfaceHolder.Callback对象
			holder.addCallback(new SurfaceHolder.Callback() 
				{ 
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
						mCamera.setParameters(parameters);
						//开始预览	
						mCamera.startPreview();	
					}	
				}
			);	 // 设置Push缓冲类型，说明surface数据由其他来源提供，而不是用自己的Canvas来绘图，在这里是由摄像头来提供数据 
			holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);	
		}
	}
}
