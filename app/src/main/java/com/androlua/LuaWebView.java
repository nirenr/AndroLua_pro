package com.androlua;

import android.annotation.SuppressLint;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.net.http.SslError;
import android.os.Build;
import android.os.Bundle;
import android.os.Message;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.KeyEvent;
import android.view.View;
import android.webkit.ClientCertRequest;
import android.webkit.DownloadListener;
import android.webkit.HttpAuthHandler;
import android.webkit.JavascriptInterface;
import android.webkit.JsPromptResult;
import android.webkit.JsResult;
import android.webkit.SslErrorHandler;
import android.webkit.ValueCallback;
import android.webkit.WebChromeClient;
import android.webkit.WebResourceRequest;
import android.webkit.WebResourceResponse;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.luajava.LuaException;
import com.luajava.LuaFunction;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

public class LuaWebView extends WebView implements LuaGcable {

    private LuaWebView.DownloadBroadcastReceiver mDownloadBroadcastReceiver;
    private HashMap<Long, String[]> mDownload = new HashMap<Long, String[]>();
    private OnDownloadCompleteListener mOnDownloadCompleteListener;
    private LuaActivity mContext;
    private ProgressBar mProgressbar;
    private DisplayMetrics dm;
    private Dialog open_dlg;
    private ListView open_list;
    private ValueCallback<Uri> mUploadMessage;
    private String mDir = "/";
    private LuaFunction<Boolean> mAdsFilter;
    private boolean mGc;

    @SuppressLint({"AddJavascriptInterface", "SetJavaScriptEnabled"})
    public LuaWebView(LuaActivity context) {
        super(context);
        context.regGc(this);
        mContext = context;
        getSettings().setJavaScriptEnabled(true);
        getSettings().setJavaScriptCanOpenWindowsAutomatically(true);
        getSettings().setDisplayZoomControls(true);
        getSettings().setSupportZoom(true);
        getSettings().setDomStorageEnabled(true);
        /*setOnLongClickListener(new OnLongClickListener() {
            @Override
            public boolean onLongClick(View v) {
                return true;
            }
        });*/
        if (Build.VERSION.SDK_INT >= 21)
            getSettings().setMixedContentMode(WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);
        //getSettings().setUseWideViewPort(true);
        //getSettings().setLoadWithOverviewMode(true);
        //getSettings().setDefaultZoom(WebSettings.ZoomDensity.FAR);
        addJavascriptInterface(new LuaJavaScriptInterface(context), "androlua");
        //requestFocus();
        setWebViewClient(new WebViewClient() {
                             public boolean shouldOverrideUrlLoading(WebView view, String url) {
                                 if (mAdsFilter != null) {
                                     try {
                                         Boolean ret = mAdsFilter.call(url);
                                         if (ret!=null&&ret)
                                             return true;
                                     } catch (LuaException e) {
                                         e.printStackTrace();
                                     }
                                 }

                                 if (url.startsWith("http") || url.startsWith("file")) {
                                     view.loadUrl(url);
                                     return true;
                                 } else {
                                     try{
                                         mContext.startActivityForResult(new Intent(Intent.ACTION_VIEW, Uri.parse(url)), 0);
                                     }catch (Exception e){
                                         mContext.sendError("LuaWebView",e);
                                     }
                                    return true;
                                 }
                             }

                             @SuppressWarnings("deprecation")
                             public WebResourceResponse shouldInterceptRequest(WebView view, String url) {
                                 if (mAdsFilter != null) {
                                     try {
                                         Boolean ret = mAdsFilter.call(url);
                                         if (ret!=null&&ret)
                                             return new WebResourceResponse(null, null, null);
                                     } catch (LuaException e) {
                                         e.printStackTrace();
                                     }
                                 }
                                 return null;
                             }

                             @Override
                             public void onReceivedSslError(WebView view, final SslErrorHandler handler,
                                                            SslError error) {
                                 AlertDialog.Builder b = new AlertDialog.Builder(mContext);
                                 b.setTitle("SslError");
                                 b.setMessage(error.toString());
                                 b.setPositiveButton(android.R.string.ok,
                                         new AlertDialog.OnClickListener() {
                                             public void onClick(DialogInterface dialog,
                                                                 int which) {
                                                 handler.proceed();
                                             }
                                         });
                                 b.setNegativeButton(android.R.string.cancel,
                                         new DialogInterface.OnClickListener() {
                                             public void onClick(DialogInterface dialog,
                                                                 int which) {
                                                 handler.cancel();
                                                 ;
                                             }
                                         });
                                 b.setCancelable(false);
                                 b.create();
                                 b.show();
                             }
                         }
        );

        dm = context.getResources().getDisplayMetrics();
        int top = (int) TypedValue.applyDimension(1, 2, dm);

        mProgressbar = new ProgressBar(context, null, android.R.attr.progressBarStyleHorizontal);
        mProgressbar.setLayoutParams(new LayoutParams(LayoutParams.FILL_PARENT, top, 0, 0));
        addView(mProgressbar);

        setWebChromeClient(new LuaWebChromeClient());
        setDownloadListener(new Download());


    }

    @Override
    public void gc() {
        destroy();
        mGc=true;
    }

    @Override
    public boolean isGc() {
        return mGc;
    }

    public void setProgressBarEnabled(boolean visibility) {
        if (visibility)
            mProgressbar.setVisibility(VISIBLE);
        else
            mProgressbar.setVisibility(GONE);
    }

    public void setProgressBar(ProgressBar pb) {
        mProgressbar = pb;
    }

    @Override
    protected void onScrollChanged(int l, int t, int oldl, int oldt) {
        LayoutParams lp = (LayoutParams) mProgressbar.getLayoutParams();
        lp.x = l;
        lp.y = t;
        mProgressbar.setLayoutParams(lp);
        super.onScrollChanged(l, t, oldl, oldt);
    }

    @Override
    public void setDownloadListener(DownloadListener listener) {
        super.setDownloadListener(listener);
    }

    public void setOnDownloadStartListener(final OnDownloadStartListener listener) {
        setDownloadListener(new DownloadListener() {
            @Override
            public void onDownloadStart(String p1, String p2, String p3, String p4, long p5) {
                listener.onDownloadStart(p1, p2, p3, p4, p5);
            }
        });
    }

    public void setOnDownloadCompleteListener(OnDownloadCompleteListener listener) {
        mOnDownloadCompleteListener = listener;
    }

    @Override
    public void destroy() {
        // TODO: Implement this method
        if (mDownloadBroadcastReceiver != null) {
            mContext.unregisterReceiver(mDownloadBroadcastReceiver);
        }
        super.destroy();
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if ((keyCode == KeyEvent.KEYCODE_BACK) && canGoBack()) {
            goBack();
            return true;
        }
        return super.onKeyDown(keyCode, event);
    }

    @Override
    public void setOnKeyListener(View.OnKeyListener l) {
        // TODO: Implement this method
        super.setOnKeyListener(l);
    }

    @SuppressLint("AddJavascriptInterface")
    public void addJSInterface(JsInterface object, String name) {
        // TODO: Implement this method
        super.addJavascriptInterface(new JsObject(object), name);
    }

    @SuppressLint("AddJavascriptInterface")
    public void addJsInterface(JsInterface object, String name) {
        // TODO: Implement this method
        super.addJavascriptInterface(new JsObject(object), name);
    }

    public void setWebViewClient(LuaWebViewClient client) {
        // TODO: Implement this method
        super.setWebViewClient(new SimpleLuaWebViewClient(client));
    }

    public void openFile(String dir) {
        if (open_dlg == null) {
            open_dlg = new Dialog(getContext());
            open_list = new ListView(getContext());
            open_list.setFastScrollEnabled(true);
            open_list.setFastScrollAlwaysVisible(true);
            open_dlg.setContentView(open_list);

            open_list.setOnItemClickListener(new AdapterView.OnItemClickListener() {

                @Override
                public void onItemClick(AdapterView<?> p1, View p2, int p3, long p4) {
                    String t = ((TextView) p2).getText().toString();
                    if (t.equals("../")) {
                        mDir = new File(mDir).getParent() + "/";
                        openFile(mDir);
                        return;
                    }
                    String fn = mDir + t;
                    File f = new File(fn);
                    if (f.isDirectory()) {
                        mDir = fn;
                        openFile(mDir);
                        return;
                    }
                    mUploadMessage.onReceiveValue(Uri.parse(fn));
                }
            });

        }

        File d = new File(dir);
        ArrayList<String> ns = new ArrayList<String>();
        ns.add("../");


        String[] fs = d.list();
        if (fs != null) {
            Arrays.sort(fs);
            for (String k : fs) {
                if (new File(mDir + k).isDirectory())
                    ns.add(k + "/");
            }

            for (String k : fs) {
                if (new File(mDir + k).isFile())
                    ns.add(k);
            }

        }

        ArrayAdapter<String> adapter = new ArrayAdapter<>(getContext(), android.R.layout.simple_list_item_1, ns);
        open_list.setAdapter(adapter);
        open_dlg.setTitle(mDir);
        open_dlg.show();
    }


    public interface OnDownloadCompleteListener {
        public abstract void onDownloadComplete(String fileName, String mimetype);
    }


    public interface OnDownloadStartListener {
        public abstract void onDownloadStart(String url, String userAgent, String contentDisposition, String mimetype, long contentLength);
    }

    public interface JsInterface {
        @JavascriptInterface
        public String execute(String arg);
    }

    public interface LuaWebViewClient {

        /**
         * Generic error
         */
        public static final int ERROR_UNKNOWN = -1;
        /**
         * Server or proxy hostname lookup failed
         */
        public static final int ERROR_HOST_LOOKUP = -2;
        /**
         * Unsupported authentication scheme (not basic or digest)
         */
        public static final int ERROR_UNSUPPORTED_AUTH_SCHEME = -3;
        /**
         * User authentication failed on server
         */
        public static final int ERROR_AUTHENTICATION = -4;
        /**
         * User authentication failed on proxy
         */
        public static final int ERROR_PROXY_AUTHENTICATION = -5;
        /**
         * Failed to connect to the server
         */
        public static final int ERROR_CONNECT = -6;


        // These ints must match up to the hidden values in EventHandler.
        /**
         * Failed to read or write to the server
         */
        public static final int ERROR_IO = -7;
        /**
         * Connection timed out
         */
        public static final int ERROR_TIMEOUT = -8;
        /**
         * Too many redirects
         */
        public static final int ERROR_REDIRECT_LOOP = -9;
        /**
         * Unsupported URI scheme
         */
        public static final int ERROR_UNSUPPORTED_SCHEME = -10;
        /**
         * Failed to perform SSL handshake
         */
        public static final int ERROR_FAILED_SSL_HANDSHAKE = -11;
        /**
         * Malformed URL
         */
        public static final int ERROR_BAD_URL = -12;
        /**
         * Generic file error
         */
        public static final int ERROR_FILE = -13;
        /**
         * File not found
         */
        public static final int ERROR_FILE_NOT_FOUND = -14;
        /**
         * Too many requests during this load
         */
        public static final int ERROR_TOO_MANY_REQUESTS = -15;

        public boolean shouldOverrideUrlLoading(WebView view, String url);

        public void onPageStarted(WebView view, String url, Bitmap favicon);

        public void onPageFinished(WebView view, String url);

        public void onLoadResource(WebView view, String url);

        public WebResourceResponse shouldInterceptRequest(WebView view,
                                                          String url);

        @Deprecated
        public void onTooManyRedirects(WebView view, Message cancelMsg,
                                       Message continueMsg);

        public void onReceivedError(WebView view, int errorCode,
                                    String description, String failingUrl);


        public void onFormResubmission(WebView view, Message dontResend,
                                       Message resend);


        public void doUpdateVisitedHistory(WebView view, String url,
                                           boolean isReload);


        public void onReceivedSslError(WebView view, SslErrorHandler handler,
                                       SslError error);


        public void onProceededAfterSslError(WebView view, SslError error);


        public void onReceivedClientCertRequest(WebView view,
                                                ClientCertRequest handler, String host_and_port);


        public void onReceivedHttpAuthRequest(WebView view,
                                              HttpAuthHandler handler, String host, String realm);


        public boolean shouldOverrideKeyEvent(WebView view, KeyEvent event);


        public void onUnhandledKeyEvent(WebView view, KeyEvent event);


        public void onScaleChanged(WebView view, float oldScale, float newScale);


        public void onReceivedLoginRequest(WebView view, String realm,
                                           String account, String args);

    }

    private final static String DOWNLOAD="Download";

    private class DownloadBroadcastReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context p1, Intent p2) {
            // TODO: Implement this method
            //id=p2.getLongExtra("flg", 0);
            //int id=p2.getFlags();
            long id = p2.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, 0);
            Bundle bundle = p2.getExtras();
            //bundle.g
            if (mDownload.containsKey(id) ) {
                if(mOnDownloadCompleteListener != null){
                    String[] data = mDownload.get(id);
                    mOnDownloadCompleteListener.onDownloadComplete(data[0], data[1]);
                }else{

                }
             }
        }
    }

    private class Download implements DownloadListener {

        EditText file_input_field;
        private String mUrl;
        private String mUserAgent;
        private String mContentDisposition;
        private String mMimetype;
        private long mContentLength;
        private String mFilename;

        @SuppressLint("DefaultLocale")
        @Override
        public void onDownloadStart(String url, String userAgent, String contentDisposition, String mimetype, long contentLength) {
            // TODO: Implement this method
            mUrl = url;
            mUserAgent = userAgent;
            mContentDisposition = contentDisposition;
            mMimetype = mimetype;
            mContentLength = contentLength;
            Uri uri = Uri.parse(mUrl);
            mFilename = uri.getLastPathSegment();
            if (contentDisposition != null) {
                String p = "filename=\"";
                int i = contentDisposition.indexOf(p);
                if (i != -1) {
                    i += p.length();
                    int n = contentDisposition.indexOf('"', i);
                    if (n > i)
                        mFilename = contentDisposition.substring(i, n);
                }
            }
            file_input_field = new EditText(mContext);
            //file_input_field.setTextColor(0xff000000);
            file_input_field.setText(mFilename);
            String size=String.valueOf(contentLength)+"B";
            if(contentLength>1024*1024)
                size=String.format("%.2f MB",Long.valueOf(contentLength).doubleValue()/(1024*1024));
            else if(contentLength>1024)
                size=String.format("%.2f KB",Long.valueOf(contentLength).doubleValue()/(1024));

            new AlertDialog.Builder(mContext)
                    .setTitle(DOWNLOAD)
                    .setMessage("Type: " + mimetype + "\nSize: " + size)
                    .setView(file_input_field)
                    .setPositiveButton(DOWNLOAD, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface p1, int p2) {
                            // TODO: Implement this method
                            mFilename = file_input_field.getText().toString();
                            download(false);
                        }
                    })
                    .setNegativeButton(android.R.string.cancel, null)
                    .setNeutralButton("Only Wifi", new DialogInterface.OnClickListener() {

                        @Override
                        public void onClick(DialogInterface p1, int p2) {
                            // TODO: Implement this method
                            mFilename = file_input_field.getText().toString();
                            download(true);
                        }
                    })
                    .create()
                    .show();
        }

        private long download(boolean isWifi) {
            if (mDownloadBroadcastReceiver == null) {
                IntentFilter filter = new IntentFilter();
                filter.addAction(DownloadManager.ACTION_DOWNLOAD_COMPLETE);
                mDownloadBroadcastReceiver = new DownloadBroadcastReceiver();
                mContext.registerReceiver(mDownloadBroadcastReceiver, filter);
            }

            DownloadManager downloadManager = (DownloadManager) mContext.getSystemService(Context.DOWNLOAD_SERVICE);

            Uri uri = Uri.parse(mUrl);
            uri.getLastPathSegment();
            DownloadManager.Request request = new DownloadManager.Request(uri);
            String dir = mContext.getLuaExtDir(DOWNLOAD);
            request.setDestinationInExternalPublicDir(new File(mContext.getLuaExtDir()).getName()+"/"+DOWNLOAD, mFilename);

            request.setTitle(mFilename);

            request.setDescription(mUrl);

            //request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
            if (isWifi)
                request.setAllowedNetworkTypes(DownloadManager.Request.NETWORK_WIFI);

            //request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_HIDDEN);
            File f = new File(dir, mFilename);
            if(f.exists())
                f.delete();

            request.setMimeType(mMimetype);
            //Environment.getExternalStoragePublicDirectory(dirType)
            long downloadId = downloadManager.enqueue(request);
            mDownload.put(downloadId, new String[]{new File(dir, mFilename).getAbsolutePath(), mMimetype});
            return downloadId;
        }
    }

    class JsObject {
        private LuaWebView.JsInterface mJs;

        public JsObject(JsInterface js) {
            mJs = js;
        }

        @JavascriptInterface
        public String execute(String arg) {
            return mJs.execute(arg);
        }

        ;
    }

    private class LuaJavaScriptInterface {

        private LuaActivity mMain;

        public LuaJavaScriptInterface(LuaActivity main) {
            mMain = main;
        }

        @JavascriptInterface
        public Object callLuaFunction(String name) {
            return mMain.runFunc(name);
        }

        @JavascriptInterface
        public Object callLuaFunction(String name, String arg) {
            return mMain.runFunc(name, arg);
        }

        @JavascriptInterface
        public Object doLuaString(String name) {
            return mMain.doString(name);
        }
    }

    public void setAdsFilter(LuaFunction<Boolean> filter) {
        mAdsFilter = filter;
    }


    private class SimpleLuaWebViewClient extends WebViewClient {

        private LuaWebView.LuaWebViewClient mLuaWebViewClient;

        public SimpleLuaWebViewClient(LuaWebViewClient wvc) {
            mLuaWebViewClient = wvc;
        }

        public boolean shouldOverrideUrlLoading(WebView view, String url) {
            return mLuaWebViewClient.shouldOverrideUrlLoading(view, url);
        }

        public void onPageStarted(WebView view, String url, Bitmap favicon) {
            mLuaWebViewClient.onPageStarted(view, url, favicon);
        }

        public void onPageFinished(WebView view, String url) {
            mLuaWebViewClient.onPageFinished(view, url);
        }

        public void onLoadResource(WebView view, String url) {
            mLuaWebViewClient.onLoadResource(view, url);
        }

        @SuppressWarnings("deprecation")
        public WebResourceResponse shouldInterceptRequest(WebView view, String url) {
            if (mAdsFilter != null) {
                try {
                    if (mAdsFilter.call(url))
                        return new WebResourceResponse(null, null, null);
                } catch (LuaException e) {
                    e.printStackTrace();
                }
            }
            return mLuaWebViewClient.shouldInterceptRequest(view, url);
        }

        @Override
        public WebResourceResponse shouldInterceptRequest(WebView view, WebResourceRequest request) {
            return super.shouldInterceptRequest(view, request);
        }

        @SuppressWarnings("deprecation")
        @Deprecated
        public void onTooManyRedirects(WebView view, Message cancelMsg,
                                       Message continueMsg) {
            cancelMsg.sendToTarget();
        }

        public void onReceivedError(WebView view, int errorCode,
                                    String description, String failingUrl) {
            mLuaWebViewClient.onReceivedError(view, errorCode, description, failingUrl);
        }

        public void onFormResubmission(WebView view, Message dontResend,
                                       Message resend) {
            dontResend.sendToTarget();
        }

        public void doUpdateVisitedHistory(WebView view, String url,
                                           boolean isReload) {
            mLuaWebViewClient.doUpdateVisitedHistory(view, url, isReload);
        }

        @Override
        public void onReceivedSslError(WebView view, final SslErrorHandler handler,
                                       SslError error) {
            mLuaWebViewClient.onReceivedSslError(view, handler, error);
        }

        public void onProceededAfterSslError(WebView view, SslError error) {
            mLuaWebViewClient.onProceededAfterSslError(view, error);
        }

        public void onReceivedClientCertRequest(WebView view,
                                                ClientCertRequest handler, String host_and_port) {
            mLuaWebViewClient.onReceivedClientCertRequest(view, handler, host_and_port);
        }

        public void onReceivedHttpAuthRequest(WebView view,
                                              HttpAuthHandler handler, String host, String realm) {
            mLuaWebViewClient.onReceivedHttpAuthRequest(view, handler, host, realm);
        }

        public boolean shouldOverrideKeyEvent(WebView view, KeyEvent event) {
            return mLuaWebViewClient.shouldOverrideKeyEvent(view, event);
        }

        public void onUnhandledKeyEvent(WebView view, KeyEvent event) {
            mLuaWebViewClient.onUnhandledKeyEvent(view, event);
        }

        public void onScaleChanged(WebView view, float oldScale, float newScale) {
            mLuaWebViewClient.onScaleChanged(view, oldScale, newScale);
        }

        public void onReceivedLoginRequest(WebView view, String realm,
                                           String account, String args) {
            mLuaWebViewClient.onReceivedLoginRequest(view, realm, account, args);
        }
    }

    class LuaWebChromeClient extends WebChromeClient {
        EditText prompt_input_field = new EditText(mContext);

        @Override
        public boolean onJsAlert(WebView view, String url, String message, final android.webkit.JsResult result) {
            new AlertDialog.Builder(mContext)
                    .setTitle(url)
                    .setMessage(message)
                    .setPositiveButton(android.R.string.ok,
                            new AlertDialog.OnClickListener() {
                                public void onClick(DialogInterface dialog, int which) {
                                    result.confirm();
                                }
                            })
                    .setCancelable(false)
                    .create()
                    .show();
            return true;
        }

        ;

        @Override
        public boolean onJsConfirm(WebView view, String url,
                                   String message, final JsResult result) {
            AlertDialog.Builder b = new AlertDialog.Builder(mContext);
            b.setTitle(url);
            b.setMessage(message);
            b.setPositiveButton(android.R.string.ok,
                    new AlertDialog.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                            int which) {
                            result.confirm();
                        }
                    });
            b.setNegativeButton(android.R.string.cancel,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                            int which) {
                            result.cancel();
                        }
                    });
            b.setCancelable(false);
            b.create();
            b.show();
            return true;
        }

        ;

        @Override
        public boolean onJsPrompt(WebView view, String url, String message,
                                  String defaultValue, final JsPromptResult result) {
            prompt_input_field.setText(defaultValue);
            AlertDialog.Builder b = new AlertDialog.Builder(mContext);
            b.setTitle(url);
            b.setMessage(message);
            b.setView(prompt_input_field);
            b.setPositiveButton(android.R.string.ok,
                    new AlertDialog.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                            int which) {
                            String value = prompt_input_field
                                    .getText().toString();
                            result.confirm(value);
                        }
                    });
            b.setNegativeButton(android.R.string.cancel,
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog,
                                            int which) {
                            result.cancel();
                        }
                    });
            b.setOnCancelListener(new DialogInterface.OnCancelListener() {
                public void onCancel(DialogInterface dialog) {
                    result.cancel();
                }
            });
            b.show();
            return true;
        }

        ;

        @Override
        public void onProgressChanged(WebView view, int newProgress) {
            //mContext.setProgressBarVisibility(true);
            //mContext.setProgress(newProgress * 100);
            //mContext.setSecondaryProgress(newProgress * 100);
            if (newProgress == 100) {
                mProgressbar.setVisibility(View.GONE);
            } else {
                mProgressbar.setVisibility(View.VISIBLE);
                mProgressbar.setProgress(newProgress);
            }
            super.onProgressChanged(view, newProgress);
        }

        @Override
        public void onReceivedTitle(WebView view, String title) {
            //mContext.setTitle(title);
            super.onReceivedTitle(view, title);
            if(mOnReceivedTitleListener!=null)
                mOnReceivedTitleListener.onReceivedTitle(title);
        }

        @Override
        public void onReceivedIcon(WebView view, Bitmap icon) {
            // TODO: Implement this method
            //mContext.setIcon(new BitmapDrawable(icon));
            super.onReceivedIcon(view, icon);
            if(mOnReceivedIconListener!=null)
                mOnReceivedIconListener.onReceivedIcon(icon);
        }

        @Override
        public Bitmap getDefaultVideoPoster() {
            return BitmapFactory.decodeResource(mContext.getResources(), R.drawable.icon);
        }

        // For Android 3.0+
        public void openFileChooser(ValueCallback<Uri> uploadMsg, String acceptType) {
            if (mUploadMessage != null) return;
            mUploadMessage = uploadMsg;
            openFile(mDir);
        }

        // For Android < 3.0
        public void openFileChooser(ValueCallback<Uri> uploadMsg) {
            openFileChooser(uploadMsg, "");
        }

        // For Android  > 4.1.1
        public void openFileChooser(ValueCallback<Uri> uploadMsg, String acceptType, String capture) {
            openFileChooser(uploadMsg, acceptType);
        }

    }

    private OnReceivedTitleListener mOnReceivedTitleListener;
    private OnReceivedIconListener mOnReceivedIconListener;

    public void setOnReceivedTitleListener(OnReceivedTitleListener listener){
        mOnReceivedTitleListener = listener;
    }

    public void setOnReceivedIconListener(OnReceivedIconListener listener){
        mOnReceivedIconListener = listener;
    }

    public interface OnReceivedTitleListener{
        public void onReceivedTitle(String string);
    }

    public interface OnReceivedIconListener{
        public void onReceivedIcon(Bitmap bitmap);
    }
}
