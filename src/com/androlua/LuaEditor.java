package com.androlua;

import android.content.*;
import android.graphics.*;
import android.graphics.drawable.*;
import android.text.*;
import android.text.style.*;
import android.view.*;
import android.widget.*;
import java.io.*;
import java.util.*;
import java.util.regex.*;

public class LuaEditor extends EditText
{

	private Editable spannable;
	private int blockStart;
	private int blockEnd;
	private String blockText;
	private boolean isOnLayout;
	private int maxChars=1000000;

	private HashMap<String,Integer> highlightMap = new HashMap<String,Integer>();

	private final int commentColor = Color.rgb(00, 128, 00);
	private final int functionColor   = Color.rgb(96, 96, 255);
	private final int keywordColor = Color.rgb(255, 00, 255);
	private final int literalColor = Color.rgb(128, 160, 255);
	private final int stringColor = Color.rgb(255, 96, 128);
	private final int operatorColor = Color.rgb(255, 0, 0);

	//private final String commentTarget = "\\-\\-\\[(=*)\\[.*\\]\\1\\]|\\-\\-[^\\[][^\\n]*";
	private final String commentTarget = "\\-\\-[^\\[\\n][^\\n]*";
	private final String functionTarget   = "_ENV|_G|_VERSION|assert|collectgarbage|coroutine.create|coroutine.isyieldable|coroutine.resume|coroutine.running|coroutine.status|coroutine.wrap|coroutine.yield|debug.debug|debug.gethook|debug.getinfo|debug.getlocal|debug.getmetatable|debug.getregistry|debug.getupvalue|debug.getuservalue|debug.sethook|debug.setlocal|debug.setmetatable|debug.setupvalue|debug.setuservalue|debug.traceback|debug.upvalueid|debug.upvaluejoin|dofile|error|getfenv|getmetatable|io.close|io.flush|io.input|io.lines|io.open|io.output|io.popen|io.read|io.stderr|io.stdin|io.stdout|io.tmpfile|io.type|io.write|ipairs|load|loadfile|loadstring|luajava.bindClass|luajava.clear|luajava.coding|luajava.createArray|luajava.createProxy|luajava.instanceof|luajava.loadLib|luajava.loaded|luajava.luapath|luajava.new|luajava.newInstance|luajava.package|math.abs|math.acos|math.asin|math.atan|math.atan2|math.ceil|math.cos|math.cosh|math.deg|math.exp|math.floor|math.fmod|math.frexp|math.huge|math.ldexp|math.log|math.log10|math.max|math.maxinteger|math.min|math.mininteger|math.modf|math.pi|math.pow|math.rad|math.random|math.randomseed|math.sin|math.sinh|math.sqrt|math.tan|math.tanh|math.tointeger|math.type|math.ult|module|next|os.clock|os.date|os.difftime|os.execute|os.exit|os.getenv|os.remove|os.rename|os.setlocale|os.time|os.tmpname|package.config|package.cpath|package.loaded|package.loaders|package.loadlib|package.path|package.preload|package.searchers|package.searchpath|package.seeall|pairs|pcall|print|rawequal|rawget|rawlen|rawset|require|select|setfenv|setmetatable|string.byte|string.char|string.dump|string.find|string.format|string.gfind|string.gmatch|string.gsub|string.len|string.lower|string.match|string.pack|string.packsize|string.rep|string.reverse|string.sub|string.unpack|string.upper|table.concat|table.foreach|table.foreachi|table.insert|table.maxn|table.move|table.pack|table.remove|table.sort|table.unpack|tonumber|tostring|type|unpack|utf8.char|utf8.charpattern|utf8.codepoint|utf8.codes|utf8.len|utf8.offset|xpcall";
	private final String keywordTarget = "and|break|do|else|elseif|end|false|for|function|goto|if|in|local|nil|not|or|repeat|return|then|true|until|while";
	private final String literalTarget = "\\b([-+]?0x\\p{XDigit}+\\.?\\p{XDigit}*[pP]?[-+]?\\p{Digit}*|[-+]?\\p{Digit}+\\.?\\p{Digit}*[eE]?[-+]?\\p{Digit}*)\\b"; 
	private final String stringTarget = "(\".*?\"|\'.*?\')";
	private final String operatorTarget = "(\\(|\\)|\\{|\\}|!|\\+|-|\\*|/|%|\\^|#|==|~=|<=|>=|<|>|=|;|:|\\.\\.\\.|\\.\\.|\\.|\\[|\\])";

	private Paint mPaint = new Paint();

	private Scroller mScroller;
	private int mTouchSlop;
	private int mMinimumVelocity;
	private int mMaximumVelocity;
	private VelocityTracker mVelocityTracker;
	private float mLastMotionY;
	private float mLastMotionX;

	private int maxWidth;
	private int flingOrientation =0;
	private int VERTICAL = 1;
	private int HORIZONTAL = -1;
	private int FLINGED = 0;

	private int mLastScrollX;
	private int mLastScrollY;

	private List<EditHistory> undoHistories; 
	private List<EditHistory> redoHistories; 
	private boolean activate = false; 
	private boolean clear = true; 

	private int lastLineCount;

	private PopupWindow pop;
	private LuaListView list; 


	private ArrayList <String> keywords=new ArrayList<String>();
	private HashMap <String,String[]> packages = new HashMap<String,String[]>();


	private String globalTarget="_ENV|_G|assert|collectgarbage|dofile|double|error|getfenv|getmetatable|ipairs|load|loadfile|loadstring|module|next|pairs|pcall|print|rawequal|rawget|rawlen|rawset|require|select|setfenv|setmetatable|tonumber|tostring|type|unpack|xpcall";

	private String package_coroutine = "create|isyieldable|resume|running|status|wrap|yield";
	private String package_debug = "debug|gethook|getinfo|getlocal|getmetatable|getregistry|getupvalue|getuservalue|sethook|setlocal|setmetatable|setupvalue|setuservalue|traceback|upvalueid|upvaluejoin";
	private String package_io = "close|flush|input|lines|open|output|popen|read|stderr|stdin|stdout|tmpfile|type|write";
	private String package_luajava = "astable|bindClass|clear|coding|createArray|createProxy|instanceof|loadLib|loaded|luapath|new|newInstance|package|tostring";
	private String package_math = "abs|acos|asin|atan|atan2|ceil|cos|cosh|deg|exp|floor|fmod|frexp|huge|ldexp|log|log10|max|maxinteger|min|mininteger|modf|pi|pow|rad|random|randomseed|sin|sinh|sqrt|tan|tanh|tointeger|type|ult";
	private String package_os = "clock|date|difftime|execute|exit|getenv|remove|rename|setlocale|time|tmpname";
	private String package_package = "config|cpath|loaded|loaders|loadlib|path|preload|searchers|searchpath|seeall";
	private String package_string = "byte|char|dump|find|format|gfind|gmatch|gsub|len|lower|match|pack|packsize|rep|reverse|sub|unpack|upper";
	private String package_table = "concat|foreach|foreachi|insert|maxn|move|pack|remove|sort|unpack";
	private String package_utf8 = "char|charpattern|codepoint|codes|len|offset";
	private String extFunctionTarget="activity|call|dump|each|enum|import|loadbitmap|loadlayout|new_env|set|task|thread";

	private int subIndex;

	private int mAbsTop;

	private boolean touchOut;

	private PopupWindow ps_pop;


	public LuaEditor(Context context)
	{
		super(context);

		setHorizontallyScrolling(true);
		setGravity(Gravity.LEFT | Gravity.TOP);
		//setBackgroundColor(Color.argb(0, 0, 0, 0));
		setHint("LuaEditor by nirenr");
		//setScrollBarStyle(View.SCROLLBARS_INSIDE_OVERLAY);
		if (getCurrentTextColor() < Color.GRAY)
			setBackgroundColor(Color.WHITE);
		else
			setBackgroundColor(Color.BLACK);

		setLineColor(Color.GRAY);

		initScroll(context);
		initUndo();
		initHighlight();
		initAutoInput(context);

	}

	//控制控件在显示时的初始状态
	@Override
	protected void onLayout(boolean changed, int left, int top, int right, int bottom)
	{
		// TODO: Implement this method
		super.onLayout(changed, left, top, right, bottom);
		if (!isOnLayout)
		{
			spannable = getText();
			isOnLayout = true;
			refreshHightlight();
			mPaint.setTextSize(getPaint().getTextSize() / 3 * 2);
			Rect rect=new Rect();
			getWindowVisibleDisplayFrame(rect);
			mAbsTop = rect.top + rect.height() - getHeight();
			pop.setWidth(getWidth() / 2);
			setPadding((int)mPaint.measureText(String.valueOf(getLineCount()) + "  "), 0, getLineHeight() / 3, 0);
			//setHeight(getLineHeight()*getLineCount() +bottom);
		}
	}
	/*
	 @Override
	 protected void onMeasure (int widthMeasureSpec, int heightMeasureSpec)
	 {
	 // TODO: Implement this method
	 super.onMeasure(widthMeasureSpec, heightMeasureSpec);
	 int wm=MeasureSpec.makeMeasureSpec(getMeasuredWidth(), 0);
	 int hm=MeasureSpec.makeMeasureSpec(getMeasuredHeight() + getHeight() / 2, 0);
	 setMeasuredDimension(wm, hm);
	 }
	 */

	//设置行号与行线颜色
	public void setLineColor(int color)
	{
		mPaint.setColor(color);
		invalidate();
	}



	//重写onDraw绘制行号
	@Override
	protected void onDraw(Canvas canvas)
	{

		int lineHeight = getLineHeight(); 
		int leftPadding = getPaddingLeft();
		int x=getScrollX();
		int r=x + getRight() - 4;
		int t=getScrollY();
		int b=getHeight() + t;

		Layout layout=getLayout();
		int SelectionLine=layout.getLineForOffset(getSelectionEnd());
		//int mSelectionLineBottom=SelectionLine * lineHeight + lineHeight;
		int selectionLineBottom = layout.getLineBottom(SelectionLine);

		canvas.drawLine(x , selectionLineBottom , r , selectionLineBottom  , mPaint);
		canvas.drawLine(x + leftPadding - 4, t, x + leftPadding - 4, b, mPaint);
		/*
		 int bh=getHeight() * getHeight() / getMeasuredHeight();
		 int bt=t+(getHeight() - bh) * t / getMeasuredHeight();
		 Log.d("","t"+t+";bt"+bt+";bh"+bh);
		 canvas.drawRect(r - getPaddingRight(), bt, x + getRight(), bt+bh, mPaint);
		 */
		int topLine=layout.getLineForVertical(t);
		int bottomLine=layout.getLineForVertical(b);
		for (int i=topLine; i <= bottomLine + 1; i++)
		{
			canvas.drawText(String.valueOf(i), x + 2, i * lineHeight, mPaint);
		}
		canvas.translate(0, 0);
		super.onDraw(canvas);
	}

	//重写onTextChanged实现代码高亮和动态提示
	@Override
	public void onTextChanged(CharSequence s, int start, int before, int count)
	{		
		spannable = getText();
		if (count > maxChars || !isOnLayout)
		{
			return;
		}

		if (count > 32)
		{
			refreshHightlight();
		}
		else 
		{
			//scrollTo(getScrollY() , mSelectionLineBottom - getLineHeight() * 2);
			refreshHightlight(start, start + count);
			updataAutoInput(start + count);
		}

	}

	private void initHighlight()
	{
		//format(literalColor, literalTarget);				
		//format(keywordColor, keywordTarget);				
		//format(operatorColor, operatorTarget);				
		format(functionColor, functionTarget);				
		//format(stringColor, stringTarget);				
		//format(commentColor, commentTarget);				

	}

	//添加高亮关键字
	private void format(int color, String s)
	{
		String[] list=s.split("\\|");
		for (String k:list)
			highlightMap.put(k, color);
	}


	class LuaListView extends ListView
	{

		private int h=0;

		public LuaListView(Context content)
		{
			super(content);
		}

		public int getItemHeight()
		{
			if (h != 0)
				return h;

			LayoutInflater inflater = (LayoutInflater) getContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			TextView item = (TextView) inflater.inflate(android.R.layout.simple_list_item_1, null);
			item.measure(0, 0);
			int h=item.getMeasuredHeight();
			return h;
		}

		@Override
		protected void onDraw(Canvas canvas)
		{	
			mPaint.setStyle(Paint.Style.STROKE);
			canvas.drawRoundRect(new RectF(1, 1, getWidth() - 1, getMeasuredHeight() - 1), 5, 5, mPaint);
			//canvas.drawRoundRect(3, 3, getWidth() - 2, getMeasuredHeight() - 2, 5, 5, p);
			canvas.translate(0, 0);
			super.onDraw(canvas);

		}
	};


	private void initAutoInput(Context context)
	{
		addKeyword(keywordTarget);
		addKeyword(globalTarget);

		addPackage("coroutine", package_coroutine);
		addPackage("debug", package_debug);
		addPackage("io", package_io);
		addPackage("luajava", package_luajava, literalColor);
		addPackage("math", package_math);
		addPackage("os", package_os);
		addPackage("package", package_package);
		addPackage("string", package_string);
		addPackage("table", package_table);
		addPackage("utf8", package_utf8);
		addKeyword(extFunctionTarget, literalColor);

		list = new LuaListView(context);

		if (getCurrentTextColor() < Color.GRAY)
			list.setBackgroundColor(Color.argb(0xff, 0xee, 0xee, 0xee));
		else
			list.setBackgroundColor(Color.argb(0xff, 0x44, 0x44, 0x44));

		list.setFadingEdgeLength(0);
		//list.setBackgroundResource(R.drawable.bg);
		list.setOnItemClickListener(new AdapterView.OnItemClickListener(){

				@Override
				public void onItemClick(AdapterView<?> p1, View p2, int p3, long p4)
				{
					spannable.insert(getSelectionEnd(), ((TextView)p2).getText().toString().substring(subIndex));
					pop.dismiss();
					// TODO: Implement this method
				}
			});

		pop = new PopupWindow(list);
		pop.setOutsideTouchable(true);
		//pop.setBackgroundDrawable(new BitmapDrawable());
	}

	//添加动态提示包
	public void addPackage(String p, String s)
	{
		if (!keywords.contains(p))
			keywords.add(p);
		String[] arr=s.split("\\|");
		packages.put(p, arr);

	}

	//添加动态提示关键字
	public void addKeyword(String s)
	{
		for (String k:s.split("\\|"))
			if (!keywords.contains(k))
				keywords.add(k);
	}

	//添加动态提示包并添加到高亮显示
	public void addPackage(String p, String s, int clr)
	{
		if (!keywords.contains(p))
			keywords.add(p);
		String[] arr=s.split("\\|");
		packages.put(p, arr);

		for (String f:arr)
			highlightMap.put(p + "." + f, clr);

	}

	//添加动态提示关键字并添加到高亮显示
	public void addKeyword(String s, int clr)
	{
		for (String k:s.split("\\|"))
		{
			if (!keywords.contains(k))
				keywords.add(k);
			highlightMap.put(k, clr);
		}
	}



	//刷新当前屏幕显示文字高亮
	public void refreshHightlight()
	{
		Layout layout = getLayout();
		int topLine = layout.getLineForVertical(getScrollY());
		int bottomLine = layout.getLineForVertical(getScrollY() + getHeight());

        blockStart = layout.getLineStart(topLine); 
		blockEnd = layout.getLineEnd(bottomLine); 
		if (blockEnd == 0)
			blockEnd = layout.getText().length();

		blockText = layout.getText().subSequence(blockStart, blockEnd).toString();

		if (blockStart >= blockEnd)
			return;

		clear(0, spannable.length());
		highlight();
		highlight(blockText, blockStart, blockEnd);

	}

	//刷新一段文字的高亮
	public void refreshHightlight(int start, int end)
	{
		Layout layout = getLayout();
		int topLine = layout.getLineForOffset(start);
		int bottomLine = layout.getLineForOffset(end);

        blockStart = layout.getLineStart(topLine); 
		blockEnd = layout.getLineEnd(bottomLine); 
		if (blockEnd == 0)
			blockEnd = layout.getText().length();

		blockText = layout.getText().subSequence(blockStart, blockEnd).toString();

		if (blockStart >= blockEnd)
			return;

		clear(blockStart, blockEnd);
		highlight();
		highlight(blockText, blockStart, blockEnd);
	}

	//高亮字符串，注释等内容
	private void highlight(String text , int start , int end)
	{ 
        CharSeqReader reader = new CharSeqReader(text);

        SpannableStringBuilder builder = (SpannableStringBuilder) spannable;
        LuaLexer lexer = new LuaLexer(reader);
        try
		{
            int idx = start;
            LuaTokenTypes lastType = null;
			int lss=-1;
		
			while (true)
			{
				LuaTokenTypes type = lexer.advance();
				if (type == null)
				{
					
					if (lss != -1)
					{
						int s=lss - start;
						String t=text.substring(s, text.length() - 1);
						highlight(t, lss, idx );
						lss = -1;
					}
					break;
				}
				int len = lexer.yylength();


				if (isKeyword(type))
				{
					//关键字
					builder.setSpan(new ForegroundColorSpan(keywordColor), idx, idx + len, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				}
				/*else if (type == LuaTokenTypes.LPAREN || type == LuaTokenTypes.RPAREN
				 || type == LuaTokenTypes.LBRACK || type == LuaTokenTypes.RBRACK
				 || type == LuaTokenTypes.LCURLY || type == LuaTokenTypes.RCURLY)
				 {
				 //括号
				 //builder.setSpan(new ForegroundColorSpan(0xFF888888), idx, idx + len, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				 }*/
				else if (type == LuaTokenTypes.STRING)
				{
					//字符串
					builder.setSpan(new ForegroundColorSpan(stringColor), idx, idx + len, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				}
				else if (type == LuaTokenTypes.LONGSTRING_BEGIN || type == LuaTokenTypes.LONGCOMMENT_BEGIN)
				{
					lss = idx+len;
				}
				else if ((type == LuaTokenTypes.LONGSTRING_END || type == LuaTokenTypes.LONGCOMMENT_END)&& lss != -1)
				{
					//builder.setSpan(new ForegroundColorSpan(stringColor), idx, idx + len, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);

					int s=lss - start ;
					String t=text.substring(s, idx - start);
					highlight(t, lss, idx );
					lss = -1;
				}
				/*else if (type == LuaTokenTypes.NAME)
				 {
				 if (lastType == LuaTokenTypes.FUNCTION)
				 {
				 //函数名
				 builder.setSpan(new ForegroundColorSpan(0xff88a722), idx, idx + len, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				 }
				 else
				 {
				 //Log.d("lua", spannable.subSequence(idx, idx + len).toString());
				 //标识符
				 //builder.setSpan(new ForegroundColorSpan(0xFFFFFFFF), idx, idx + len, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				 }
				 }*/
				else if (type == LuaTokenTypes.SHORTCOMMENT)// || type == LuaTokenTypes.LONGCOMMENT)
				{
                    //注释
					builder.setSpan(new ForegroundColorSpan(commentColor), idx, idx + len, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				}
				else if (type == LuaTokenTypes.NUMBER)
				{
					builder.setSpan(new ForegroundColorSpan(0xFFc868a8), idx, idx + len, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				}

				/*if (type != LuaTokenTypes.WS
				 && type != LuaTokenTypes.NEWLINE
				 && type != LuaTokenTypes.NL_BEFORE_LONGSTRING)
				 {
				 lastType = type;
				 }*/
				lastType = type;
				idx += len;
			}
        }
		catch (IOException e)
		{
            e.printStackTrace();
        }
 	}

    private static boolean isKeyword(LuaTokenTypes t)
	{
        switch (t)
		{
			case TRUE:
			case FALSE:
            case DO:
            case FUNCTION:
            case NOT:
            case AND:
            case OR:
            case WITH:
            case IF:
            case THEN:
            case ELSEIF:
            case ELSE:
            case WHILE:
            case FOR:
            case IN:
            case RETURN:
            case BREAK:
            case CONTINUE:
            case LOCAL:
            case REPEAT:
            case UNTIL:
            case END:
            case NIL:
                return true;
            default:
                return false;
        }
    }


	//高亮包含在highlightMap的内容
	private void highlight()
	{ 
		CharacterStyle span=null; 
		Pattern p =Pattern.compile("([\\w\\.:_]+)"); 
		Matcher m =p.matcher(blockText);
		int clr=0;
		while (m.find()) 
		{ 
			Integer I=highlightMap.get(m.group());
			if (I != null)
			{
				clr = I.intValue();
				span = new ForegroundColorSpan(clr);
				spannable.setSpan(span, blockStart + m.start(), blockStart + m.end(), Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
			}
		} 
	}

	//清除高亮内容
	private void clear(int start, int end)
	{
		CharacterStyle[] spans= spannable.getSpans(start , end, CharacterStyle.class);
		for (int i=0;i < spans.length;i++)
			spannable.removeSpan(spans[i]);

	}



	//显示提示窗口
	private void showPopupWindow()
	{
		Layout layout=getLayout();
		int SelectionLine=layout.getLineForOffset(getSelectionEnd());
		//int mSelectionLineBottom=SelectionLine * lineHeight + lineHeight;
		int selectionLineBottom = layout.getLineBottom(SelectionLine);

		int top=selectionLineBottom - getScrollY();

		ListAdapter adp=list.getAdapter();

		int h=list.getItemHeight();
		int n=Math.min(2, adp.getCount());
		int w=getWidth() / 2;
		//list.setLayoutParams(new FrameLayout.LayoutParams(w,h*n));

		if (top + h * n > getHeight())
			top = top - h * n - getLineHeight();

		int x=getScrollX();
		float left=layout.getSecondaryHorizontal(getSelectionEnd()) - x ;

		if (left > w)
			left = left - w;
		if (!pop.isShowing())
			pop.showAtLocation(this, Gravity.TOP | Gravity.LEFT, 0, 0);
		pop.update((int)left + getPaddingLeft(), top + mAbsTop, w, h * n);
	}

	//更新提示窗口内容
	private void updataList(String[] arr)
	{
		ArrayAdapter<Object> adapter=new ArrayAdapter<Object>(getContext(), android.R.layout.simple_list_item_1, arr);
		list.setAdapter(adapter);
		showPopupWindow();
	}

	//更新提示窗口内容
	private void updataList(ArrayList buf)
	{
		if (!buf.isEmpty())
		{
			ArrayAdapter<ArrayList> adapter=new ArrayAdapter<ArrayList>(getContext(), android.R.layout.simple_list_item_1, buf);
			list.setAdapter(adapter);
			showPopupWindow();
		}
		else
		{
			pop.dismiss();
		}

	}

	//获得提示窗口要显示的内容
	public void updataAutoInput(int index)
	{
		CharSequence tmp=spannable.subSequence(blockStart, index);
		if (tmp.length() < 1)
		{
			pop.dismiss();
			return;
		}
		if (tmp.charAt(tmp.length() - 1) == '\n')
		{
			tmp = tmp.toString().replaceAll(stringTarget, "").replaceAll(commentTarget, "");
			int ps = 0;
			Pattern p=Pattern.compile("^( +)");
			Matcher m=p.matcher(tmp);
			if (m.find())
			{
				ps = m.group().length();
				//spannable.insert(index,m.group());
			}
			Pattern addp=Pattern.compile("\\b(do|function|repeat|then)\\b|\\{");
			Matcher addm=addp.matcher(tmp);

			while (addm.find())
				ps = ps + 4;
			Pattern subp=Pattern.compile("\\b(elseif|end|until)\\b|\\}");
			Matcher subm=subp.matcher(tmp);

			while (subm.find())
				ps = ps - 4;

			if (ps > 0)
				spannable.insert(index, "                                ".substring(0, ps));

			pop.dismiss();
			return;
		}
		Pattern p1 =Pattern.compile("\\b(\\w+)(\\.?)(\\w*)\\z"); 
		Matcher m1 =p1.matcher(tmp);

		if (!m1.find())
		{
			pop.dismiss();
			return;
		}

		String g1=m1.group(1);
		String g2=m1.group(2);
		String g3=m1.group(3);

		ArrayList<String> buf=new ArrayList<String>();

		if (packages.get(g1) != null && !g2.isEmpty() && g3.isEmpty())
		{
			subIndex = 0;
			updataList(packages.get(g1));
		}
		else if (packages.get(g1) != null && !g2.isEmpty() && !g3.isEmpty())
		{
			subIndex = g3.length();
			updataList(packages.get(g1));
			for (String k:packages.get(g1))
			{
				if (k.indexOf(g3) == 0)
					buf.add(k);
			}
			updataList(buf);
		}
		else if (!g1.isEmpty() && g2.isEmpty() && g3.isEmpty())
		{
			subIndex = g1.length();
			for (String k:keywords)
			{
				if (k.indexOf(g1) == 0)
					buf.add(k);
			}
			updataList(buf);
		}
		else
		{
			pop.dismiss();
		}
	}



	//转到行
	public boolean gotoLine(int line)
	{
		line--;
		if (line > getLineCount())
			return false;
		Layout layout = getLayout();
		setSelection(layout.getLineStart(line), layout.getLineEnd(line));
//				Selection.setSelection(getText(),m.start(),m.end()-1);
		return true;
	}


	private void initScroll(Context context)
	{
//		helper = new Helper();
		mScroller = new Scroller(getContext());
		setFocusable(true);
		setWillNotDraw(false);
		final ViewConfiguration configuration = ViewConfiguration.get(context);
		mTouchSlop = configuration.getScaledTouchSlop();
		mMinimumVelocity = configuration.getScaledMinimumFlingVelocity();
		mMaximumVelocity = configuration.getScaledMaximumFlingVelocity();
	}


	private void fling(int velocityX, int velocityY)
	{
		int maxWidth=0;
		int maxHeight=0;
		if (getLineCount() < 100)
		{
			measure(0, 0);
			maxWidth = getMeasuredWidth() - getWidth() + getPaddingLeft() - getPaddingRight();
			maxHeight = getMeasuredHeight() - getHeight();
		}
		else
		{
			Layout layout = getLayout();
			int topLine = layout.getLineForVertical(getScrollY());
			int bottomLine = layout.getLineForVertical(getScrollY() + getHeight());
			for (int l=topLine;l <= bottomLine;l++)
			{
				maxWidth = (int) Math.max(maxWidth, layout.getLineWidth(l));
			}
			maxWidth = maxWidth - getWidth() + getPaddingLeft();
			maxHeight = getLineHeight() * getLineCount() - getHeight();
		}

		//Log.d("me",getMeasuredWidth()+";"+getMeasuredHeight());

		if (getLineCount() > 0)
		{
			mScroller.fling(getScrollX(), getScrollY(), velocityX , velocityY, 0, maxWidth, 0, maxHeight);
//			final boolean movingDown = velocityY > 0;
			awakenScrollBars(mScroller.getDuration());
			invalidate();
		}
	}

	private void obtainVelocityTracker(MotionEvent event)
	{
		if (mVelocityTracker == null)
		{
			mVelocityTracker = VelocityTracker.obtain();
		}
		mVelocityTracker.addMovement(event);
	}

	private void releaseVelocityTracker()
	{
		if (mVelocityTracker != null)
		{
			mVelocityTracker.recycle();
			mVelocityTracker = null;
		}
	}

	@Override
	public boolean onTouchEvent(MotionEvent event)
	{
		if (event.getAction() == MotionEvent.ACTION_DOWN
			&& event.getEdgeFlags() != 0)
		{
			return false;
		}

		obtainVelocityTracker(event);
		final int action = event.getAction();
		final float x = event.getX();
		final float y = event.getY();

		switch (action)
		{
			case MotionEvent.ACTION_DOWN:
				if (!mScroller.isFinished())
				{
					mScroller.abortAnimation();
				}
				pop.dismiss();
				//setCursorVisible(true);
				mLastMotionY = y;
				mLastMotionX = x;
				mLastScrollY = getScrollY();
				mLastScrollX = getScrollX();
				flingOrientation = FLINGED;
				touchOut = false;
				Layout layout=getLayout();
				int line=layout.getLineForVertical((int)y + getScrollY());

				if (x + getScrollX() - getWidth() / 4 > Math.max(getWidth() / 2, layout.getLineWidth(line)))
				{
					touchOut = true;
					return true;
				}
				if (x < getPaddingLeft() / 2)
				{
					return true;
				}
				break;
			case MotionEvent.ACTION_MOVE:
				if (x < getPaddingLeft() / 2)
				{
					scrollTo(getScrollX(), (int)(Math.max(0, getLineCount() * getLineHeight() - getHeight()) * Math.min(1, Math.max(0, (y / getHeight())))));
					return true;
				}
				touchOut = false;
				int deltaX = (int) (mLastMotionX - x);
				int deltaY = (int) (mLastMotionY - y);
				//setCursorVisible(false);
				if (flingOrientation == FLINGED)
				{
					if (Math.abs(deltaX) >= Math.abs(deltaY))
						flingOrientation = HORIZONTAL;
					else
						flingOrientation = VERTICAL;
				}
				break;
			case MotionEvent.ACTION_UP:
				final VelocityTracker velocityTracker = mVelocityTracker;
				velocityTracker.computeCurrentVelocity(1000, mMaximumVelocity);
				int initialVelocityY = (int) velocityTracker.getYVelocity();
				int initialVelocityX = (int) velocityTracker.getXVelocity();
				if (flingOrientation == HORIZONTAL)
					initialVelocityY = 0;
				else if (flingOrientation == VERTICAL)
					initialVelocityX = 0;
				if ((Math.max(Math.abs(initialVelocityX), Math.abs(initialVelocityY)) > mMinimumVelocity)
					&& getLineCount() > 0)
				{
					fling(-initialVelocityX, -initialVelocityY);
				}
				releaseVelocityTracker();
				flingOrientation = FLINGED;
				if (touchOut)
				{
					Layout layout2=getLayout();
					int line2=layout2.getLineForVertical((int)y + getScrollY());
					if (x + getScrollX() - getWidth() / 4 > Math.max(getWidth() / 2, layout2.getLineWidth(line2)))
					{

						return true;
					}
				}
				break;
		}
		return  super.onTouchEvent(event);
	}

	@Override
	public void computeScroll()
	{
		if (mScroller.computeScrollOffset())
		{
			scrollTo(mScroller.getCurrX() + 4, mScroller.getCurrY() + 4);
			postInvalidate();
			return;
		}
		if (flingOrientation != HORIZONTAL)
			if (mScroller.getCurrY() != getScrollY())
				refreshHightlight();

	}


	@Override
	public void scrollTo(int x, int y)
	{
		// TODO: Implement this method
		if (flingOrientation == VERTICAL)
			x = mLastScrollX;
		if (flingOrientation == HORIZONTAL)
			y = mLastScrollY;
		super.scrollTo(x, y);
	}


	/**
	 * 初始化
	 */
	private void initUndo()
	{
		undoHistories = new ArrayList<EditHistory>();
		redoHistories = new ArrayList<EditHistory>();
		addTextChangedListener(new TextWatcher() {
				@Override
				public void onTextChanged(CharSequence s, int start, int before, int count)
				{

				}

				@Override
				public void beforeTextChanged(CharSequence s, int start, int count,
											  int after)
				{
					if (activate)
					{
						redoHistories.add(new EditHistory(s.toString().subSequence(start, start + count), start, count, after));
						activate = false;
					}
					else
					{
						undoHistories.add(new EditHistory(s.toString().subSequence(start, start + count), start, count, after));
						if (clear)
						{
							redoHistories.clear();
						}
						clear = true;
					}
				}

				@Override
				public void afterTextChanged(Editable s)
				{
					if (getLineCount() != lastLineCount)
					{
						lastLineCount = getLineCount();
						setPadding((int)mPaint.measureText(lastLineCount + "  "), 0,  getLineHeight() / 3, 0);

					}
				}
			});
	}

	/**
	 * 撤销
	 */
	public void undo()
	{
		if (canUndo())
		{
			activate = true;
			Editable editable = getEditableText();
			EditHistory history = undoHistories.remove(undoHistories.size() - 1);
			editable.replace(history.start, history.start + history.after, history.text);
			setSelection(history.start + history.count);
		}
	}

	/**
	 * 重做
	 */
	public void redo()
	{
		if (canRedo())
		{
			clear = false;
			Editable editable = getEditableText();
			EditHistory history = redoHistories.remove(redoHistories.size() - 1);
			editable.replace(history.start, history.start + history.after, history.text);
			setSelection(history.start + history.count);
		}
	}

	public void clearHistories()
	{
		redoHistories.clear();
		undoHistories.clear();
	}
	/**
	 * 是否可以撤销
	 */
	public boolean canUndo()
	{
		if (undoHistories.size() > 0)
		{
			return true;
		}
		return false;
	}

	/**
	 * 是否可以重做
	 */
	public boolean canRedo()
	{
		if (redoHistories.size() > 0)
		{
			return true;
		}
		return false;
	}


	static class CharSeqReader extends Reader
	{
        int offset = 0;
        CharSequence src;

        CharSeqReader(CharSequence src)
		{
            this.src = src;
        }

        @Override
        public void close() throws IOException
		{
            src = null;
            offset = 0;
        }

        @Override
        public int read(char[] chars, int i, int i1) throws IOException
		{
            int len = Math.min(src.length() - offset, i1);
            for (int n = 0; n < len; n++)
			{
                chars[i++] = src.charAt(offset++);
            }
            if (len <= 0)
                return  -1;
            return len;
        }
    }


	/**
	 * 存储Edit操作
	 */
	private class EditHistory
	{
		public CharSequence text;
		public int start;
		public int count;
		public int after;

		public EditHistory(CharSequence text, int start, int count, int after)
		{
			this.text = text;
			this.start = start;
			this.count = count;
			this.after = after;
		}

		@Override
		public String toString()
		{
			return "EditHistory [text=" + text + ", start=" + start
				+ ", count=" + count + ", after=" + after + "]";
		}
	}

}
