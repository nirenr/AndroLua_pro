package android.app;

import android.content.*;
import android.graphics.drawable.*;
import android.view.*;
import android.widget.*;
import java.util.*;

public class AlertDialogBuilder extends AlertDialog
{
	
	private Context mContext;

	private ListView mListView;

	private String mMessage;

	private String mTitle;

	private View mView;

	public AlertDialogBuilder(Context context)
	{
		super(context);
		mContext = context;
		mListView = new ListView(mContext);

	}

	public AlertDialogBuilder(Context context, int theme)
	{
		super(context,theme);
		mContext = context;
		mListView = new ListView(mContext);
	}

	public void setPositiveButton(CharSequence text, DialogInterface.OnClickListener listener)
	{
		setButton(DialogInterface.BUTTON_POSITIVE, text, listener);
	}

	public void setNegativeButton(CharSequence text, DialogInterface.OnClickListener listener)
	{
		setButton(DialogInterface.BUTTON_NEGATIVE, text, listener);
	}

	public void setNeutralButton(CharSequence text, DialogInterface.OnClickListener listener)
	{
		setButton(DialogInterface.BUTTON_NEUTRAL, text, listener);
	}
	
	@Override
	public void setTitle(CharSequence title)
	{
		// TODO: Implement this method
		mTitle=title.toString();
		super.setTitle(title);
	}

	public String getTitle()
	{
		return mTitle;
	}
	
	@Override
	public void setMessage(CharSequence message)
	{
		// TODO: Implement this method
		mMessage=message.toString();
		super.setMessage(message);
	}

	public String getMessage()
	{
		return mMessage;
	}
	
	@Override
	public void setIcon(Drawable icon)
	{
		// TODO: Implement this method
		super.setIcon(icon);
	}
	
	@Override
	public void setView(View view)
	{
		// TODO: Implement this method
		mView=view;
		super.setView(view);
	}

	public View getView()
	{
		return mView;
	}

	public void setItems(String[] items)
	{
		ArrayList<String> alist=new ArrayList<String>(Arrays.asList(items));
		ArrayListAdapter adp=new ArrayListAdapter<String>(mContext, android.R.layout.simple_list_item_1, alist);
		setAdapter(adp);
	}

	public void setAdapter(ListAdapter adp)
	{
		if(!mListView.equals(mView))
			setView(mListView);
		mListView.setAdapter(adp);
	}



	public ListView getListView()
	{
		return mListView;
	}
	
	@Override
	public void setOnCancelListener(DialogInterface.OnCancelListener listener)
	{
		// TODO: Implement this method
		super.setOnCancelListener(listener);
	}
	
	@Override
	public void setOnDismissListener(DialogInterface.OnDismissListener listener)
	{
		// TODO: Implement this method
		super.setOnDismissListener(listener);
	}
	
	@Override
	public void show()
	{
		super.show();
	}
	
	@Override
	public void hide()
	{
		super.hide();
	}
	
	
	public void close()
	{
		super.dismiss();
	}
	
	@Override
	public boolean isShowing()
	{
		return super.isShowing();
	}
	
	
	@Override
	public void dismiss()
	{
		// TODO: Implement this method
		super.hide();
	}

}
