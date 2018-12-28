package com.androlua;

import android.app.AlertDialog;
import android.app.DownloadManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Bundle;
import android.widget.EditText;

import java.io.File;
import java.util.HashMap;

/**
 * Created by Administrator on 2017/02/14 0014.
 */

public class Download {
    //private final Context mContext;
    private final LuaContext mContext;
    private EditText file_input_field;
    private String mUrl;
    private DownloadBroadcastReceiver mDownloadBroadcastReceiver;
    private HashMap<Long, String[]> mDownload = new HashMap<Long, String[]>();
    private OnDownloadCompleteListener mOnDownloadCompleteListener;
    private String mMessage;

    public String getDestinationDir() {
        return mDestinationDir;
    }

    public void setDestinationDir(String destinationDir) {
        mDestinationDir = destinationDir;
    }

    private String mDestinationDir;
    private String mUserAgent;
    private String mMimeType;
    private long mContentLength;
    private String mFilename;

    public Download(LuaContext context) {
        mContext = context;
    }

    public String getUserAgent() {
        return mUserAgent;
    }

    public void setUserAgent(String userAgent) {
        mUserAgent = userAgent;
    }

    public String getUrl() {
        return mUrl;
    }

    public void setUrl(String url) {
        mUrl = url;
    }

    public String getMimeType() {
        return mMimeType;
    }

    public void setMimeType(String mimeType) {
        mMimeType = mimeType;
    }

    public long getContentLength() {
        return mContentLength;
    }

    public void setContentLength(long contentLength) {
        mContentLength = contentLength;
    }

    public String getFilePath() {
        return mFilename;
    }

    public void setFilePath(String filename) {
        mFilename = filename;
    }

    public void setMessage(String message) {
        mMessage = message;
    }

    public void setOnDownloadCompleteListener(OnDownloadCompleteListener listener) {
        mOnDownloadCompleteListener = listener;
    }

    public void start() {

    }

    public void start(String url, String destinationDir, String filename, String message) {
        // TODO: Implement this method
        mUrl = url;
        mMessage = message;
        Uri uri = Uri.parse(mUrl);
        mFilename = filename;
        if (filename == null)
            mFilename = uri.getLastPathSegment();
        if (destinationDir == null)
            mDestinationDir = "Download";

        file_input_field = new EditText(mContext.getContext());
        file_input_field.setText(mFilename);
        if (mMessage == null) {
            mMessage = url;
        }
        new AlertDialog.Builder(mContext.getContext())
                .setTitle("Download")
                .setMessage(mMessage)
                .setView(file_input_field)
                .setPositiveButton("Download", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface p1, int p2) {
                        // TODO: Implement this method
                        mFilename = file_input_field.getText().toString();
                        start(false);
                    }
                })
                .setNegativeButton("Cancel", null)
                .setNeutralButton("Only Wifi", new DialogInterface.OnClickListener() {

                    @Override
                    public void onClick(DialogInterface p1, int p2) {
                        // TODO: Implement this method
                        mFilename = file_input_field.getText().toString();
                        start(true);
                    }
                })
                .create()
                .show();
    }

    public long start(boolean isWifi) {
        if (mDownloadBroadcastReceiver == null) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(DownloadManager.ACTION_DOWNLOAD_COMPLETE);
            mDownloadBroadcastReceiver = new DownloadBroadcastReceiver();
            mContext.getContext().registerReceiver(mDownloadBroadcastReceiver, filter);
        }

        DownloadManager downloadManager = (DownloadManager) mContext.getContext().getSystemService(Context.DOWNLOAD_SERVICE);

        Uri uri = Uri.parse(mUrl);
        uri.getLastPathSegment();
        DownloadManager.Request request = new DownloadManager.Request(uri);

        if (mDestinationDir == null)
            mDestinationDir = "Download";
        request.setDestinationInExternalPublicDir(mDestinationDir, mFilename);

        request.setTitle(mFilename);

        request.setDescription(mUrl);

        //request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED);
        if (isWifi)
            request.setAllowedNetworkTypes(DownloadManager.Request.NETWORK_WIFI);

        //request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_HIDDEN);
        if (mMimeType == null)
            mMimeType = "*/*";

        request.setMimeType(mMimeType);
        //Environment.getExternalStoragePublicDirectory(dirType)
        long downloadId = downloadManager.enqueue(request);
        mDownload.put(downloadId, new String[]{new File(mDestinationDir, mFilename).getAbsolutePath(), mMimeType});
        return downloadId;
    }

    public interface OnDownloadCompleteListener {
        public abstract void onDownloadComplete(String fileName, String mimetype);
    }

    private class DownloadBroadcastReceiver extends BroadcastReceiver {


        @Override
        public void onReceive(Context p1, Intent p2) {
            // TODO: Implement this method
            //id=p2.getLongExtra("flg", 0);
            //int id=p2.getFlags();
            long id = p2.getLongExtra(DownloadManager.EXTRA_DOWNLOAD_ID, 0);
            Bundle bundle = p2.getExtras();
            //bundle.g
            if (mDownload.containsKey(id) && mOnDownloadCompleteListener != null) {
                String[] data = mDownload.get(id);
                mOnDownloadCompleteListener.onDownloadComplete(data[0], data[1]);
            }
        }
    }

}
