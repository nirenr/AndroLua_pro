package com.androlua;

import com.androlua.util.AsyncTaskX;
import com.luajava.LuaException;
import com.luajava.LuaObject;
import com.luajava.LuaString;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class Http {

    private static HashMap<String, String> sHeader;

    public static void setHeader(HashMap<String, String> header) {
        sHeader = header;
    }

    public static void setUserAgent(String userAgent) {
        if (sHeader == null)
            sHeader = new HashMap<>();
        sHeader.put("User-Agent", userAgent);
    }

   /* static {
        setUserAgent("Mozilla/5.0 (Linux; Android 8.0.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/63.0.3239.111 Mobile Safari/537.36 EdgA/41.0.0.1722");
    }*/

    public static HashMap<String, String> getHeader() {
        return sHeader;
    }

    public static HttpTask get(String url, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "GET", null, null, null, callback);
        task.execute();
        return task;
    }

    public static HttpTask get(String url, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "GET", null, null, header, callback);
        task.execute();
        return task;
    }

    public static HttpTask get(String url, String cookie, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "GET", null, cookie, header, callback) : new HttpTask(url, "GET", cookie, null, header, callback);
        task.execute();
        return task;
    }

    public static HttpTask get(String url, String cookie, LuaObject callback) {
        Http.HttpTask task = cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "GET", null, cookie, null, callback) : new HttpTask(url, "GET", cookie, null, null, callback);
        task.execute();
        return task;
    }

    public static HttpTask get(String url, String cookie, String charset, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "GET", cookie, charset, null, callback);
        task.execute();
        return task;
    }

    public static HttpTask get(String url, String cookie, String charset, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "GET", cookie, charset, header, callback);
        task.execute();
        return task;
    }

    public static HttpTask download(String url, String data, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "GET", null, null, null, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask download(String url, String data, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "GET", null, null, header, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask download(String url, String data, String cookie, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "GET", cookie, null, null, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask download(String url, String data, String cookie, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "GET", cookie, null, header, callback);
        task.execute(data);
        return task;
    }


    public static HttpTask delete(String url, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "DELETE", null, null, null, callback);
        task.execute();
        return task;
    }

    public static HttpTask delete(String url, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "DELETE", null, null, header, callback);
        task.execute();
        return task;
    }

    public static HttpTask delete(String url, String cookie, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "DELETE", null, cookie, header, callback) : new HttpTask(url, "DELETE", cookie, null, header, callback);
        task.execute();
        return task;
    }

    public static HttpTask delete(String url, String cookie, LuaObject callback) {
        Http.HttpTask task = cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "DELETE", null, cookie, null, callback) : new HttpTask(url, "DELETE", cookie, null, null, callback);
        task.execute();
        return task;
    }

    public static HttpTask delete(String url, String cookie, String charset, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "DELETE", cookie, charset, null, callback);
        task.execute();
        return task;
    }

    public static HttpTask delete(String url, String cookie, String charset, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "DELETE", cookie, charset, header, callback);
        task.execute();
        return task;
    }


    public static HttpTask post(String url, String data, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "POST", null, null, null, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask post(String url, String data, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "POST", null, null, header, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask post(String url, String data, String cookie, LuaObject callback) {
        Http.HttpTask task = cookie.matches("[\\w\\-.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "POST", null, cookie, null, callback) : new HttpTask(url, "POST", cookie, null, null, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask post(String url, String data, String cookie, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = cookie.matches("[\\w\\-.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "POST", null, cookie, header, callback) : new HttpTask(url, "POST", cookie, null, header, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask post(String url, String data, String cookie, String charset, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "POST", cookie, charset, null, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask post(String url, String data, String cookie, String charset, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "POST", cookie, charset, header, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask post(String url, HashMap<String, String> data, LuaObject callback) {
        return post(url, formatMap(data), callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, String cookie, LuaObject callback) {
        return post(url, formatMap(data), cookie, callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, String cookie, HashMap<String, String> header, LuaObject callback) {
        return post(url, formatMap(data), cookie, header, callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, String cookie, String charset, LuaObject callback) {
        return post(url, formatMap(data), cookie, charset, callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, String cookie, String charset, HashMap<String, String> header, LuaObject callback) {
        return post(url, formatMap(data), cookie, charset, header, callback);
    }

    private static String formatMap(HashMap<String, String> data) {
        StringBuilder buf = new StringBuilder();
        for (Map.Entry<String, String> entry : data.entrySet()) {
            buf.append(entry.getKey()).append("=").append(entry.getValue()).append("&");
        }
        if (!data.isEmpty())
            buf.deleteCharAt(buf.length() - 1);
        return buf.toString();
    }


    private final static String boundary = "----qwertyuiopasdfghjklzxcvbnm";

    public static HttpTask post(String url, HashMap<String, String> data, HashMap<String, String> file, LuaObject callback) {
        return post(url, data, file, null, null, null, callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, HashMap<String, String> file, String cookie, LuaObject callback) {
        return post(url, data, file, cookie, new HashMap<String, String>(), callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, HashMap<String, String> file, HashMap<String, String> header, LuaObject callback) {
        return post(url, data, file, null, header, callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, HashMap<String, String> file, String cookie, HashMap<String, String> header, LuaObject callback) {
        return cookie.matches("[\\w\\-.:]+") && Charset.isSupported(cookie) ? post(url, data, file, cookie, null, header, callback) : post(url, data, file, null, cookie, header, callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, HashMap<String, String> file, String cookie, String charset, LuaObject callback) {
        return post(url, data, file, cookie, charset, null, callback);
    }

    public static HttpTask post(String url, HashMap<String, String> data, HashMap<String, String> file, String cookie, String charset, HashMap<String, String> header, LuaObject callback) {
        if (header == null)
            header = new HashMap<>();
        header.put("Content-Type", "multipart/form-data;boundary=" + boundary);
        Http.HttpTask task = new HttpTask(url, "POST", cookie, charset, header, callback);
        task.execute(new Object[]{formatMultiDate(data, file, charset)});
        return task;
    }

    private static byte[] formatMultiDate(HashMap<String, String> data, HashMap<String, String> file, String charset) {
        if (charset == null)
            charset = "UTF-8";
        ByteArrayOutputStream buff = new ByteArrayOutputStream();
        for (Map.Entry<String, String> entry : data.entrySet()) {
            try {
                buff.write(String.format("--%s\r\nContent-Disposition:form-data;name=\"%s\"\r\n\r\n%s\r\n", boundary, entry.getKey(), entry.getValue()).getBytes(charset));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        for (Map.Entry<String, String> entry : file.entrySet()) {
            try {
                buff.write(String.format("--%s\r\nContent-Disposition:form-data;name=\"%s\";filename=\"%s\"\r\nContent-Type:application/octet-stream\r\n\r\n", boundary, entry.getKey(), entry.getValue()).getBytes(charset));
                buff.write(LuaUtil.readAll(new FileInputStream(entry.getValue())));
                buff.write("\r\n".getBytes(charset));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        return buff.toByteArray();
    }


    public static HttpTask put(String url, String data, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "PUT", null, null, null, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask put(String url, String data, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "PUT", null, null, header, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask put(String url, String data, String cookie, LuaObject callback) {
        Http.HttpTask task = cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "PUT", null, cookie, null, callback) : new HttpTask(url, "PUT", cookie, null, null, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask put(String url, String data, String cookie, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = cookie.matches("[\\w\\-\\.:]+") && Charset.isSupported(cookie) ? new HttpTask(url, "PUT", null, cookie, header, callback) : new HttpTask(url, "PUT", cookie, null, header, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask put(String url, String data, String cookie, String charset, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "PUT", cookie, charset, null, callback);
        task.execute(data);
        return task;
    }

    public static HttpTask put(String url, String data, String cookie, String charset, HashMap<String, String> header, LuaObject callback) {
        Http.HttpTask task = new HttpTask(url, "PUT", cookie, charset, header, callback);
        task.execute(data);
        return task;
    }


    public static class HttpTask extends AsyncTaskX<Object, Object, Object> {

        private String mUrl;

        private LuaObject mCallback;

        private byte[] mData;

        private String mCharset;

        private String mOutCharset;

        private String mCookie;

        private HashMap<String, String> mHeader;

        private String mMethod;


        public HttpTask(String url, String method, String cookie, String charset, HashMap<String, String> header, LuaObject callback) {
            mUrl = url;
            mMethod = method;
            mCookie = cookie;
            mCharset = charset;
            mOutCharset=charset;
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
                HttpURLConnection.setFollowRedirects(true);
                conn.setDoInput(true);
                conn.setRequestProperty("Accept-Language", "zh-cn,zh;q=0.5");

                if (mCharset == null)
                    mCharset = "UTF-8";
                conn.setRequestProperty("Accept-Charset", mCharset);

                if (mCookie != null)
                    conn.setRequestProperty("Cookie", mCookie);

                if (sHeader != null) {
                    Set<Map.Entry<String, String>> entries = sHeader.entrySet();
                    for (Map.Entry<String, String> entry : entries) {
                        conn.setRequestProperty(entry.getKey(), entry.getValue());
                    }
                }

                if (mHeader != null) {
                    Set<Map.Entry<String, String>> entries = mHeader.entrySet();
                    for (Map.Entry<String, String> entry : entries) {
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
                    File f = new File((String) p1[0]);
                    if (!f.getParentFile().exists())
                        //noinspection ResultOfMethodCallIgnored
                        f.getParentFile().mkdirs();
                    FileOutputStream os = new FileOutputStream(f);
                    InputStream is = conn.getInputStream();
                    LuaUtil.copyFile(is, os);
                    return new Object[]{conn.getResponseCode(), p1[0], conn.getHeaderFields()};
                }

                //post upload
                if (p1.length != 0) {
                    OutputStream os = conn.getOutputStream();
                    os.write(mData);
                }

                int code = conn.getResponseCode();
                Map<String, List<String>> hs = conn.getHeaderFields();
                String encoding = conn.getContentEncoding();
                List<String> cs = hs.get("Set-Cookie");
                StringBuilder cok = new StringBuilder();
                if (cs != null) {
                    for (String s : cs) {
                        cok.append(s).append(";");
                    }
                }

                List<String> ct = hs.get("Content-Type");
                if (ct != null) {
                    for (String s : ct) {
                        int idx = s.indexOf("charset");
                        if (idx != -1) {
                            idx = s.indexOf("=", idx);
                            if (idx != -1) {
                                int idx2 = s.indexOf(";", idx);
                                if (idx2 == -1)
                                    idx2 = s.length();
                                mCharset = s.substring(idx + 1, idx2);
                                break;
                            }
                        }
                    }
                }

                if(mOutCharset==null){
                   try {
                        InputStream is = conn.getInputStream();
                        byte[] reader = LuaUtil.readAll(is);
                        is.close();
                        LuaString buf = new LuaString(reader);
                        return new Object[]{code, buf, cok.toString(), hs};
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
                StringBuilder buf = new StringBuilder();
                try {
                    InputStream is = conn.getInputStream();
                    BufferedReader reader = new BufferedReader(new InputStreamReader(is, mCharset));
                    String line;
                    while ((line = reader.readLine()) != null && !isCancelled())
                        buf.append(line).append('\n');
                    is.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                InputStream is = conn.getErrorStream();
                if (is != null) {
                    BufferedReader reader = new BufferedReader(new InputStreamReader(is, mCharset));
                    String line;
                    while ((line = reader.readLine()) != null && !isCancelled())
                        buf.append(line).append('\n');
                    is.close();
                }
                return new Object[]{code, new String(buf), cok.toString(), hs};
            } catch (Exception e) {
                e.printStackTrace();
                return new Object[]{-1, e.getMessage()};
            }

        }

        private byte[] formatData(Object[] p1) throws UnsupportedEncodingException, IOException {
            // TODO: Implement this method
            byte[] bs = null;
            if (p1.length == 1) {
                Object obj = p1[0];
                if (obj instanceof String)
                    bs = ((String) obj).getBytes(mCharset);
                else if (obj.getClass().getComponentType() == byte.class)
                    bs = (byte[]) obj;
                else if (obj instanceof File)
                    bs = LuaUtil.readAll(new FileInputStream((File) obj));
                else if (obj instanceof Map)
                    bs = formatData((Map<String, String>) obj);
            }
            return bs;
        }

        private byte[] formatData(Map<String, String> obj) throws UnsupportedEncodingException {
            // TODO: Implement this method
            StringBuilder buf = new StringBuilder();
            Set<Map.Entry<String, String>> entries = obj.entrySet();
            for (Map.Entry<String, String> entry : entries) {
                buf.append(entry.getKey()).append("=").append(entry.getValue()).append("&");
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
            if (isCancelled())
                return;
            try {
                mCallback.call((Object[]) result);
            } catch (LuaException e) {
                try {
                    mCallback.getLuaState().getLuaObject("print").call(e.getMessage());
                } catch (LuaException e2) {
                }
                android.util.Log.i("lua", e.getMessage());
            }
        }

    }
}
