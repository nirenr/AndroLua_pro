package com.nirenr;

import android.app.AlertDialog;
import android.app.Service;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Build;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayListAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.GridView;
import android.widget.LinearLayout;

import com.androlua.LuaEditor;

import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Created by Administrator on 2018/07/19 0019.
 */

public class SplitEditView extends LinearLayout implements View.OnClickListener, AdapterView.OnItemClickListener {
    private static final int ALL = 0;
    private static final int CHUNK = 1;
    private static final int LINE = 2;
    private static final int ROW = 3;
    private static final int CHAR = 4;
    private static final int SAVE = 5;
    private final Context mContext;
    private LinearLayout mRoot;
    private GridView mListView;
    private LuaEditor mEditView;
    private int mSplitMode=ALL;
    private String mText = "";
    private String[] mList=new String[]{""};
    private OnSaveListener mOnSaveListener;
    private LinearLayout mButtonBar;

    public SplitEditView(Context context) {
        super(context);
        initView(context);
        mContext = context;
    }

    private void initView(Context context) {
        mRoot = this;
        mRoot.setOrientation(LinearLayout.VERTICAL);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT, 1);
        LinearLayout.LayoutParams lp2 = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT, 1);
        LinearLayout.LayoutParams lp3 = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        mListView = new GridView(context);
        mEditView = new LuaEditor(context);
        mListView.setOnItemClickListener(this);
        mRoot.addView(mListView, lp);
        mRoot.addView(mEditView, lp);
        mButtonBar = new LinearLayout(context);
        String[] bts = new String[]{"全文", "按段", "按行", "按句", "按字", "确定"};

        for (int i = 0; i < bts.length; i++) {
            String b = bts[i];
            Button btn = new Button(context);
            btn.setText(b);
            btn.setId(i);
            btn.setOnClickListener(this);
            mButtonBar.addView(btn, lp2);
        }

        mRoot.addView(mButtonBar, lp3);
        mListView.setVisibility(View.GONE);
        mEditView.setVisibility(View.VISIBLE);
        setText("");
        setOnSaveListener(null);
    }

    public void setText(String text) {
        mText = text;
        if (mText == null)
            mText = "";
        mEditView.setText(mText);
        initText();
    }

    public String getText() {
        if(isShowEdit())
            return mEditView.getText().toString();
        StringBuilder buf = new StringBuilder();
        for (String s : mList){
             buf.append(s);
            if(mSplitMode==CHUNK)
                buf.append("\n\n");
        }
        if(mSplitMode==CHUNK)
            buf.delete(buf.length()-2,buf.length());
        return buf.toString();
    }

    private void initText() {
        mList = new String[]{mText};
        switch (mSplitMode) {
            case CHUNK:
                splitChunk();
                break;
            case LINE:
                splitLine();
                break;
            case ROW:
                splitRow();
                break;
            case CHAR:
                splitChar();
                break;
            default:
                setShowEdit(true);
        }
    }

    private void setShowEdit(boolean show) {
        if (isShowEdit() == show)
            return;
        if (!show) {
            mListView.setVisibility(View.VISIBLE);
            mEditView.setVisibility(View.GONE);
            mText = mEditView.getText().toString();
            mList=new String[]{mText};
        } else {
            mEditView.setText(getText());
            mListView.setVisibility(View.GONE);
            mEditView.setVisibility(View.VISIBLE);
        }
    }

    public boolean isShowEdit() {
        return mEditView.getVisibility() == View.VISIBLE;
    }

    @Override
    public void onClick(View v) {
        Button b = (Button) v;
        switch (v.getId()) {
            case ALL:
                setShowEdit(true);
                mSplitMode = ALL;
                break;
            case CHUNK:
                setShowEdit(false);
                splitChunk();
                break;
            case LINE:
                setShowEdit(false);
                splitLine();
                break;
            case ROW:
                setShowEdit(false);
                splitRow();
                break;
            case CHAR:
                setShowEdit(false);
                splitChar();
                break;
            case SAVE:
                if(mOnSaveListener!=null)
                    mOnSaveListener.onSave(getText());
                break;
        }
    }

    private String[] split(String text, String reg) {
        ArrayList<String> list = new ArrayList<>();
        Pattern pattern = Pattern.compile(reg);
        Matcher matcher = pattern.matcher(text);
        int start = 0;
        while (matcher.find()) {
            int end = matcher.end();
            if(mSplitMode==CHUNK)
                end=matcher.start();
            list.add(text.substring(start, end));
            start = end;
            if(mSplitMode==CHUNK)
                start = matcher.end();
        }
        if (start != text.length())
            list.add(text.substring(start));
        String[] ret = new String[list.size()];
        list.toArray(ret);
        return ret;
    }

    private void splitChar() {
        //mDlg.setTitle(mService.getString(R.string.split_edit_title) + " 按字");
        mText=getText();
        mSplitMode = CHAR;
        mList = new String[mText.length()];
        for (int i = 0; i < mText.length(); i++)
            mList[i] = String.valueOf(mText.charAt(i));
        if (mList.length == 0)
            mList = new String[]{""};
        mListView.setNumColumns(8);
        mListView.setAdapter(new ArrayListAdapter<>(mContext, android.R.layout.simple_list_item_1, mList));
    }

    private void splitLine() {
        //mDlg.setTitle(mService.getString(R.string.split_edit_title) + " 按段");
        mText=getText();
        mSplitMode = LINE;
        mList = split(mText, "\n");
        if (mList.length == 0)
            mList = new String[]{""};
        mListView.setNumColumns(1);
        mListView.setAdapter(new ArrayListAdapter<>(mContext, android.R.layout.simple_list_item_1, mList));
    }

    private void splitRow() {
        //mDlg.setTitle(mService.getString(R.string.split_edit_title) + " 按句");
        mText=getText();
        mSplitMode = ROW;
        //mList = mText.split("[。？！，\n “”]+");
        mList = split(mText, "\\. |[。？！，\n “”,：；;\\?!]+");
        if (mList.length == 0)
            mList = new String[]{""};
        mListView.setNumColumns(1);
        mListView.setAdapter(new ArrayListAdapter<>(mContext, android.R.layout.simple_list_item_1, mList));
    }

    private void splitChunk() {
        //mDlg.setTitle(mService.getString(R.string.split_edit_title) + " 按句");
        mText=getText();
        mSplitMode = CHUNK;
        //mList = mText.split("[。？！，\n “”]+");
        mList = split(mText, "\\n{2,10}");
        if (mList.length == 0)
            mList = new String[]{""};
        mListView.setNumColumns(1);
        mListView.setAdapter(new ArrayListAdapter<>(mContext, android.R.layout.simple_list_item_1, mList));
    }

    private void updateSplit() {
        switch (mSplitMode) {
            case CHUNK:
                splitChunk();
                break;
            case LINE:
                splitLine();
                break;
            case ROW:
                splitRow();
                break;
            case CHAR:
                splitChar();
                break;
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        new EditDialog(position).show();
    }

    public void setOnSaveListener(OnSaveListener listener){
        mOnSaveListener=listener;
        if(listener==null)
            mButtonBar.getChildAt(SAVE).setVisibility(GONE);
        else
            mButtonBar.getChildAt(SAVE).setVisibility(VISIBLE);
    }
    public static interface OnSaveListener{
        public void onSave(String text);
    }

    private class EditDialog implements DialogInterface.OnClickListener {
        private final int mIdx;
        private final EditText mEdit;
        private AlertDialog dlg;

        public EditDialog(int idx) {
            mIdx = idx;
            mEdit = new EditText(mContext);
            mEdit.setText(mList[idx]);
            mEdit.setSelection(mList[idx].length());
        }

        public void show() {
            dlg = new AlertDialog.Builder(mContext)
                    .setTitle("输入内容")
                    .setView(mEdit)
                    .setNegativeButton(android.R.string.cancel, null)
                    .setPositiveButton(android.R.string.ok, this)
                    .setCancelable(false)
                    .create();

            Window window = dlg.getWindow();
            if (window != null) {
                window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
                if(mContext instanceof Service){
                    if (Build.VERSION.SDK_INT >= 22)
                        window.setType(WindowManager.LayoutParams.TYPE_ACCESSIBILITY_OVERLAY);
                    else
                        window.setType(WindowManager.LayoutParams.TYPE_SYSTEM_ERROR);
                }
                dlg.show();
            }
            mEdit.setFocusable(true);
            mEdit.requestFocus();
        }

        @Override
        public void onClick(DialogInterface dialog, int which) {
            mList[mIdx] = mEdit.getText().toString();
            updateSplit();
            mListView.smoothScrollToPosition(mIdx);
        }
    }

}
