package com.androlua;

import android.app.*;
import android.content.*;
import android.content.pm.*;
import android.graphics.*;
import android.os.*;
import android.view.*;
import android.widget.*;
import android.widget.ImageView.*;
import com.androlua.util.*;
import java.io.*;
import java.util.*;
import java.util.zip.*;


public class Welcome extends Activity {

	private boolean isUpdata;

	private LuaApplication app;

	private String luaMdDir;

	private String localDir;

	private long mLastTime;

	private long mOldLastTime;

	private ProgressDialog pd;

	private boolean isVersionChanged;

	private String mVersionName;

	private String mOldVersionName;

	private Dialog dlg;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
		app = (LuaApplication)getApplication();
		luaMdDir = app.luaMdDir;
		localDir = app.localDir;
		if (checkInfo()) {
			try {
				Bitmap welcome=LuaBitmap.getAssetBitmap(this, "welcome.png");
				dlg = new Dialog(this, android.R.style.Theme_DeviceDefault_Light_NoActionBar);
				ImageView img=new ImageView(this);
				img.setImageBitmap(welcome);
				img.setScaleType(ScaleType.FIT_END);
				img.setBackgroundColor(welcome.getPixel(1,1));
				dlg.setContentView(img);
				dlg.show();
			}
			catch (IOException e) {

				pd = new ProgressDialog(this, android.R.style.Theme_DeviceDefault_Light_Dialog_NoActionBar_MinWidth);
				Window w=pd.getWindow();
				WindowManager.LayoutParams lp=w.getAttributes();
				lp.alpha = (float) 0.75;
				w.setAttributes(lp);
				pd.setMessage("Loading...");
				pd.show();
			}
			new UpdateTask().execute();
		}
		else {
			startActivity();
		}
	}

	public void startActivity() {
		// TODO: Implement this method
		Intent intent = new Intent(Welcome.this, Main.class);
		if (isVersionChanged) {
			intent.putExtra("isVersionChanged", isVersionChanged);
			intent.putExtra("newVersionName", mVersionName);
			intent.putExtra("oldVersionName", mOldVersionName);
		}
		startActivity(intent);
		finish();
	}




	public boolean checkInfo() {
		try {
			PackageInfo packageInfo=getPackageManager().getPackageInfo(this.getPackageName(), 0);
			long lastTime=packageInfo.lastUpdateTime;
			String versionName=packageInfo.versionName;
			SharedPreferences info=getSharedPreferences("appInfo", 0);
			String oldVersionName=info.getString("versionName", "");
			if (!versionName.equals(oldVersionName)) {
				SharedPreferences.Editor edit=info.edit();
				edit.putString("versionName", versionName);
				edit.commit();
				isVersionChanged = true;
				mVersionName = versionName;
				mOldVersionName = oldVersionName;
			}
			long oldLastTime=info.getLong("lastUpdateTime", 0);
			if (oldLastTime != lastTime) {
				SharedPreferences.Editor edit=info.edit();
				edit.putLong("lastUpdateTime", lastTime);
				edit.commit();
				isUpdata = true;
				//onUpdata(lastTime, oldLastTime);
				mLastTime = lastTime;
				mOldLastTime = oldLastTime;
				return true;
			}
		}
		catch (PackageManager.NameNotFoundException e) {

		}
		return false;
	}


	class UpdateTask extends AsyncTask {
		@Override
		protected Object doInBackground(Object[] p1) {
			// TODO: Implement this method
			onUpdata(mLastTime, mOldLastTime);
			return null;
		}
		@Override
		protected void onPostExecute(Object result) {
			startActivity();
			//if (pd != null)
				//pd.dismiss();
			//if (dlg != null)
				//dlg.dismiss();

		}

		private void onUpdata(long lastTime, long oldLastTime) {

			try {
				//LuaUtil.rmDir(new File(localDir),".lua");
				//LuaUtil.rmDir(new File(luaMdDir),".lua");

				unApk("assets", localDir);
				unApk("lua", luaMdDir);
				//unZipAssets("main.alp", extDir);
			}
			catch (IOException e) {
				sendMsg(e.getMessage());
			}
		}

		private void sendMsg(String message) {
			// TODO: Implement this method
			
		}

		private void unApk(String dir, String extDir) throws IOException {
			int i=dir.length() + 1;
			ZipFile zip=new ZipFile(getApplicationInfo().publicSourceDir);
			Enumeration<? extends ZipEntry> entries=zip.entries();
			while (entries.hasMoreElements()) {
				ZipEntry entry=entries.nextElement();
				String name=entry.getName();
				if (name.indexOf(dir) != 0)
					continue;
				String path=name.substring(i);
				if (entry.isDirectory()) {
					File f=new File(extDir + File.separator + path);
					if (!f.exists())
						f.mkdirs();
				}
				else {
					String fname=extDir + File.separator + path;
					File ff=new File(fname);
					File temp=new File(fname).getParentFile();
					if (!temp.exists()) {
						if (!temp.mkdirs()) {
							throw new RuntimeException("create file " + temp.getName() + " fail");
						}
					}
					if (ff.exists() && entry.getSize() == ff.length() && LuaUtil.getFileMD5(ff).equals(LuaUtil.getFileMD5(zip.getInputStream(entry))))
						continue;

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

	}
}
