package com.androlua;
import android.os.*;
import com.androlua.util.*;
import com.luajava.*;
import java.io.*;
import java.net.*;
import java.nio.charset.*;
import java.util.*;

public class Http {

	private static HashMap<String,String> sHeader;

	public static void setHeader(HashMap<String, String> header) {
		sHeader = header;
	}

	public static HashMap<String, String> getHeader() {
		return sHeader;
	}


	public static HttpTask get(String url, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "GET", null, null, null, callback);
		task.execute();
		return task;
	}

	public static HttpTask get(String url, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "GET", null, null, header, callback);
		task.execute();
		return task;
	}

	public static HttpTask get(String url, String cookie, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "GET", null, cookie, header, callback) : new HttpTask(url, "GET", cookie, null, header, callback);  
		task.execute();
		return task;
	}

	public static HttpTask get(String url, String cookie, LuaObject callback) {
		Http.HttpTask task=cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "GET", null, cookie, null, callback) : new HttpTask(url, "GET", cookie, null, null, callback);  
		task.execute();
		return task;
	}

	public static HttpTask get(String url, String cookie, String charset, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "GET", cookie, charset, null, callback);
		task.execute();
		return task;
	}

	public static HttpTask get(String url, String cookie, String charset, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "GET", cookie, charset, header, callback);
		task.execute();
		return task;
	}

	public static HttpTask download(String url, String data, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "GET", null, null, null, callback);
		task.execute(data);
		return task;
	}

	public static HttpTask download(String url, String data, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "GET", null, null, header, callback);
		task.execute(data);
		return task;
	}

	public static HttpTask download(String url, String data, String cookie, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "GET", cookie, null, null, callback);
		task.execute(data);
		return task;
	}
	
	public static HttpTask download(String url, String data, String cookie, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "GET", cookie, null, header, callback);
		task.execute(data);
		return task;
	}

	
	public static HttpTask delete(String url, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "DELETE", null, null, null, callback);
		task.execute();
		return task;
	}

	public static HttpTask delete(String url, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "DELETE", null, null, header, callback);
		task.execute();
		return task;
	}

	public static HttpTask delete(String url, String cookie, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "DELETE", null, cookie, header, callback) : new HttpTask(url, "DELETE", cookie, null, header, callback);  
		task.execute();
		return task;
	}

	public static HttpTask delete(String url, String cookie, LuaObject callback) {
		Http.HttpTask task=cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "DELETE", null, cookie, null, callback) : new HttpTask(url, "DELETE", cookie, null, null, callback);  
		task.execute();
		return task;
	}

	public static HttpTask delete(String url, String cookie, String charset, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "DELETE", cookie, charset, null, callback);
		task.execute();
		return task;
	}

	public static HttpTask delete(String url, String cookie, String charset, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "DELETE", cookie, charset, header, callback);
		task.execute();
		return task;
	}


	public static HttpTask post(String url, String data, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "POST", null, null, null, callback);
		task.execute(data);
		return task;
	}

	public static HttpTask post(String url, String data, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "POST", null, null, header, callback);
		task.execute(data);
		return task;
	}

	public static HttpTask post(String url, String data, String cookie, LuaObject callback) {
		Http.HttpTask task=cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "POST", null, cookie, null, callback) : new HttpTask(url, "POST", cookie, null, null, callback);  
		task.execute(data);
		return task;
	}

	public static HttpTask post(String url, String data, String cookie, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "POST", null, cookie, header, callback) : new HttpTask(url, "POST", cookie, null, header, callback);  
		task.execute(data);
		return task;
	}

	public static HttpTask post(String url, String data, String cookie, String charset, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "POST", cookie, charset, null, callback);
		task.execute(data);
		return task;
	}

	public static HttpTask post(String url, String data, String cookie, String charset, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "POST", cookie, charset, header, callback);
		task.execute(data);
		return task;
	}


	public static HttpTask put(String url, String data, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "PUT", null, null, null, callback);
		task.execute(data);
		return task;
	}

	public static HttpTask put(String url, String data, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "PUT", null, null, header, callback);
		task.execute(data);
		return task;
	}

	public static HttpTask put(String url, String data, String cookie, LuaObject callback) {
		Http.HttpTask task=cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "PUT", null, cookie, null, callback) : new HttpTask(url, "PUT", cookie, null, null, callback);  
		task.execute(data);
		return task;
	}

	public static HttpTask put(String url, String data, String cookie, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "PUT", null, cookie, header, callback) : new HttpTask(url, "PUT", cookie, null, header, callback);  
		task.execute(data);
		return task;
	}

	public static HttpTask put(String url, String data, String cookie, String charset, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "PUT", cookie, charset, null, callback);
		task.execute(data);
		return task;
	}

	public static HttpTask put(String url, String data, String cookie, String charset, HashMap<String,String> header, LuaObject callback) {
		Http.HttpTask task=new HttpTask(url, "PUT", cookie, charset, header, callback);
		task.execute(data);
		return task;
	}


	public static class HttpTask extends AsyncTask {

		private String mUrl;

		private LuaObject mCallback;

		private byte[] mData;

		private String mCharset;

		private String mCookie;

		private HashMap<String,String> mHeader;

		private String mMethod;


		public HttpTask(String url, String method, String cookie, String charset, HashMap<String,String> header, LuaObject callback) {
			mUrl = url;
			mMethod = method;
			mCookie = cookie;
			mCharset = charset;
			mHeader = header;
			mCallback = callback;
		}


		@Override
		protected Object doInBackground(Object[] p1) {
			// TODO: Implement this method
			try {
				URL url = new URL(mUrl);

				HttpURLConnection conn = (HttpURLConnection) url.openConnection();
				conn.setConnectTimeout(6000);
				conn.setFollowRedirects(true);
				conn.setDoInput(true);
				conn.setRequestProperty("Accept-Language", "zh-cn,zh;q=0.5");

				if (mCharset == null)
					mCharset = "UTF-8";
				conn.setRequestProperty("Accept-Charset", mCharset);

				if (mCookie != null)
					conn.setRequestProperty("Cookie", mCookie); 

				if (sHeader != null) {
					Set<Map.Entry<String, String>> entries=sHeader.entrySet();
					for (Map.Entry<String, String> entry:entries) {
						conn.setRequestProperty(entry.getKey(), entry.getValue());
					}
				}

				if (mHeader != null) {
					Set<Map.Entry<String, String>> entries=mHeader.entrySet();
					for (Map.Entry<String, String> entry:entries) {
						conn.setRequestProperty(entry.getKey(), entry.getValue());
					}
				}

				if (mMethod != null)
					conn.setRequestMethod(mMethod);

				if (!"GET".equals(mMethod) && p1.length != 0) {
					mData = formatData(p1);

					conn.setDoOutput(true);
					conn.setRequestProperty("Content-length", "" + mData.length); 
				}

				conn.connect();

				//download
				if ("GET".equals(mMethod) && p1.length != 0) {
					File f=new File((String)p1[0]);
					if(!f.getParentFile().exists())
						f.getParentFile().mkdirs();
					FileOutputStream os=new FileOutputStream(f);
					InputStream is = conn.getInputStream();
					LuaUtil.copyFile(is,os);
					return new Object[]{conn.getResponseCode(),conn.getHeaderFields()};
				}
				
				//post upload
				if (p1.length != 0) {
					OutputStream os=conn.getOutputStream();
					os.write(mData);
				}

				int code=conn.getResponseCode();
				Map<String, List<String>> hs=conn.getHeaderFields();
				if (code >= 200 && code < 400) {
					String encoding=conn.getContentEncoding();
					List<String> cs=hs.get("Set-Cookie");
					StringBuffer cok=new StringBuffer();
					if (cs != null)
						for (String s:cs) {
							cok.append(s + ";");
						}

					InputStream is = conn.getInputStream();
					BufferedReader reader=new BufferedReader(new InputStreamReader(is, mCharset));
					StringBuffer buf=new StringBuffer();
					String line;
					while ((line = reader.readLine()) != null && !isCancelled())
						buf.append(line + '\n');
					is.close();
					return new Object[]{code,new String(buf),cok.toString(),hs};
				}
				else {
					return new Object[]{code,conn.getResponseMessage(),null,hs};
				}
			}
			catch (Exception e) {
				e.printStackTrace();
				return new Object[]{-1,e.getMessage()};
			}

		}

		private byte[] formatData(Object[] p1) throws UnsupportedEncodingException, IOException {
			// TODO: Implement this method
			byte[] bs = null;
			if (p1.length == 1) {
				Object obj=p1[0];
				if (obj instanceof String)
					bs = ((String)obj).getBytes(mCharset);
				else if (obj.getClass().getComponentType() == byte.class)
					bs = (byte[])obj;
				else if (obj instanceof File)
					bs = LuaUtil.readAll(new FileInputStream((File)obj));
				else if (obj instanceof File)
					bs = formatData((Map)obj);
			}
			return bs;
		}

		private byte[] formatData(Map obj) throws UnsupportedEncodingException {
			// TODO: Implement this method
			StringBuilder buf=new StringBuilder();
			Set<Map.Entry<String, String>> entries=mHeader.entrySet();
			for (Map.Entry<String, String> entry:entries) {
				buf.append(entry.getKey() + "=" + entry.getValue() + "&");
			}
			return buf.toString().getBytes(mCharset);
		}

		
		public boolean cancel() {
			// TODO: Implement this method
			return super.cancel(true);
		}

		
		
		
		@Override
		protected void onPostExecute(Object result) {
			// TODO: Implement this method
			super.onPostExecute(result);
			if(isCancelled())
				return;
			try {
				mCallback.call((Object[])result);
			}
			catch (LuaException e) {
				try {
					mCallback.getLuaState().getLuaObject("print").call(e.getMessage());
				}
				catch (LuaException e2) {}
				android.util.Log.d("lua", e.getMessage());
			}
		}

	}
}
