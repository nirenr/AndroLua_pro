package com.myopicmobile.textwarrior.common;
import java.io.*;
import android.os.*;
import com.androlua.*;
import com.myopicmobile.textwarrior.android.*;
import android.app.*;
import java.util.concurrent.*;

public class ReadTask extends AsyncTask
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
	
	public ReadTask(LuaEditor edit,String fileName){
		this(edit,new File(fileName));
	}
	
	public ReadTask(LuaEditor edit,File file){
		_file=file;
		_len=_file.length();
		_edit=edit;
		_buf=new Document(edit);
		_dlg=new ProgressDialog(edit.getContext());
		_dlg.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
		_dlg.setTitle("正在打开");
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
			FileInputStream fi = new FileInputStream(_file);
			byte[] buf=readAll(fi);
			return new String(buf);
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
		_edit.setText((String)result);
		_dlg.dismiss();
	}

	@Override
	protected void onProgressUpdate(Object[] values)
	{
		// TODO: Implement this method
		_dlg.setProgress(_total);
		super.onProgressUpdate(values);
	}
	
	
	
	private byte[] readAll(InputStream input) throws IOException 
	{
		ByteArrayOutputStream output = new ByteArrayOutputStream(4096);
		byte[] buffer = new byte[4096];
		int n = 0;
		_total=0;
		while (-1 != (n = input.read(buffer)))
		{
			output.write(buffer, 0, n);
			_total+=n;
			publishProgress();
		}
		byte[] ret= output.toByteArray();
		output.close();
		return ret;
	}

}
