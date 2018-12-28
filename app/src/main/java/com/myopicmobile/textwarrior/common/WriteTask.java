package com.myopicmobile.textwarrior.common;

import android.app.ProgressDialog;
import android.os.AsyncTask;

import com.androlua.LuaEditor;

import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;

/**
 * Created by Administrator on 2018/07/18 0018.
 */

public class WriteTask extends AsyncTask
{
    private ProgressDialog _dlg;


    public int getMin()
    {//new Future;
        // TODO: Implement this method
        return 0;
    }


    public int getMax()
    {
        // TODO: Implement this method
        return (int)_len;
    }


    public int getCurrent()
    {
        // TODO: Implement this method
        return _total;
    }


    final protected Document _buf;
    private static int _total = 0;
    private LuaEditor _edit;

    private File _file;

    private long _len;

    public WriteTask(LuaEditor edit,String fileName){
        this(edit,new File(fileName));
    }

    public WriteTask(LuaEditor edit,File file){
        _file=file;
        _len=_file.length();
        _edit=edit;
        _buf=new Document(edit);
        _dlg=new ProgressDialog(edit.getContext());
        _dlg.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        _dlg.setTitle("正在保存");
        _dlg.setIcon(android.R.drawable.ic_dialog_info);
        _dlg.setMax((int)_len);
    }

    public void start()
    {
        // TODO: Implement this method
        execute();
        _dlg.show();
    }
    @Override
    protected Object doInBackground(Object[] p1)
    {
        // TODO: Implement this method
        try
        {

            BufferedWriter fi = new BufferedWriter( new OutputStreamWriter(new BufferedOutputStream(new FileOutputStream(_file))));
            fi.write(_edit.getText().toString());
            return true;
        }
        catch (Exception e)
        {
            _dlg.setMessage(e.getMessage());
        }
        return "";
    }

    @Override
    protected void onPostExecute(Object result)
    {
        // TODO: Implement this method
        super.onPostExecute(result);
         _dlg.dismiss();
    }

    @Override
    protected void onProgressUpdate(Object[] values)
    {
        // TODO: Implement this method
        _dlg.setProgress(_total);
        super.onProgressUpdate(values);
    }


}
