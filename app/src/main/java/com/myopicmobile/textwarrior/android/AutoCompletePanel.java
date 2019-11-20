package com.myopicmobile.textwarrior.android;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Filter;
import android.widget.Filterable;
import android.widget.ListPopupWindow;
import android.widget.TextView;

import com.myopicmobile.textwarrior.common.Flag;
import com.myopicmobile.textwarrior.common.Language;
import com.myopicmobile.textwarrior.common.LanguageNonProg;

import java.util.ArrayList;

public class AutoCompletePanel {

	private FreeScrollingTextField _textField;
	private Context _context;
	private static Language _globalLanguage = LanguageNonProg.getInstance();
	private ListPopupWindow _autoCompletePanel;
	private AutoCompletePanel.MyAdapter _adapter;
	private Filter _filter;

	private int _verticalOffset;

	private int _height;

	private int _horizontal;

	private CharSequence _constraint;

	private int _backgroundColor;

	private GradientDrawable gd;

	private int _textColor;

	public AutoCompletePanel(FreeScrollingTextField textField) {
		_textField = textField;
		_context = textField.getContext();
		initAutoCompletePanel();

	}

	public void setTextColor(int color){
		_textColor=color;
		gd.setStroke(1, color);
		_autoCompletePanel.setBackgroundDrawable(gd);
	}
	
	
	public void setBackgroundColor(int color){
		_backgroundColor=color;
		gd.setColor(color);
		_autoCompletePanel.setBackgroundDrawable(gd);
	}
	
	public void setBackground(Drawable color){
		_autoCompletePanel.setBackgroundDrawable(color);
	}
	
	private void initAutoCompletePanel() {
		_autoCompletePanel = new ListPopupWindow(_context);
		_autoCompletePanel.setAnchorView(_textField);
		_adapter = new MyAdapter(_context, android.R.layout.simple_list_item_1);
		_autoCompletePanel.setAdapter(_adapter);
		//_autoCompletePanel.setDropDownGravity(Gravity.BOTTOM | Gravity.LEFT);
		_filter = _adapter.getFilter();
		setHeight(300);

		TypedArray array = _context.getTheme().obtainStyledAttributes(new int[] {  
																		  android.R.attr.colorBackground, 
																		  android.R.attr.textColorPrimary, 
																	  }); 
		int backgroundColor = array.getColor(0, 0xFF00FF); 
		int textColor = array.getColor(1, 0xFF00FF); 
		array.recycle();
		gd=new GradientDrawable();
		gd.setColor(backgroundColor);
		gd.setCornerRadius(4);
		gd.setStroke(1, textColor);
		setTextColor(textColor);
		_autoCompletePanel.setBackgroundDrawable(gd);
		_autoCompletePanel.setOnItemClickListener(new OnItemClickListener(){

				@Override
				public void onItemClick(AdapterView<?> p1, View p2, int p3, long p4) {
					// TODO: Implement this method
					_textField.replaceText(_textField.getCaretPosition() - _constraint.length(), _constraint.length(), ((TextView)p2).getText().toString());
					_adapter.abort();
					dismiss();
				}
			});
			
	}

	public void setWidth(int width) {
		// TODO: Implement this method
		_autoCompletePanel.setWidth(width);
	}

	private void setHeight(int height) {
		// TODO: Implement this method

		if (_height != height) {
			_height = height;
			_autoCompletePanel.setHeight(height);
		}
	}

	private void setHorizontalOffset(int horizontal) {
		// TODO: Implement this method
		horizontal = Math.min(horizontal, _textField.getWidth() / 2);
		if (_horizontal != horizontal) {
			_horizontal = horizontal;
			_autoCompletePanel.setHorizontalOffset(horizontal);
		}
	}


	private void setVerticalOffset(int verticalOffset) {
		// TODO: Implement this method
		//verticalOffset=Math.min(verticalOffset,_textField.getWidth()/2);
		int max=0 - _autoCompletePanel.getHeight();
		if (verticalOffset > max) {
			_textField.scrollBy(0, verticalOffset - max);
			verticalOffset = max;
		}
		if (_verticalOffset != verticalOffset) {
			_verticalOffset = verticalOffset;
			_autoCompletePanel.setVerticalOffset(verticalOffset);
		}
	}

	public void update(CharSequence constraint) {
		_adapter.restart();
		_filter.filter(constraint);
	}

	public void show() {
		if (!_autoCompletePanel.isShowing())
			_autoCompletePanel.show();
		_autoCompletePanel.getListView().setFadingEdgeLength(0);
	}

	public void dismiss() {
		if (_autoCompletePanel.isShowing()) {
			_autoCompletePanel.dismiss();
		}
	}
	synchronized public void setLanguage(Language lang) {
		_globalLanguage = lang;
	}

	synchronized public static Language getLanguage() {
		return _globalLanguage;
	}

	/**
	 * Adapter定义
	 */
	class MyAdapter extends ArrayAdapter<String> implements Filterable {

		private int _h;
		private Flag _abort;

		private DisplayMetrics dm;

		public MyAdapter(android.content.Context context, int resource) {
			super(context, resource);
			_abort = new Flag();
			setNotifyOnChange(false);
			dm=context.getResources().getDisplayMetrics();
			
		}

		public void abort() {
			_abort.set();
		}

		
		private int dp(float n) {
			// TODO: Implement this method
			return (int)TypedValue.applyDimension(1,n,dm);
		}
		
		@Override
		public View getView(int position, View convertView, ViewGroup parent) {
			// TODO: Implement this method
			TextView view=(TextView) super.getView(position, convertView, parent);
			/*TextView view=null;
			if(convertView==null){
				 view=new TextView(_context);
				 view.setTextSize(16);
				 view.setPadding(dp(8),dp(3),dp(8),dp(3));
			}
			else{
				view=(TextView) convertView;
			}
			view.setText(getItem(position));*/
			view.setTextColor(_textColor);
			return view;
		}


		
		public void restart() {
			// TODO: Implement this method
			_abort.clear();
		}

		public int getItemHeight() {
			if (_h != 0)
				return _h;

			LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			TextView item = (TextView) inflater.inflate(android.R.layout.simple_list_item_1, null);
			item.measure(0, 0);
			_h = item.getMeasuredHeight();
			return _h;
		}
		/**
		 * 实现自动完成的过滤算法
		 */
		@Override
		public Filter getFilter() {
			Filter filter = new Filter() {

				/**
				 * 本方法在后台线程执行，定义过滤算法
				 */
				@Override
				protected FilterResults performFiltering(CharSequence constraint) {
					/*int l=constraint.length();
					 int i=l;
					 for(;i>0;i--){
					 if(constraint.charAt(l-1)=='.')
					 break;
					 }
					 if(i>0){
					 constraint=constraint.subSequence(i,l);
					 }*/

					// 此处实现过滤
					// 过滤后利用FilterResults将过滤结果返回
					ArrayList <String>buf = new ArrayList<String>();
					String keyword = String.valueOf(constraint).toLowerCase();
					String[] ss=keyword.split("\\.");
					if (ss.length == 2) {
						String pkg=ss[0];
						keyword = ss[1];
						if (_globalLanguage.isBasePackage(pkg)) {
							String[] keywords=_globalLanguage.getBasePackage(pkg);
							for (String k:keywords) {
								if (k.toLowerCase().startsWith(keyword))
									buf.add(k);
							}
						}
					}
					else if (ss.length == 1) {
						if (keyword.charAt(keyword.length() - 1) == '.') {
							String pkg=keyword.substring(0, keyword.length() - 1);
							keyword = "";
							if (_globalLanguage.isBasePackage(pkg)) {
								String[] keywords=_globalLanguage.getBasePackage(pkg);
								for (String k:keywords) {
									buf.add(k);
								}
							}
						}
						else {
							String[] keywords = _globalLanguage.getUserWord();
							for (String k:keywords) {
								if (k.toLowerCase().startsWith(keyword))
									buf.add(k);
							}
							keywords = _globalLanguage.getKeywords();
							for (String k:keywords) {
								if (k.indexOf(keyword) == 0)
									buf.add(k);
							}
							keywords = _globalLanguage.getNames();
							for (String k:keywords) {
								if (k.toLowerCase().startsWith(keyword))
									buf.add(k);
							}
						}
					}
					_constraint = keyword;
					FilterResults filterResults = new FilterResults();
					filterResults.values = buf;   // results是上面的过滤结果
					filterResults.count = buf.size();  // 结果数量
					return filterResults;
				}
				/**
				 * 本方法在UI线程执行，用于更新自动完成列表
				 */
				@Override
				protected void publishResults(CharSequence constraint, FilterResults results) {
					if (results != null && results.count > 0 && !_abort.isSet()) {
						// 有过滤结果，显示自动完成列表
						MyAdapter.this.clear();   // 清空旧列表
						MyAdapter.this.addAll((ArrayList<String>)results.values);
						//int y = _textField.getPaintBaseline(_textField.getCaretRow()) - _textField.getScrollY();
						int y = _textField.getCaretY() + _textField.rowHeight() / 2 - _textField.getScrollY();
						setHeight(getItemHeight() * Math.min(2, results.count));
						//setHeight((int)(Math.min(_textField.getContentHeight()*0.4,getItemHeight() * Math.min(6, results.count))));
						
						setHorizontalOffset(_textField.getCaretX() - _textField.getScrollX());
						setVerticalOffset(y - _textField.getHeight());//_textField.getCaretY()-_textField.getScrollY()-_textField.getHeight());
						notifyDataSetChanged();
						show();
					}
					else {
						// 无过滤结果，关闭列表
						notifyDataSetInvalidated();
					}
				}

			};
			return filter;
		}
	}
}
