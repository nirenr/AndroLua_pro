/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights "eserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */

/*
 *****************************************************************************
 *
 * --------------------------------- row length
 * Hello World(\n)                 | 12
 * This is a test of the caret(\n) | 28
 * func|t|ions(\n)                 | 10
 * of this program(EOF)            | 16
 * ---------------------------------
 *
 * The figure illustrates the convention for counting characters.
 * Rows 36 to 39 of a hypothetical text file are shown.
 * The 0th char of the file is off-screen.
 * Assume the first char on screen is the 257th char.
 * The caret is before the char 't' of the word "functions". The caret is drawn
 * as a filled blue rectangle enclosing the 't'.
 *
 * _caretPosition == 257 + 12 + 28 + 4 == 301
 *
 * Note 1: EOF (End Of File) is a real char with a length of 1
 * Note 2: Characters enclosed in parentheses are non-printable
 *
 *****************************************************************************
 *
 * There is a difference between rows and lines in TextWarrior.
 * Rows are displayed while lines are a pure logical construct.
 * When there is no word-wrap, a line of text is displayed as a row on screen.
 * With word-wrap, a very long line of text may be split across several rows
 * on screen.
 *
 */
package com.myopicmobile.textwarrior.android;

import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnDismissListener;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.os.Build;
import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.text.ClipboardManager;
import android.text.InputType;
import android.text.Selection;
import android.text.SpannableStringBuilder;
import android.text.method.CharacterPickerDialog;
import android.util.AttributeSet;
import android.util.Log;
import android.util.SparseArray;
import android.view.InputDevice;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.accessibility.AccessibilityEvent;
import android.view.accessibility.AccessibilityManager;
import android.view.accessibility.AccessibilityNodeInfo;
import android.view.accessibility.AccessibilityRecord;
import android.view.animation.AnimationUtils;
import android.view.inputmethod.BaseInputConnection;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
import android.widget.Scroller;

import com.myopicmobile.textwarrior.common.AutoIndent;
import com.myopicmobile.textwarrior.common.ColorScheme;
import com.myopicmobile.textwarrior.common.ColorScheme.Colorable;
import com.myopicmobile.textwarrior.common.ColorSchemeLight;
import com.myopicmobile.textwarrior.common.Document;
import com.myopicmobile.textwarrior.common.DocumentProvider;
import com.myopicmobile.textwarrior.common.Language;
import com.myopicmobile.textwarrior.common.LanguageLua;
import com.myopicmobile.textwarrior.common.Lexer;
import com.myopicmobile.textwarrior.common.Pair;
import com.myopicmobile.textwarrior.common.RowListener;
import com.myopicmobile.textwarrior.common.TextWarriorException;

import java.util.ArrayList;
import java.util.List;

import static android.view.accessibility.AccessibilityNodeInfo.ACTION_NEXT_AT_MOVEMENT_GRANULARITY;
import static android.view.accessibility.AccessibilityNodeInfo.ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY;
import static com.myopicmobile.textwarrior.common.Lexer.NORMAL;

/**
 * A custom text view that uses a solid shaded caret (aka cursor) instead of a
 * blinking caret and allows a variety of navigation methods to be easily
 * integrated.
 * <p>
 * It also has a built-in syntax highlighting feature. The global programming
 * language syntax to use is specified with Lexer.setLanguage(Language).
 * To disable syntax highlighting, simply pass LanguageNonProg to that function.
 * <p>
 * Responsibilities
 * 1. Display text
 * 2. Display padding
 * 3. Scrolling
 * 4. Store and display caret position and selection range
 * 5. Store font type, font size, and tab length
 * 6. Interpret non-touch input events and shortcut keystrokes, triggering
 * the appropriate inner class controller actions
 * 7. Reset view, set cursor position and selection range
 * <p>
 * Inner class controller responsibilities
 * 1. Caret movement
 * 2. Activate/deactivate selection mode
 * 3. Cut, copy, paste, delete, insert
 * 4. Schedule areas to repaint and analyze for spans in response to edits
 * 5. Directs scrolling if caret movements or edits causes the caret to be off-screen
 * 6. Notify rowListeners when caret row changes
 * 7. Provide helper methods for InputConnection to setComposingText from the IME
 * <p>
 * This class is aware that the underlying text buffer uses an extra char (EOF)
 * to mark the end of the text. The text size reported by the text buffer includes
 * this extra char. Some bounds manipulation is done so that this implementation
 * detail is hidden from client classes.
 */
public class FreeScrollingTextField extends View
        implements Document.TextFieldMetrics {

    //---------------------------------------------------------------------
    //--------------------------  Caret Scroll  ---------------------------
    public final static int SCROLL_UP = 0;
    public final static int SCROLL_DOWN = 1;
    public final static int SCROLL_LEFT = 2;
    public final static int SCROLL_RIGHT = 3;
    /**
     * Scale factor for the width of a caret when on a NEWLINE or EOF char.
     * A factor of 1.0 is equals to the width of a space character
     */
    protected static float EMPTY_CARET_WIDTH_SCALE = 0.75f;
    /**
     * When in selection mode, the caret height is scaled by this factor
     */
    protected static float SEL_CARET_HEIGHT_SCALE = 0.5f;
    protected static int DEFAULT_TAB_LENGTH_SPACES = 4;
    protected static int BASE_TEXT_SIZE_PIXELS = 16;
    protected static long SCROLL_PERIOD = 250; //in milliseconds
    /*
     * Hash map for determining which characters to let the user choose from when
	 * a hardware key is long-pressed. For example, long-pressing "e" displays
	 * choices of "é, è, ê, ë" and so on.
	 * This is biased towards European locales, but is standard Android behavior
	 * for TextView.
	 *
	 * Copied from android.text.method.QwertyKeyListener, dated 2006
	 */
    private static SparseArray<String> PICKER_SETS =
            new SparseArray<String>();

    static {
        PICKER_SETS.put('A', "\u00C0\u00C1\u00C2\u00C4\u00C6\u00C3\u00C5\u0104\u0100");
        PICKER_SETS.put('C', "\u00C7\u0106\u010C");
        PICKER_SETS.put('D', "\u010E");
        PICKER_SETS.put('E', "\u00C8\u00C9\u00CA\u00CB\u0118\u011A\u0112");
        PICKER_SETS.put('G', "\u011E");
        PICKER_SETS.put('L', "\u0141");
        PICKER_SETS.put('I', "\u00CC\u00CD\u00CE\u00CF\u012A\u0130");
        PICKER_SETS.put('N', "\u00D1\u0143\u0147");
        PICKER_SETS.put('O', "\u00D8\u0152\u00D5\u00D2\u00D3\u00D4\u00D6\u014C");
        PICKER_SETS.put('R', "\u0158");
        PICKER_SETS.put('S', "\u015A\u0160\u015E");
        PICKER_SETS.put('T', "\u0164");
        PICKER_SETS.put('U', "\u00D9\u00DA\u00DB\u00DC\u016E\u016A");
        PICKER_SETS.put('Y', "\u00DD\u0178");
        PICKER_SETS.put('Z', "\u0179\u017B\u017D");
        PICKER_SETS.put('a', "\u00E0\u00E1\u00E2\u00E4\u00E6\u00E3\u00E5\u0105\u0101");
        PICKER_SETS.put('c', "\u00E7\u0107\u010D");
        PICKER_SETS.put('d', "\u010F");
        PICKER_SETS.put('e', "\u00E8\u00E9\u00EA\u00EB\u0119\u011B\u0113");
        PICKER_SETS.put('g', "\u011F");
        PICKER_SETS.put('i', "\u00EC\u00ED\u00EE\u00EF\u012B\u0131");
        PICKER_SETS.put('l', "\u0142");
        PICKER_SETS.put('n', "\u00F1\u0144\u0148");
        PICKER_SETS.put('o', "\u00F8\u0153\u00F5\u00F2\u00F3\u00F4\u00F6\u014D");
        PICKER_SETS.put('r', "\u0159");
        PICKER_SETS.put('s', "\u00A7\u00DF\u015B\u0161\u015F");
        PICKER_SETS.put('t', "\u0165");
        PICKER_SETS.put('u', "\u00F9\u00FA\u00FB\u00FC\u016F\u016B");
        PICKER_SETS.put('y', "\u00FD\u00FF");
        PICKER_SETS.put('z', "\u017A\u017C\u017E");
        PICKER_SETS.put(KeyCharacterMap.PICKER_DIALOG_INPUT,
                "\u2026\u00A5\u2022\u00AE\u00A9\u00B1[]{}\\|");
        PICKER_SETS.put('/', "\\");

        // From packages/inputmethods/LatinIME/res/xml/kbd_symbols.xml

        PICKER_SETS.put('1', "\u00b9\u00bd\u2153\u00bc\u215b");
        PICKER_SETS.put('2', "\u00b2\u2154");
        PICKER_SETS.put('3', "\u00b3\u00be\u215c");
        PICKER_SETS.put('4', "\u2074");
        PICKER_SETS.put('5', "\u215d");
        PICKER_SETS.put('7', "\u215e");
        PICKER_SETS.put('0', "\u207f\u2205");
        PICKER_SETS.put('$', "\u00a2\u00a3\u20ac\u00a5\u20a3\u20a4\u20b1");
        PICKER_SETS.put('%', "\u2030");
        PICKER_SETS.put('*', "\u2020\u2021");
        PICKER_SETS.put('-', "\u2013\u2014");
        PICKER_SETS.put('+', "\u00b1");
        PICKER_SETS.put('(', "[{<");
        PICKER_SETS.put(')', "]}>");
        PICKER_SETS.put('!', "\u00a1");
        PICKER_SETS.put('"', "\u201c\u201d\u00ab\u00bb\u02dd");
        PICKER_SETS.put('?', "\u00bf");
        PICKER_SETS.put(',', "\u201a\u201e");

        // From packages/inputmethods/LatinIME/res/xml/kbd_symbols_shift.xml

        PICKER_SETS.put('=', "\u2260\u2248\u221e");
        PICKER_SETS.put('<', "\u2264\u00ab\u2039");
        PICKER_SETS.put('>', "\u2265\u00bb\u203a");
    }

    private final Scroller _scroller;
    protected boolean _isEdited = false; // whether the text field is dirtied
    protected TouchNavigationMethod _navMethod;
    protected DocumentProvider _hDoc; // the model in MVC
    protected int _caretPosition = 0;
    protected int _selectionAnchor = -1; // inclusive
    protected int _selectionEdge = -1; // exclusive
    protected int _tabLength = DEFAULT_TAB_LENGTH_SPACES;
    protected ColorScheme _colorScheme = new ColorSchemeLight();
    protected boolean _isHighlightRow = false;
    protected boolean _showNonPrinting = false;
    protected boolean _isAutoIndent = true;
    protected int _autoIndentWidth = 4;
    protected boolean _isLongPressCaps = false;
    protected AutoCompletePanel _autoCompletePanel;
    protected boolean _isAutoComplete = true;
    private TextFieldController _fieldController; // the controller in MVC
    private TextFieldInputConnection _inputConnection;
    private RowListener _rowLis;
    private OnSelectionChangedListener _selModeLis;
    private int _caretRow = 0; // can be calculated, but stored for efficiency purposes
    private Paint _brush;
    /**
     * Max amount that can be scrolled horizontally based on the longest line
     * displayed on screen so far
     */
    private int _xExtent = 0;
    private int _leftOffset = 0;
    private boolean _showLineNumbers = false;
    private ClipboardPanel _clipboardPanel;
    private ClipboardManager _clipboardManager;
    private float _zoomFactor = 1;
    private int _caretX;
    private int _caretY;
    private TextChangeListener _textLis;
    private int _topOffset;
    private Typeface _defTypeface = Typeface.DEFAULT;
    private Typeface _boldTypeface = Typeface.DEFAULT_BOLD;
    private Typeface _italicTypeface = Typeface.create(Typeface.DEFAULT, Typeface.ITALIC);
    private char _emoji;
    private boolean _isLayout;
    private Paint _brushLine;
    private int _alphaWidth;
    private final Runnable _scrollCaretDownTask = new Runnable() {
        @Override
        public void run() {
            _fieldController.moveCaretDown();
            if (!caretOnLastRowOfFile()) {
                postDelayed(_scrollCaretDownTask, SCROLL_PERIOD);
            }
        }
    };
    private final Runnable _scrollCaretUpTask = new Runnable() {
        @Override
        public void run() {
            _fieldController.moveCaretUp();
            if (!caretOnFirstRowOfFile()) {
                postDelayed(_scrollCaretUpTask, SCROLL_PERIOD);
            }
        }
    };
    private final Runnable _scrollCaretLeftTask = new Runnable() {
        @Override
        public void run() {
            _fieldController.moveCaretLeft(false);
            if (_caretPosition > 0 &&
                    _caretRow == _hDoc.findRowNumber(_caretPosition - 1)) {
                postDelayed(_scrollCaretLeftTask, SCROLL_PERIOD);
            }
        }
    };
    private final Runnable _scrollCaretRightTask = new Runnable() {
        @Override
        public void run() {
            _fieldController.moveCaretRight(false);
            if (!caretOnEOF() &&
                    _caretRow == _hDoc.findRowNumber(_caretPosition + 1)) {
                postDelayed(_scrollCaretRightTask, SCROLL_PERIOD);
            }
        }
    };
    private int _spaceWidth;
    /**
     * Like {@link View#scrollBy}, but scroll smoothly instead of immediately.
     *
     * @param dx the number of pixels to scroll by on the X axis
     * @param dy the number of pixels to scroll by on the Y axis
     */
    private long mLastScroll;
    private boolean isAccessibilityEnabled = false;

    public FreeScrollingTextField(Context context) {
        super(context);
        _hDoc = new DocumentProvider(this);
        _navMethod = new TouchNavigationMethod(this);
        _scroller = new Scroller(context);
        initView();
    }

    public FreeScrollingTextField(Context context, AttributeSet attrs) {
        super(context, attrs);
        _hDoc = new DocumentProvider(this);
        _navMethod = new TouchNavigationMethod(this);
        _scroller = new Scroller(context);
        initView();
    }

    public FreeScrollingTextField(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        _hDoc = new DocumentProvider(this);
        _navMethod = new TouchNavigationMethod(this);
        _scroller = new Scroller(context);
        initView();
    }

    public int getTopOffset() {
        return _topOffset;
    }

    public int getAutoIndentWidth() {
        return _autoIndentWidth;
    }

    public void setAutoIndentWidth(int autoIndentWidth) {
        _autoIndentWidth = autoIndentWidth;
    }

    public int getCaretY() {
        return _caretY;
    }

    public int getCaretX() {
        return _caretX;
    }

    public boolean isShowLineNumbers() {
        return _showLineNumbers;
    }

    public void setShowLineNumbers(boolean showLineNumbers) {
        _showLineNumbers = showLineNumbers;
    }

    public int getLeftOffset() {
        return _leftOffset;
    }

    public float getTextSize() {
        return _brush.getTextSize();
    }

    public void setTextSize(int pix) {
        if (pix <= 8 || pix >= 80 || pix == _brush.getTextSize()) {
            return;
        }
        double oldHeight = rowHeight();
        double oldWidth = getAdvance('a');
        _zoomFactor = pix / BASE_TEXT_SIZE_PIXELS;
        _brush.setTextSize(pix);
        _brushLine.setTextSize(pix);
        if (_hDoc.isWordWrap())
            _hDoc.analyzeWordWrap();
        _fieldController.updateCaretRow();
        double x = getScrollX() * ((double) getAdvance('a') / oldWidth);
        double y = getScrollY() * ((double) rowHeight() / oldHeight);
        scrollTo((int) x, (int) y);
        _alphaWidth = (int) _brush.measureText("a");
        _spaceWidth = (int) _brush.measureText(" ");
        //int idx=coordToCharIndex(getScrollX(), getScrollY());
        //if (!makeCharVisible(idx))
        {
            invalidate();
        }
    }

    public void replaceText(int from, int charCount, String text) {
        _hDoc.beginBatchEdit();
        _fieldController.replaceText(from, charCount, text);
        _fieldController.stopTextComposing();
        _hDoc.endBatchEdit();
    }

    public void format() {
        selectText(false);
        CharSequence text = AutoIndent.format(new DocumentProvider(_hDoc), _autoIndentWidth);
        _hDoc.beginBatchEdit();
        _hDoc.deleteAt(0, _hDoc.docLength() - 1, System.nanoTime());
        _hDoc.insertBefore(text.toString().toCharArray(), 0, System.nanoTime());
        _hDoc.endBatchEdit();
        _hDoc.clearSpans();
        respan();
        invalidate();
    }

    public int getLength() {
        return _hDoc.docLength();
    }

    private void initView() {
        AccessibilityManager accessibilityManager =
                (AccessibilityManager) getContext().getSystemService(Context.ACCESSIBILITY_SERVICE);
        isAccessibilityEnabled = accessibilityManager.isTouchExplorationEnabled();
        _fieldController = this.new TextFieldController();
        _clipboardManager = (ClipboardManager) getContext().getSystemService(Context.CLIPBOARD_SERVICE);
        _brush = new Paint();
        _brush.setAntiAlias(true);
        _brush.setTextSize(BASE_TEXT_SIZE_PIXELS);
        _brushLine = new Paint();
        _brushLine.setAntiAlias(true);
        _brushLine.setTextSize(BASE_TEXT_SIZE_PIXELS);
        //setBackgroundColor(_colorScheme.getColor(Colorable.BACKGROUND));
        setLongClickable(true);
        setFocusableInTouchMode(true);
        setHapticFeedbackEnabled(true);

        _rowLis = new RowListener() {
            @Override
            public void onRowChange(int newRowIndex) {
                // Do nothing
            }
        };

        _selModeLis = new OnSelectionChangedListener() {

            @Override
            public void onSelectionChanged(boolean active, int selStart, int selEnd) {
                // TODO: Implement this method
                if (active)
                    _clipboardPanel.show();
                else
                    _clipboardPanel.hide();
            }
        };

        _textLis = new TextChangeListener() {

            @Override
            public void onNewLine(String c, int _caretPosition, int p2) {
                // TODO: Implement this method
                if (isAccessibilityEnabled) {
                    AccessibilityEvent event = AccessibilityEvent.obtain(AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED);
                    event.setFromIndex(_caretPosition - 1);
                    event.setAddedCount(1);
                    sendAccessibilityEventUnchecked(event);
                }
                _autoCompletePanel.dismiss();
            }


            @Override
            public void onDel(CharSequence text, int caretPosition, int delCount) {
                if (isAccessibilityEnabled) {
                    AccessibilityEvent event = AccessibilityEvent.obtain(AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED);
                    event.setFromIndex(caretPosition - delCount);
                    event.setRemovedCount(delCount);
                    event.setBeforeText(_hDoc);
                    sendAccessibilityEventUnchecked(event);
                }
                _autoCompletePanel.dismiss();
            }

            @Override
            public void onAdd(CharSequence text, int caretPosition, int addCount) {
                if (isAccessibilityEnabled) {
                    AccessibilityEvent event = AccessibilityEvent.obtain(AccessibilityEvent.TYPE_VIEW_TEXT_CHANGED);
                    event.setFromIndex(caretPosition - addCount);
                    event.setAddedCount(addCount);
                    sendAccessibilityEventUnchecked(event);
                }
                if (!_isAutoComplete)
                    return;
                int curr = _caretPosition;
                for (; curr >= 0; curr--) {
                    char c = _hDoc.charAt(curr - 1);
                    if (!(Character.isLetterOrDigit(c) || c == '_' || c == '.')) {
                        break;
                    }
                }
                if (_caretPosition - curr > 0)
                    _autoCompletePanel.update(_hDoc.subSequence(curr, _caretPosition - curr));
                else
                    _autoCompletePanel.dismiss();
            }
        };
        resetView();
        _clipboardPanel = new ClipboardPanel(this);
        _autoCompletePanel = new AutoCompletePanel(this);
        _autoCompletePanel.setLanguage(LanguageLua.getInstance());
        //TODO find out if this function works
        //setScrollContainer(true);
        invalidate();
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {
//The input source is a pointing device associated with a display.
//输入源为可显示的指针设备，如：mouse pointing device(鼠标指针),stylus pointing device(尖笔设备)
        if (0 != (event.getSource() & InputDevice.SOURCE_CLASS_POINTER)) {
            switch (event.getAction()) {
                // process the scroll wheel movement...处理滚轮事件
                case MotionEvent.ACTION_SCROLL:
                    //获得垂直坐标上的滚动方向,也就是滚轮向下滚
                    /*if( event.getAxisValue(MotionEvent.AXIS_VSCROLL) < 0.0f){
                        //Log.i("fortest::onGenericMotionEvent", "down" );
                    }
                    //获得垂直坐标上的滚动方向,也就是滚轮向上滚
                    else{
                        //Log.i("fortest::onGenericMotionEvent", "up" );
                    }*/
                    scrollView(0, -event.getAxisValue(MotionEvent.AXIS_VSCROLL) * rowHeight());
                    return true;
            }
        }
        return super.onGenericMotionEvent(event);
    }

    private void scrollView(float distanceX, float distanceY) {
        int newX = (int) distanceX + getScrollX();
        int newY = (int) distanceY + getScrollY();

        // If scrollX and scrollY are somehow more than the recommended
        // max scroll values, use them as the new maximum
        // Also take into account the size of the caret,
        // which may extend beyond the text boundaries
        int maxWidth = Math.max(getMaxScrollX(),
                getScrollX());
        if (newX > maxWidth) {
            newX = maxWidth;
        } else if (newX < 0) {
            newX = 0;
        }

        int maxHeight = Math.max(getMaxScrollY(),
                getScrollY());
        if (newY > maxHeight) {
            newY = maxHeight;
        } else if (newY < 0) {
            newY = 0;
        }
        //_textField.scrollTo(newX, newY);
        smoothScrollTo(newX, newY);

    }

    @SuppressWarnings("deprecation")
    @Override
    public AccessibilityNodeInfo createAccessibilityNodeInfo() {
        AccessibilityNodeInfo node = super.createAccessibilityNodeInfo();
        if (Build.VERSION.SDK_INT > 20) {
            node.addAction(AccessibilityNodeInfo.AccessibilityAction.ACTION_SCROLL_FORWARD);
            node.addAction(AccessibilityNodeInfo.AccessibilityAction.ACTION_SCROLL_BACKWARD);
            node.addAction(AccessibilityNodeInfo.AccessibilityAction.ACTION_NEXT_AT_MOVEMENT_GRANULARITY);
            node.addAction(AccessibilityNodeInfo.AccessibilityAction.ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY);
        } else {
            if (Build.VERSION.SDK_INT > 15) {
                node.addAction(AccessibilityNodeInfo.ACTION_SCROLL_FORWARD);
                node.addAction(AccessibilityNodeInfo.ACTION_SCROLL_BACKWARD);
                node.addAction(AccessibilityNodeInfo.ACTION_NEXT_AT_MOVEMENT_GRANULARITY);
                node.addAction(AccessibilityNodeInfo.ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY);
            }
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
            node.setTextSelection(getSelectionStart(), getSelectionEnd());
        }
        node.setFocusable(true);
        if (Build.VERSION.SDK_INT >= 18)
            node.setEditable(true);
        if (Build.VERSION.SDK_INT >= 19)
            node.setMultiLine(true);
        return node;
    }

    @Override
    public boolean performAccessibilityAction(int action, Bundle arguments) {
        if (Build.VERSION.SDK_INT < 16)
            return super.performAccessibilityAction(action, arguments);

        switch (action) {
            case AccessibilityNodeInfo.ACTION_NEXT_AT_MOVEMENT_GRANULARITY:
                switch (arguments.getInt(AccessibilityNodeInfo.ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT)) {
                    case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_LINE:
                        moveCaretDown();
                        break;
                    case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_CHARACTER:
                        moveCaretRight();
                        break;
                }
                return true;
            case ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY:
                switch (arguments.getInt(AccessibilityNodeInfo.ACTION_ARGUMENT_MOVEMENT_GRANULARITY_INT)) {
                    case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_LINE:
                        moveCaretUp();
                        break;
                    case AccessibilityNodeInfo.MOVEMENT_GRANULARITY_CHARACTER:
                        moveCaretLeft();
                        break;
                }
                return true;
        }
        return super.performAccessibilityAction(action, arguments);
    }

    private void resetView() {
        _caretPosition = 0;
        _caretRow = 0;
        _xExtent = 0;
        _fieldController.setSelectText(false);
        _fieldController.stopTextComposing();
        _hDoc.clearSpans();
        if (getContentWidth() > 0 || !_hDoc.isWordWrap()) {
            _hDoc.analyzeWordWrap();
        }
        _rowLis.onRowChange(0);
        scrollTo(0, 0);
    }

    /**
     * Sets the text displayed to the document referenced by hDoc. The view
     * state is reset and the view is invalidated as a side-effect.
     */
    public void setDocumentProvider(DocumentProvider hDoc) {
        _hDoc = hDoc;
        resetView();
        _fieldController.cancelSpanning(); //stop existing lex threads
        _fieldController.determineSpans();
        invalidate();
        if (isAccessibilityEnabled) {
            setContentDescription(_hDoc);
        }
    }

    /**
     * Returns a DocumentProvider that references the same Document used by the
     * FreeScrollingTextField.
     */
    public DocumentProvider createDocumentProvider() {
        return new DocumentProvider(_hDoc);
    }

    public void setRowListener(RowListener rLis) {
        _rowLis = rLis;
    }

    public void setOnSelectionChangedListener(OnSelectionChangedListener sLis) {
        _selModeLis = sLis;
    }

    /**
     * Sets the caret navigation method used by this text field
     */
    public void setNavigationMethod(TouchNavigationMethod navMethod) {
        _navMethod = navMethod;
    }

    public void setChirality(boolean isRightHanded) {
        _navMethod.onChiralityChanged(isRightHanded);
    }

    // this used to be isDirty(), but was renamed to avoid conflicts with Android API 11
    public boolean isEdited() {
        return _isEdited;
    }

    //---------------------------------------------------------------------
    //-------------------------- Paint methods ----------------------------

    public void setEdited(boolean set) {
        _isEdited = set;
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        outAttrs.inputType = InputType.TYPE_CLASS_TEXT
                | InputType.TYPE_TEXT_FLAG_MULTI_LINE;
        outAttrs.imeOptions = EditorInfo.IME_FLAG_NO_ENTER_ACTION
                | EditorInfo.IME_ACTION_DONE
                | EditorInfo.IME_FLAG_NO_EXTRACT_UI;
        if (_inputConnection == null) {
            _inputConnection = this.new TextFieldInputConnection(this);
        } else {
            _inputConnection.resetComposingState();
        }
        return _inputConnection;
    }

    @Override
    public boolean onCheckIsTextEditor() {
        return true;
    }

    @Override
    public boolean isSaveEnabled() {
        return true;
    }

    //---------------------------------------------------------------------
    //------------------------- Layout methods ----------------------------
    //TODO test with height less than 1 complete row
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        //super.onMeasure(widthMeasureSpec,heightMeasureSpec);
        setMeasuredDimension(useAllDimensions(widthMeasureSpec), useAllDimensions(heightMeasureSpec));
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        // TODO: Implement this method
        if (changed) {
            Rect rect = new Rect();
            getWindowVisibleDisplayFrame(rect);
            _topOffset = rect.top + rect.height() - getHeight();
            if (!_isLayout)
                respan();
            _isLayout = right > 0;
            invalidate();
            _autoCompletePanel.setWidth(getWidth() / 2);
        }
        super.onLayout(changed, left, top, right, bottom);
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);
        if (_hDoc.isWordWrap() && oldw != w)
            _hDoc.analyzeWordWrap();
        _fieldController.updateCaretRow();
        if (h < oldh)
            makeCharVisible(_caretPosition);
    }

    private int useAllDimensions(int measureSpec) {
        int specMode = MeasureSpec.getMode(measureSpec);
        int result = MeasureSpec.getSize(measureSpec);
        if (specMode != MeasureSpec.EXACTLY && specMode != MeasureSpec.AT_MOST) {
            result = Integer.MAX_VALUE;
            TextWarriorException.fail("MeasureSpec cannot be UNSPECIFIED. Setting dimensions to max.");
        }

        return result;
    }

    protected int getNumVisibleRows() {
        return (int) Math.ceil((double) getContentHeight() / rowHeight());
    }

    protected int rowHeight() {
        Paint.FontMetricsInt metrics = _brush.getFontMetricsInt();
        return (metrics.descent - metrics.ascent);
    }

    /*
     The only methods that have to worry about padding are invalidate, draw
	 and computeVerticalScrollRange() methods. Other methods can assume that
	 the text completely fills a rectangular viewport given by getContentWidth()
	 and getContentHeight()
	 */
    protected int getContentHeight() {
        return getHeight() - getPaddingTop() - getPaddingBottom();
    }

    protected int getContentWidth() {
        return getWidth() - getPaddingLeft() - getPaddingRight();
    }

    /**
     * Determines if the View has been layout or is still being constructed
     */
    public boolean hasLayout() {
        return (getWidth() == 0); // simplistic implementation, but should work for most cases
    }

    /**
     * The first row of text to paint, which may be partially visible.
     * Deduced from the clipping rectangle given to onDraw()
     */
    private int getBeginPaintRow(Canvas canvas) {
        Rect bounds = canvas.getClipBounds();
        return bounds.top / rowHeight();
    }

    /**
     * The last row of text to paint, which may be partially visible.
     * Deduced from the clipping rectangle given to onDraw()
     */
    private int getEndPaintRow(Canvas canvas) {
        //clip top and left are inclusive; bottom and right are exclusive
        Rect bounds = canvas.getClipBounds();
        return (bounds.bottom - 1) / rowHeight();
    }

    /**
     * @return The x-value of the baseline for drawing text on the given row
     */
    public int getPaintBaseline(int row) {
        Paint.FontMetricsInt metrics = _brush.getFontMetricsInt();
        return (row + 1) * rowHeight() - metrics.descent;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        canvas.save();

        //translate clipping region to create padding around edges
        canvas.clipRect(getScrollX() + getPaddingLeft(),
                getScrollY() + getPaddingTop(),
                getScrollX() + getWidth() - getPaddingRight(),
                getScrollY() + getHeight() - getPaddingBottom());
        canvas.translate(getPaddingLeft(), getPaddingTop());
        realDraw(canvas);

        canvas.restore();

        _navMethod.onTextDrawComplete(canvas);
    }

    private void realDraw(Canvas canvas) {
        //----------------------------------------------
        // initialize and set up boundaries
        //----------------------------------------------
        int currRowNum = getBeginPaintRow(canvas);
        int currIndex = _hDoc.getRowOffset(currRowNum);
        if (currIndex < 0) {
            return;
        }
        int len = _hDoc.length();
        int currLineNum = isWordWrap() ? _hDoc.findLineNumber(currIndex) + 1 : currRowNum + 1;
        int lastLineNum = 0;
        if (_showLineNumbers)
            _leftOffset = (int) _brushLine.measureText(_hDoc.getRowCount() + " ");
        int endRowNum = getEndPaintRow(canvas);
        int paintX = 0;
        int paintY = getPaintBaseline(currRowNum);

        //----------------------------------------------
        // set up initial span color
        //----------------------------------------------
        int spanIndex = 0;
        List<Pair> spans = _hDoc.getSpans();

        // There must be at least one span to paint, even for an empty file,
        // where the span contains only the EOF character
        TextWarriorException.assertVerbose(!spans.isEmpty(),
                "No spans to paint in TextWarrior.paint()");
        if (spans.isEmpty())
            spans.add(new Pair(0, NORMAL));

        //TODO use binary search
        Pair nextSpan = spans.get(spanIndex++);

        Pair currSpan;
        int spanOffset = 0;
        int spanSize = spans.size();
        do {
            currSpan = nextSpan;
            spanOffset += currSpan.getFirst();
            if (spanIndex < spanSize) {
                nextSpan = spans.get(spanIndex++);
            } else {
                nextSpan = null;
            }
        }
        while (nextSpan != null && spanOffset <= currIndex);
        int currType = currSpan.getSecond();
        int lastType = currType;

        switch (currSpan.getSecond()) {
            case Lexer.KEYWORD:
                _brush.setTypeface(_boldTypeface);
                break;
            case Lexer.DOUBLE_SYMBOL_LINE:
                _brush.setTypeface(_italicTypeface);
                break;
            default:
                _brush.setTypeface(_defTypeface);
        }
        int spanColor = _colorScheme.getTokenColor(currSpan.getSecond());
        _brush.setColor(spanColor);

        //----------------------------------------------
        // start painting!
        //----------------------------------------------
        int rowCount = _hDoc.getRowCount();
        if (_showLineNumbers) {
            _brushLine.setColor(_colorScheme.getColor(Colorable.NON_PRINTING_GLYPH));
            canvas.drawLine(_leftOffset - _spaceWidth / 2, getScrollY(), _leftOffset - _spaceWidth / 2, getScrollY() + getHeight(), _brushLine);
            if (getMaxScrollY() > getHeight()) {
                int s = getScrollY() + (getHeight() * getScrollY() / getMaxScrollY());
                int e = getScrollY() + (getHeight() * (getScrollY() + getHeight()) / getMaxScrollY());
                if (e - s < _alphaWidth / 4)
                    e = s + _alphaWidth / 4;
                //_brushLine.setColor(_colorScheme.getColor(Colorable.CARET_FOREGROUND));
                canvas.drawLine(_leftOffset - _spaceWidth / 2 - _alphaWidth / 4, s, _leftOffset - _spaceWidth / 2 - _alphaWidth / 4, e, _brushLine);
            }
        }

        Typeface lastTypeface;
        switch (currType) {
            case Lexer.KEYWORD:
                lastTypeface = _boldTypeface;
                break;
            case Lexer.DOUBLE_SYMBOL_LINE:
                lastTypeface = _italicTypeface;
                break;
            default:
                lastTypeface = _defTypeface;
        }

        _brush.setTypeface(lastTypeface);

        while (currRowNum <= endRowNum) {
            int spanLen = spanOffset - currIndex;

            /*String row = _hDoc.getRow(currRowNum);
            boolean charDraw = false;
            if (row.contains("\t")) {
                charDraw = true;
            } else if (currRowNum == rowCount - 1) {
                charDraw = true;
            } else if (currRowNum == _caretRow) {
                charDraw = true;
            } else if (isSelectText()) {
                charDraw = true;
            }
*/
            int rowLen = _hDoc.getRowSize(currRowNum);
            if (currRowNum >= rowCount) {
                break;
            }

            if (_showLineNumbers && currLineNum != lastLineNum) {
                lastLineNum = currLineNum;
                String num = String.valueOf(currLineNum);
                drawLineNum(canvas, num, 0, paintY);
            }
            paintX = _leftOffset;

            int i = 0;

            while (i < rowLen) {
                // check if formatting changes are needed
                if (nextSpan != null && currIndex >= spanOffset) {
                    currSpan = nextSpan;

                    spanLen = currSpan.getFirst();
                    spanOffset += spanLen;
                    lastType = currType;
                    currType = currSpan.getSecond();

                    if (lastType != currType) {
                        Typeface currTypeface;
                        switch (currType) {
                            case Lexer.KEYWORD:
                                currTypeface = _boldTypeface;
                                break;
                            case Lexer.DOUBLE_SYMBOL_LINE:
                                currTypeface = _italicTypeface;
                                break;
                            default:
                                currTypeface = _defTypeface;
                        }

                        if (lastTypeface != currTypeface) {
                            _brush.setTypeface(currTypeface);
                            lastTypeface = currTypeface;
                        }

                        spanColor = _colorScheme.getTokenColor(currType);
                        _brush.setColor(spanColor);
                    }
                    if (spanIndex < spanSize) {
                        nextSpan = spans.get(spanIndex++);
                    } else {
                        nextSpan = null;
                    }
                }

                //if (charDraw) {
                if (currIndex == _caretPosition) {
                    drawCaret(canvas, paintX, paintY);
                }

                char c = _hDoc.charAt(currIndex);

                if (_fieldController.inSelectionRange(currIndex)) {
                    paintX += drawSelectedText(canvas, c, paintX, paintY);
                } else {
                    paintX += drawChar(canvas, c, paintX, paintY);
                }
                ++currIndex;
                ++i;
                spanLen--;
                /*} else {
                    if (i + spanLen > rowLen)
                        spanLen = rowLen - i;
                    int end = i + spanLen;
                    currIndex += spanLen;
                    paintX += drawString(canvas, row, i, end, paintX, paintY);
                    i += spanLen;
                }*/
            }

            if (_hDoc.charAt(currIndex - 1) == Language.NEWLINE)
                ++currLineNum;

            paintY += rowHeight();
            if (paintX > _xExtent) {
                // record widest line seen so far
                _xExtent = paintX;
            }
            ++currRowNum;
        } // end while
        doOptionHighlightRow(canvas);
        if (!isWordWrap())
            doBlockLine(canvas);
    }

    private void doBlockLine(Canvas canvas) {
        ArrayList<Rect> lines = Lexer.getLines();
        if (lines == null || lines.isEmpty())
            return;
        Rect bounds = canvas.getClipBounds();
        int bt = bounds.top;
        int bb = bounds.bottom;
        Rect curr=null;
        for (Rect rect : lines) {
            /*if(rect.top==_caretRow){
                doBlockRow(canvas,rect.bottom);
            }else if(rect.bottom==_caretRow){
                doBlockRow(canvas,rect.top);
            }*/
            int top = (rect.top + 1) * rowHeight();
            int bottom = rect.bottom * rowHeight();
            if (bottom < bt || top > bb)
                continue;
            int left = Math.min(getCharExtent(rect.left).getFirst(), getCharExtent(rect.right).getFirst());
            if(rect.left<_caretPosition&&rect.right>=_caretPosition){
                if(curr==null||curr.left<rect.left)
                curr=rect;
            }
            canvas.drawLine(left, top, left, bottom, _brushLine);
        }
        if(curr!=null){
            int top = (curr.top + 1) * rowHeight();
            int bottom = curr.bottom * rowHeight();
            if (bottom < bt || top > bb)
                return;
            int left = Math.min(getCharExtent(curr.left).getFirst(), getCharExtent(curr.right).getFirst());
            _brushLine.setColor(_colorScheme.getColor(Colorable.CARET_FOREGROUND));
            canvas.drawLine(left, top, left, bottom, _brushLine);
            _brushLine.setColor(_colorScheme.getColor(Colorable.NON_PRINTING_GLYPH));
        }
    }

    private void doBlockRow(Canvas canvas, int _caretRow) {
        if (_isHighlightRow) {
            int y = getPaintBaseline(_caretRow);
            int originalColor = _brush.getColor();
            _brush.setColor(_colorScheme.getColor(Colorable.LINE_HIGHLIGHT));

            int lineLength = Math.max(_xExtent, getContentWidth());
            //canvas.drawRect(0, y+1, lineLength, y+2, _brush);
            drawTextBackground(canvas, 0, y, lineLength);
            //_brush.setColor(0x88000000);
            _brush.setColor(originalColor);
        }
    }

    /**
     * Underline the caret row if the option for highlighting it is set
     */
    private void doOptionHighlightRow(Canvas canvas) {
        if (_isHighlightRow) {
            int y = getPaintBaseline(_caretRow);
            int originalColor = _brush.getColor();
            _brush.setColor(_colorScheme.getColor(Colorable.LINE_HIGHLIGHT));

            int lineLength = Math.max(_xExtent, getContentWidth());
            //canvas.drawRect(0, y+1, lineLength, y+2, _brush);
            drawTextBackground(canvas, 0, y, lineLength);
            //_brush.setColor(0x88000000);
            _brush.setColor(originalColor);
        }
    }

    private int drawString(Canvas canvas, String s, int start, int end, int paintX, int paintY) {
        int len = s.length();
        if (end >= len)
            end = len - 1;
        int charWidth = (int) _brush.measureText(s, start, end);
        if (paintX > getScrollX() || paintX < (getScrollX() + getContentWidth()))
            canvas.drawText(s, start, end, paintX, paintY, _brush);
        return charWidth;
    }

    private int drawSelectedString(Canvas canvas, String s, int paintX, int paintY) {
        int advance = (int) _brush.measureText(s);
        int oldColor = _brush.getColor();

        _brush.setColor(_colorScheme.getColor(Colorable.SELECTION_BACKGROUND));
        drawTextBackground(canvas, paintX, paintY, advance);

        _brush.setColor(_colorScheme.getColor(Colorable.SELECTION_FOREGROUND));
        canvas.drawText(s, paintX, paintY, _brush);

        _brush.setColor(oldColor);
        return advance;
    }

    private int drawChar(Canvas canvas, char c, int paintX, int paintY) {
        int originalColor = _brush.getColor();
        int charWidth = getAdvance(c, paintX);

        if (paintX > getScrollX() || paintX < (getScrollX() + getContentWidth()))
            switch (c) {
                case 0xd83c:
                case 0xd83d:
                    _emoji = c;
                    break;
                case ' ':
                    if (_showNonPrinting) {
                        _brush.setColor(_colorScheme.getColor(Colorable.NON_PRINTING_GLYPH));
                        canvas.drawText(Language.GLYPH_SPACE, 0, 1, paintX, paintY, _brush);
                        _brush.setColor(originalColor);
                    } else {
                        canvas.drawText(" ", 0, 1, paintX, paintY, _brush);
                    }
                    break;

                case Language.EOF: //fall-through
                case Language.NEWLINE:
                    if (_showNonPrinting) {
                        _brush.setColor(_colorScheme.getColor(Colorable.NON_PRINTING_GLYPH));
                        canvas.drawText(Language.GLYPH_NEWLINE, 0, 1, paintX, paintY, _brush);
                        _brush.setColor(originalColor);
                    }
                    break;

                case Language.TAB:
                    if (_showNonPrinting) {
                        _brush.setColor(_colorScheme.getColor(Colorable.NON_PRINTING_GLYPH));
                        canvas.drawText(Language.GLYPH_TAB, 0, 1, paintX, paintY, _brush);
                        _brush.setColor(originalColor);
                    }
                    break;

                default:
                    if (_emoji != 0) {
                        canvas.drawText(new char[]{_emoji, c}, 0, 2, paintX, paintY, _brush);
                        _emoji = 0;
                    } else {
                        char[] ca = {c};
                        canvas.drawText(ca, 0, 1, paintX, paintY, _brush);
                    }
                    break;
            }

        return charWidth;
    }

    // paintY is the baseline for text, NOT the top extent
    private void drawTextBackground(Canvas canvas, int paintX, int paintY,
                                    int advance) {
        Paint.FontMetricsInt metrics = _brush.getFontMetricsInt();
        canvas.drawRect(paintX,
                paintY + metrics.ascent,
                paintX + advance,
                paintY + metrics.descent,
                _brush);
    }

    private int drawSelectedText(Canvas canvas, char c, int paintX, int paintY) {
        int oldColor = _brush.getColor();
        int advance = getAdvance(c);

        _brush.setColor(_colorScheme.getColor(Colorable.SELECTION_BACKGROUND));
        drawTextBackground(canvas, paintX, paintY, advance);

        _brush.setColor(_colorScheme.getColor(Colorable.SELECTION_FOREGROUND));
        drawChar(canvas, c, paintX, paintY);

        _brush.setColor(oldColor);
        return advance;
    }

    private void drawCaret(Canvas canvas, int paintX, int paintY) {
        int originalColor = _brush.getColor();
        _caretX = paintX;
        _caretY = paintY;

        int caretColor = _colorScheme.getColor(Colorable.CARET_DISABLED);
        _brush.setColor(caretColor);
        // draw full caret
        drawTextBackground(canvas, paintX - 1, paintY, 2);
        _brush.setColor(originalColor);
    }

    private int drawLineNum(Canvas canvas, String s, int paintX, int paintY) {
        //int originalColor = _brush.getColor();
        //_brush.setColor(_colorScheme.getColor(Colorable.NON_PRINTING_GLYPH));
        canvas.drawText(s, paintX, paintY, _brushLine);
        //_brush.setColor(originalColor);
        return 0;
    }

    @Override
    final public int getRowWidth() {
        return getContentWidth() - _leftOffset;
    }

    /**
     * Returns printed width of c.
     * <p>
     * Takes into account user-specified tab width and also handles
     * application-defined widths for NEWLINE and EOF
     *
     * @param c Character to measure
     * @return Advance of character, in pixels
     */
    @Override
    public int getAdvance(char c) {
        int advance;

        switch (c) {
            case 0xd83c:
            case 0xd83d:
                advance = 0;
                break;
            case ' ':
                advance = getSpaceAdvance();
                break;
            case Language.NEWLINE: // fall-through
            case Language.EOF:
                advance = getEOLAdvance();
                break;
            case Language.TAB:
                advance = getTabAdvance();
                break;
            default:
                if (_emoji != 0) {
                    char[] ca = {_emoji, c};
                    advance = (int) _brush.measureText(ca, 0, 2);
                } else {
                    char[] ca = {c};
                    advance = (int) _brush.measureText(ca, 0, 1);
                }
                break;
        }

        return advance;
    }

    public int getAdvance(char c, int x) {
        int advance;

        switch (c) {
            case 0xd83c:
            case 0xd83d:
                advance = 0;
                break;
            case ' ':
                advance = getSpaceAdvance();
                break;
            case Language.NEWLINE: // fall-through
            case Language.EOF:
                advance = getEOLAdvance();
                break;
            case Language.TAB:
                advance = getTabAdvance(x);
                break;
            default:
                if (_emoji != 0) {
                    char[] ca = {_emoji, c};
                    advance = (int) _brush.measureText(ca, 0, 2);
                } else {
                    char[] ca = {c};
                    advance = (int) _brush.measureText(ca, 0, 1);
                }
                break;
        }

        return advance;
    }

    public int getCharAdvance(char c) {
        int advance;
        char[] ca = {c};
        advance = (int) _brush.measureText(ca, 0, 1);
        return advance;
    }

    protected int getSpaceAdvance() {
        if (_showNonPrinting) {
            return (int) _brush.measureText(Language.GLYPH_SPACE,
                    0, Language.GLYPH_SPACE.length());
        } else {
            return _spaceWidth;
        }
    }


    //---------------------------------------------------------------------
    //------------------- Scrolling and touch -----------------------------

    protected int getEOLAdvance() {
        if (_showNonPrinting) {
            return (int) _brush.measureText(Language.GLYPH_NEWLINE,
                    0, Language.GLYPH_NEWLINE.length());
        } else {
            return (int) (EMPTY_CARET_WIDTH_SCALE * _brush.measureText(" ", 0, 1));
        }
    }

    protected int getTabAdvance() {
        if (_showNonPrinting) {
            return _tabLength * (int) _brush.measureText(Language.GLYPH_SPACE,
                    0, Language.GLYPH_SPACE.length());
        } else {
            return _tabLength * _spaceWidth;
        }
    }

    protected int getTabAdvance(int x) {
        if (_showNonPrinting) {
            return _tabLength * (int) _brush.measureText(Language.GLYPH_SPACE,
                    0, Language.GLYPH_SPACE.length());
        } else {
            int i = (x - _leftOffset) / _spaceWidth % _tabLength;
            return (_tabLength - i) * _spaceWidth;
        }
    }

    /**
     * Invalidate rows from startRow (inclusive) to endRow (exclusive)
     */
    private void invalidateRows(int startRow, int endRow) {
        TextWarriorException.assertVerbose(startRow <= endRow && startRow >= 0,
                "Invalid startRow and/or endRow");

        Rect caretSpill = _navMethod.getCaretBloat();
        //TODO The ascent of (startRow+1) may jut inside startRow, so part of
        // that rows have to be invalidated as well.
        // This is a problem for Thai, Vietnamese and Indic scripts
        Paint.FontMetricsInt metrics = _brush.getFontMetricsInt();
        int top = startRow * rowHeight() + getPaddingTop();
        top -= Math.max(caretSpill.top, metrics.descent);
        top = Math.max(0, top);

        super.invalidate(0,
                top,
                getScrollX() + getWidth(),
                endRow * rowHeight() + getPaddingTop() + caretSpill.bottom);
    }

    /**
     * Invalidate rows from startRow (inclusive) to the end of the field
     */
    private void invalidateFromRow(int startRow) {
        TextWarriorException.assertVerbose(startRow >= 0,
                "Invalid startRow");

        Rect caretSpill = _navMethod.getCaretBloat();
        //TODO The ascent of (startRow+1) may jut inside startRow, so part of
        // that rows have to be invalidated as well.
        // This is a problem for Thai, Vietnamese and Indic scripts
        Paint.FontMetricsInt metrics = _brush.getFontMetricsInt();
        int top = startRow * rowHeight() + getPaddingTop();
        top -= Math.max(caretSpill.top, metrics.descent);
        top = Math.max(0, top);

        super.invalidate(0,
                top,
                getScrollX() + getWidth(),
                getScrollY() + getHeight());
    }

    private void invalidateCaretRow() {
        invalidateRows(_caretRow, _caretRow + 1);
    }

    private void invalidateSelectionRows() {
        int startRow = _hDoc.findRowNumber(_selectionAnchor);
        int endRow = _hDoc.findRowNumber(_selectionEdge);

        invalidateRows(startRow, endRow + 1);
    }

    /**
     * Scrolls the text horizontally and/or vertically if the character
     * specified by charOffset is not in the visible text region.
     * The view is invalidated if it is scrolled.
     *
     * @param charOffset The index of the character to make visible
     * @return True if the drawing area was scrolled horizontally
     * and/or vertically
     */
    private boolean makeCharVisible(int charOffset) {
        TextWarriorException.assertVerbose(
                charOffset >= 0 && charOffset < _hDoc.docLength(),
                "Invalid charOffset given");
        int scrollVerticalBy = makeCharRowVisible(charOffset);
        int scrollHorizontalBy = makeCharColumnVisible(charOffset);

        if (scrollVerticalBy == 0 && scrollHorizontalBy == 0) {
            return false;
        } else {
            scrollBy(scrollHorizontalBy, scrollVerticalBy);
            return true;
        }
    }

    /**
     * Calculates the amount to scroll vertically if the char is not
     * in the visible region.
     *
     * @param charOffset The index of the character to make visible
     * @return The amount to scroll vertically
     */
    private int makeCharRowVisible(int charOffset) {
        int scrollBy = 0;
        int charTop = _hDoc.findRowNumber(charOffset) * rowHeight();
        int charBottom = charTop + rowHeight();

        if (charTop < getScrollY()) {
            scrollBy = charTop - getScrollY();
        } else if (charBottom > (getScrollY() + getContentHeight())) {
            scrollBy = charBottom - getScrollY() - getContentHeight();
        }

        return scrollBy;
    }

    /**
     * Calculates the amount to scroll horizontally if the char is not
     * in the visible region.
     *
     * @param charOffset The index of the character to make visible
     * @return The amount to scroll horizontally
     */
    private int makeCharColumnVisible(int charOffset) {
        int scrollBy = 0;
        Pair visibleRange = getCharExtent(charOffset);

        int charLeft = visibleRange.getFirst();
        int charRight = visibleRange.getSecond();

        if (charRight > (getScrollX() + getContentWidth())) {
            scrollBy = charRight - getScrollX() - getContentWidth();
        }

        if (charLeft < getScrollX() + _alphaWidth) {
            scrollBy = charLeft - getScrollX() - _alphaWidth;
        }

        return scrollBy;
    }

    /**
     * Calculates the x-coordinate extent of charOffset.
     *
     * @return The x-values of left and right edges of charOffset. Pair.first
     * contains the left edge and Pair.second contains the right edge
     */
    protected Pair getCharExtent(int charOffset) {
        int row = _hDoc.findRowNumber(charOffset);
        int rowOffset = _hDoc.getRowOffset(row);
        int left = _leftOffset;
        int right = _leftOffset;
        boolean isEmoji = false;
        String rowText = _hDoc.getRow(row);
        int i = 0;

        int len = rowText.length();
        while (rowOffset + i <= charOffset && i < len) {
            char c = rowText.charAt(i);
            left = right;
            switch (c) {
                case 0xd83c:
                case 0xd83d:
                    isEmoji = true;
                    char[] ca = {c, rowText.charAt(i + 1)};
                    right += (int) _brush.measureText(ca, 0, 2);
                    break;
                case Language.NEWLINE:
                case Language.EOF:
                    right += getEOLAdvance();
                    break;
                case ' ':
                    right += getSpaceAdvance();
                    break;
                case Language.TAB:
                    right += getTabAdvance(right);
                    break;
                default:
                    if (isEmoji)
                        isEmoji = false;
                    else
                        right += getCharAdvance(c);
                    break;
            }
            ++i;
        }
        return new Pair(left, right);
    }

    /**
     * Returns the bounding box of a character in the text field.
     * The coordinate system used is one where (0, 0) is the top left corner
     * of the text, before padding is added.
     *
     * @param charOffset The character offset of the character of interest
     * @return Rect(left, top, right, bottom) of the character bounds,
     * or Rect(-1, -1, -1, -1) if there is no character at that coordinate.
     */
    Rect getBoundingBox(int charOffset) {
        if (charOffset < 0 || charOffset >= _hDoc.docLength()) {
            return new Rect(-1, -1, -1, -1);
        }

        int row = _hDoc.findRowNumber(charOffset);
        int top = row * rowHeight();
        int bottom = top + rowHeight();

        Pair xExtent = getCharExtent(charOffset);
        int left = xExtent.getFirst();
        int right = xExtent.getSecond();

        return new Rect(left, top, right, bottom);
    }

    public ColorScheme getColorScheme() {
        return _colorScheme;
    }

    public void setColorScheme(ColorScheme colorScheme) {
        _colorScheme = colorScheme;
        _navMethod.onColorSchemeChanged(colorScheme);
        setBackgroundColor(colorScheme.getColor(Colorable.BACKGROUND));
    }

    /**
     * Maps a coordinate to the character that it is on. If the coordinate is
     * on empty space, the nearest character on the corresponding row is returned.
     * If there is no character on the row, -1 is returned.
     * <p>
     * The coordinates passed in should not have padding applied to them.
     *
     * @param x x-coordinate
     * @param y y-coordinate
     * @return The index of the closest character, or -1 if there is
     * no character or nearest character at that coordinate
     */
    int coordToCharIndex(int x, int y) {
        int row = y / rowHeight();
        if (row > _hDoc.getRowCount())
            return _hDoc.docLength() - 1;

        int charIndex = _hDoc.getRowOffset(row);
        if (charIndex < 0) {
            //non-existent row
            return -1;
        }

        if (x < 0) {
            return charIndex; // coordinate is outside, to the left of view
        }

        String rowText = _hDoc.getRow(row);

        int extent = _leftOffset;
        int i = 0;
        boolean isEmoji = false;

        //x-=getAdvance('a')/2;
        int len = rowText.length();
        while (i < len) {
            char c = rowText.charAt(i);
            switch (c) {
                case 0xd83c:
                case 0xd83d:
                    isEmoji = true;
                    char[] ca = {c, rowText.charAt(i + 1)};
                    extent += (int) _brush.measureText(ca, 0, 2);
                    break;
                case Language.NEWLINE:
                case Language.EOF:
                    extent += getEOLAdvance();
                    break;
                case ' ':
                    extent += getSpaceAdvance();
                    break;
                case Language.TAB:
                    extent += getTabAdvance(extent);
                    break;
                default:
                    if (isEmoji)
                        isEmoji = false;
                    else
                        extent += getCharAdvance(c);

            }

            if (extent >= x) {
                break;
            }

            ++i;
        }


        if (i < rowText.length()) {
            return charIndex + i;
        }
        //nearest char is last char of line
        return charIndex + i - 1;
    }

    /**
     * Maps a coordinate to the character that it is on.
     * Returns -1 if there is no character on the coordinate.
     * <p>
     * The coordinates passed in should not have padding applied to them.
     *
     * @param x x-coordinate
     * @param y y-coordinate
     * @return The index of the character that is on the coordinate,
     * or -1 if there is no character at that coordinate.
     */
    int coordToCharIndexStrict(int x, int y) {
        int row = y / rowHeight();
        int charIndex = _hDoc.getRowOffset(row);

        if (charIndex < 0 || x < 0) {
            //non-existent row
            return -1;
        }

        String rowText = _hDoc.getRow(row);

        int extent = 0;
        int i = 0;
        boolean isEmoji = false;

        //x-=getAdvance('a')/2;
        int len = rowText.length();
        while (i < len) {
            char c = rowText.charAt(i);
            switch (c) {
                case 0xd83c:
                case 0xd83d:
                    isEmoji = true;
                    char[] ca = {c, rowText.charAt(i + 1)};
                    extent += (int) _brush.measureText(ca, 0, 2);
                    break;
                case Language.NEWLINE:
                case Language.EOF:
                    extent += getEOLAdvance();
                    break;
                case ' ':
                    extent += getSpaceAdvance();
                    break;
                case Language.TAB:
                    extent += getTabAdvance(extent);
                    break;
                default:
                    if (isEmoji)
                        isEmoji = false;
                    else
                        extent += getCharAdvance(c);

            }

            if (extent >= x) {
                break;
            }

            ++i;
        }

        if (i < rowText.length()) {
            return charIndex + i;
        }

        //no char enclosing x
        return -1;
    }

    /**
     * Not private to allow access by TouchNavigationMethod
     *
     * @return The maximum x-value that can be scrolled to for the current rows
     * of text in the viewport.
     */
    int getMaxScrollX() {
        if (isWordWrap())
            return _leftOffset;
        else
            return Math.max(0,
                    _xExtent - getContentWidth() + _navMethod.getCaretBloat().right + _alphaWidth);
    }

    /**
     * Not private to allow access by TouchNavigationMethod
     *
     * @return The maximum y-value that can be scrolled to.
     */
    int getMaxScrollY() {
        return Math.max(0,
                _hDoc.getRowCount() * rowHeight() - getContentHeight() / 2 + _navMethod.getCaretBloat().bottom);
    }

    @Override
    protected int computeVerticalScrollOffset() {
        return getScrollY();
    }

    @Override
    protected int computeVerticalScrollRange() {
        return _hDoc.getRowCount() * rowHeight() + getPaddingTop() + getPaddingBottom();
    }

    @Override
    public void computeScroll() {
        if (_scroller.computeScrollOffset()) {
            scrollTo(_scroller.getCurrX(), _scroller.getCurrY());
            postInvalidate();
        }
    }

    public final void smoothScrollBy(int dx, int dy) {
        if (getHeight() == 0) {
            // Nothing to do.
            return;
        }
        long duration = AnimationUtils.currentAnimationTimeMillis() - mLastScroll;
        if (duration > 250) {
            //final int maxY = getMaxScrollX();
            final int scrollY = getScrollY();
            final int scrollX = getScrollX();

            //dy = Math.max(0, Math.min(scrollY + dy, maxY)) - scrollY;

            _scroller.startScroll(scrollX, scrollY, dx, dy);
            postInvalidate();
        } else {
            if (!_scroller.isFinished()) {
                _scroller.abortAnimation();
            }
            scrollBy(dx, dy);
        }
        mLastScroll = AnimationUtils.currentAnimationTimeMillis();
    }

    /**
     * Like {@link #scrollTo}, but scroll smoothly instead of immediately.
     *
     * @param x the position where to scroll on the X axis
     * @param y the position where to scroll on the Y axis
     */
    public final void smoothScrollTo(int x, int y) {
        smoothScrollBy(x - getScrollX(), y - getScrollY());
    }

    /**
     * Start fling scrolling
     */
    void flingScroll(int velocityX, int velocityY) {

        _scroller.fling(getScrollX(), getScrollY(), velocityX, velocityY,
                0, getMaxScrollX(), 0, getMaxScrollY());
        // Keep on drawing until the animation has finished.
        postInvalidate();
        //postInvalidateOnAnimation();
    }

    public boolean isFlingScrolling() {
        return !_scroller.isFinished();
    }


    //---------------------------------------------------------------------
    //------------------------- Caret methods -----------------------------

    public void stopFlingScrolling() {
        _scroller.forceFinished(true);
    }

    /**
     * Starting scrolling continuously in scrollDir.
     * Not private to allow access by TouchNavigationMethod.
     *
     * @return True if auto-scrolling started
     */
    boolean autoScrollCaret(int scrollDir) {
        boolean scrolled = false;
        switch (scrollDir) {
            case SCROLL_UP:
                removeCallbacks(_scrollCaretUpTask);
                if ((!caretOnFirstRowOfFile())) {
                    post(_scrollCaretUpTask);
                    scrolled = true;
                }
                break;
            case SCROLL_DOWN:
                removeCallbacks(_scrollCaretDownTask);
                if (!caretOnLastRowOfFile()) {
                    post(_scrollCaretDownTask);
                    scrolled = true;
                }
                break;
            case SCROLL_LEFT:
                removeCallbacks(_scrollCaretLeftTask);
                if (_caretPosition > 0 &&
                        _caretRow == _hDoc.findRowNumber(_caretPosition - 1)) {
                    post(_scrollCaretLeftTask);
                    scrolled = true;
                }
                break;
            case SCROLL_RIGHT:
                removeCallbacks(_scrollCaretRightTask);
                if (!caretOnEOF() &&
                        _caretRow == _hDoc.findRowNumber(_caretPosition + 1)) {
                    post(_scrollCaretRightTask);
                    scrolled = true;
                }
                break;
            default:
                TextWarriorException.fail("Invalid scroll direction");
                break;
        }
        return scrolled;
    }

    /**
     * Stops automatic scrolling initiated by autoScrollCaret(int).
     * Not private to allow access by TouchNavigationMethod
     */
    void stopAutoScrollCaret() {
        removeCallbacks(_scrollCaretDownTask);
        removeCallbacks(_scrollCaretUpTask);
        removeCallbacks(_scrollCaretLeftTask);
        removeCallbacks(_scrollCaretRightTask);
    }

    /**
     * Stops automatic scrolling in scrollDir direction.
     * Not private to allow access by TouchNavigationMethod
     */
    void stopAutoScrollCaret(int scrollDir) {
        switch (scrollDir) {
            case SCROLL_UP:
                removeCallbacks(_scrollCaretUpTask);
                break;
            case SCROLL_DOWN:
                removeCallbacks(_scrollCaretDownTask);
                break;
            case SCROLL_LEFT:
                removeCallbacks(_scrollCaretLeftTask);
                break;
            case SCROLL_RIGHT:
                removeCallbacks(_scrollCaretRightTask);
                break;
            default:
                TextWarriorException.fail("Invalid scroll direction");
                break;
        }
    }

    public int getCaretRow() {
        return _caretRow;
    }

    public int getCaretPosition() {
        return _caretPosition;
    }

    /**
     * Sets the caret to position i, scrolls it to view and invalidates
     * the necessary areas for redrawing
     *
     * @param i The character index that the caret should be set to
     */
    public void moveCaret(int i) {
        _fieldController.moveCaret(i);
    }

    /**
     * Sets the caret one position back, scrolls it on screen, and invalidates
     * the necessary areas for redrawing.
     * <p>
     * If the caret is already on the first character, nothing will happen.
     */
    public void moveCaretLeft() {
        _fieldController.moveCaretLeft(false);
    }

    /**
     * Sets the caret one position forward, scrolls it on screen, and
     * invalidates the necessary areas for redrawing.
     * <p>
     * If the caret is already on the last character, nothing will happen.
     */
    public void moveCaretRight() {
        _fieldController.moveCaretRight(false);
    }

    /**
     * Sets the caret one row down, scrolls it on screen, and invalidates the
     * necessary areas for redrawing.
     * <p>
     * If the caret is already on the last row, nothing will happen.
     */
    public void moveCaretDown() {
        _fieldController.moveCaretDown();
    }

    /**
     * Sets the caret one row up, scrolls it on screen, and invalidates the
     * necessary areas for redrawing.
     * <p>
     * If the caret is already on the first row, nothing will happen.
     */
    public void moveCaretUp() {
        _fieldController.moveCaretUp();
    }

    /**
     * Scrolls the caret into view if it is not on screen
     */
    public void focusCaret() {
        makeCharVisible(_caretPosition);
    }


    //---------------------------------------------------------------------
    //------------------------- Text Selection ----------------------------

    /**
     * @return The column number where charOffset appears on
     */
    protected int getColumn(int charOffset) {
        int row = _hDoc.findRowNumber(charOffset);
        TextWarriorException.assertVerbose(row >= 0,
                "Invalid char offset given to getColumn");
        int firstCharOfRow = _hDoc.getRowOffset(row);
        return charOffset - firstCharOfRow;
    }

    protected boolean caretOnFirstRowOfFile() {
        return (_caretRow == 0);
    }

    protected boolean caretOnLastRowOfFile() {
        return (_caretRow == (_hDoc.getRowCount() - 1));
    }

    protected boolean caretOnEOF() {
        return (_caretPosition == (_hDoc.docLength() - 1));
    }

    public final boolean isSelectText() {
        return _fieldController.isSelectText();
    }

    public final boolean isSelectText2() {
        return _fieldController.isSelectText2();
    }

    /**
     * Enter or exit select mode.
     * Invalidates necessary areas for repainting.
     *
     * @param mode If true, enter select mode; else exit select mode
     */
    public void selectText(boolean mode) {
        if (_fieldController.isSelectText() && !mode) {
            invalidateSelectionRows();
            _fieldController.setSelectText(false);
        } else if (!_fieldController.isSelectText() && mode) {
            invalidateCaretRow();
            _fieldController.setSelectText(true);
        }
    }

    public void selectAll() {
        _fieldController.setSelectionRange(0, _hDoc.docLength() - 1, false, true);
    }

    public void setSelection(int beginPosition, int numChars) {
        _fieldController.setSelectionRange(beginPosition, numChars, true, false);
    }

    public void setSelectionRange(int beginPosition, int numChars) {
        _fieldController.setSelectionRange(beginPosition, numChars, true, true);
    }

    public boolean inSelectionRange(int charOffset) {
        return _fieldController.inSelectionRange(charOffset);
    }

    public int getSelectionStart() {
        if (_selectionAnchor < 0)
            return _caretPosition;
        else
            return _selectionAnchor;
    }

    public int getSelectionEnd() {
        if (_selectionEdge < 0)
            return _caretPosition;
        else
            return _selectionEdge;
    }

    public void focusSelectionStart() {
        _fieldController.focusSelection(true);
    }

    public void focusSelectionEnd() {
        _fieldController.focusSelection(false);
    }

    public void cut() {
        if (_selectionAnchor != _selectionEdge)
            _fieldController.cut(_clipboardManager);
    }

    public void copy() {
        if (_selectionAnchor != _selectionEdge)
            _fieldController.copy(_clipboardManager);
        selectText(false);
    }

    //---------------------------------------------------------------------
    //------------------------- Formatting methods ------------------------

    public void paste() {
        CharSequence text = _clipboardManager.getText();
        if (text != null)
            _fieldController.paste(text.toString());
    }

    public void cut(ClipboardManager cb) {
        _fieldController.cut(cb);
    }

    public void copy(ClipboardManager cb) {
        _fieldController.copy(cb);
    }

    public void paste(String text) {
        _fieldController.paste(text);
    }

    private boolean reachedNextSpan(int charIndex, Pair span) {
        return span != null && (charIndex == span.getFirst());
    }

    public void respan() {
        _fieldController.determineSpans();
    }

    public void cancelSpanning() {
        _fieldController.cancelSpanning();
    }

    /**
     * Sets the text to use the new typeface, scrolls the view to display the
     * caret if needed, and invalidates the entire view
     */
    public void setTypeface(Typeface typeface) {
        _defTypeface = typeface;
        _boldTypeface = Typeface.create(typeface, Typeface.BOLD);
        _italicTypeface = Typeface.create(typeface, Typeface.ITALIC);
        _brush.setTypeface(typeface);
        _brushLine.setTypeface(typeface);
        if (_hDoc.isWordWrap())
            _hDoc.analyzeWordWrap();
        _fieldController.updateCaretRow();
        if (!makeCharVisible(_caretPosition)) {
            invalidate();
        }
    }

    public void setItalicTypeface(Typeface typeface) {
        _italicTypeface = typeface;
    }

    public void setBoldTypeface(Typeface typeface) {
        _boldTypeface = typeface;
    }

    public boolean isWordWrap() {
        return _hDoc.isWordWrap();
    }

    public void setWordWrap(boolean enable) {
        _hDoc.setWordWrap(enable);

        if (enable) {
            _xExtent = 0;
            scrollTo(0, 0);
        }

        _fieldController.updateCaretRow();

        if (!makeCharVisible(_caretPosition)) {
            invalidate();
        }
    }

    public float getZoom() {
        return _zoomFactor;
    }

    /**
     * Sets the text size to be factor of the base text size, scrolls the view
     * to display the caret if needed, and invalidates the entire view
     */
    public void setZoom(float factor) {
        if (factor <= 0.5 || factor >= 5 || factor == _zoomFactor) {
            return;
        }
        _zoomFactor = factor;
        int newSize = (int) (factor * BASE_TEXT_SIZE_PIXELS);
        _brush.setTextSize(newSize);
        _brushLine.setTextSize(newSize);
        if (_hDoc.isWordWrap())
            _hDoc.analyzeWordWrap();
        _fieldController.updateCaretRow();
        _alphaWidth = (int) _brush.measureText("a");
        //if(!makeCharVisible(_caretPosition)){
        invalidate();
        //}
    }

    /**
     * Sets the length of a tab character, scrolls the view to display the
     * caret if needed, and invalidates the entire view
     *
     * @param spaceCount The number of spaces a tab represents
     */
    public void setTabSpaces(int spaceCount) {
        if (spaceCount < 0) {
            return;
        }

        _tabLength = spaceCount;
        if (_hDoc.isWordWrap())
            _hDoc.analyzeWordWrap();
        _fieldController.updateCaretRow();
        if (!makeCharVisible(_caretPosition)) {
            invalidate();
        }
    }

    /**
     * Enable/disable auto-indent
     */
    public void setAutoIndent(boolean enable) {
        _isAutoIndent = enable;
    }

    public void setAutoComplete(boolean enable) {
        _isAutoComplete = enable;
    }


    /**
     * Enable/disable long-pressing capitalization.
     * When enabled, a long-press on a hardware key capitalizes that letter.
     * When disabled, a long-press on a hardware key bring up the
     * CharacterPickerDialog, if there are alternative characters to choose from.
     */
    public void setLongPressCaps(boolean enable) {
        _isLongPressCaps = enable;
    }

    /**
     * Enable/disable highlighting of the current row. The current row is also
     * invalidated
     */
    public void setHighlightCurrentRow(boolean enable) {
        _isHighlightRow = enable;
        invalidateCaretRow();
    }

    /**
     * Enable/disable display of visible representations of non-printing
     * characters like spaces, tabs and end of lines
     * Invalidates the view if the enable state changes
     */
    public void setNonPrintingCharVisibility(boolean enable) {
        if (enable ^ _showNonPrinting) {
            _showNonPrinting = enable;
            if (_hDoc.isWordWrap())
                _hDoc.analyzeWordWrap();
            _fieldController.updateCaretRow();
            if (!makeCharVisible(_caretPosition)) {
                invalidate();
            }
        }
    }

    //---------------------------------------------------------------------
    //------------------------- Event handlers ----------------------------
    @Override
    public boolean onKeyPreIme(int keyCode, KeyEvent event) {
        //Intercept multiple key presses of printing characters to implement
        //long-press caps, because the IME may consume them and not pass the
        //event to onKeyDown() for long-press caps logic to work.
        //TODO Technically, long-press caps should be implemented in the IME,
        //but is put here for end-user's convenience. Unfortunately this may
        //cause some IMEs to break. Remove this feature in future.
        if (_isLongPressCaps
                && event.getRepeatCount() == 1
                && event.getAction() == KeyEvent.ACTION_DOWN) {

            char c = KeysInterpreter.keyEventToPrintableChar(event);
            if (Character.isLowerCase(c)
                    && c == Character.toLowerCase(_hDoc.charAt(_caretPosition - 1))) {
                _fieldController.onPrintableChar(Language.BACKSPACE);
                _fieldController.onPrintableChar(Character.toUpperCase(c));
                return true;
            }
        }

        return super.onKeyPreIme(keyCode, event);
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        // Let touch navigation method intercept key event first
        if (_navMethod.onKeyDown(keyCode, event)) {
            return true;
        }

        //check if direction or symbol key
        if (KeysInterpreter.isNavigationKey(event)) {
            handleNavigationKey(keyCode, event);
            return true;
        } else if (keyCode == KeyEvent.KEYCODE_SYM ||
                keyCode == KeyCharacterMap.PICKER_DIALOG_INPUT) {
            showCharacterPicker(
                    PICKER_SETS.get(KeyCharacterMap.PICKER_DIALOG_INPUT), false);
            return true;
        }

        //check if character is printable
        char c = KeysInterpreter.keyEventToPrintableChar(event);
        if (c == Language.NULL_CHAR) {
            return super.onKeyDown(keyCode, event);
        }

        int repeatCount = event.getRepeatCount();
        //handle multiple (held) key presses
        if (repeatCount == 1) {
            if (_isLongPressCaps) {
                handleLongPressCaps(c);
            } else {
                handleLongPressDialogDisplay(c);
            }
        } else if (repeatCount == 0
                || _isLongPressCaps && !Character.isLowerCase(c)
                || !_isLongPressCaps && PICKER_SETS.get(c) == null) {
            _fieldController.onPrintableChar(c);
        }

        return true;
    }

    private void handleNavigationKey(int keyCode, KeyEvent event) {
        if (event.isShiftPressed() && !isSelectText()) {
            invalidateCaretRow();
            _fieldController.setSelectText(true);
        } else if (!event.isShiftPressed() && isSelectText()) {
            invalidateSelectionRows();
            _fieldController.setSelectText(false);
        }

        switch (keyCode) {
            case KeyEvent.KEYCODE_DPAD_RIGHT:
                _fieldController.moveCaretRight(false);
                break;
            case KeyEvent.KEYCODE_DPAD_LEFT:
                _fieldController.moveCaretLeft(false);
                break;
            case KeyEvent.KEYCODE_DPAD_DOWN:
                _fieldController.moveCaretDown();
                break;
            case KeyEvent.KEYCODE_DPAD_UP:
                _fieldController.moveCaretUp();
                break;
            default:
                break;
        }
    }

    private void handleLongPressCaps(char c) {
        if (Character.isLowerCase(c)
                && c == _hDoc.charAt(_caretPosition - 1)) {
            _fieldController.onPrintableChar(Language.BACKSPACE);
            _fieldController.onPrintableChar(Character.toUpperCase(c));
        } else {
            _fieldController.onPrintableChar(c);
        }
    }

    //Precondition: If c is alphabetical, the character before the caret is
    //also c, which can be lower- or upper-case
    private void handleLongPressDialogDisplay(char c) {
        //workaround to get the appropriate caps mode to use
        boolean isCaps = Character.isUpperCase(_hDoc.charAt(_caretPosition - 1));
        char base = (isCaps) ? Character.toUpperCase(c) : c;

        String candidates = PICKER_SETS.get(base);
        if (candidates != null) {
            _fieldController.stopTextComposing();
            showCharacterPicker(candidates, true);
        } else {
            _fieldController.onPrintableChar(c);
        }
    }

    /**
     * @param candidates A string of characters to for the user to choose from
     * @param replace    If true, the character before the caret will be replaced
     *                   with the user-selected char. If false, the user-selected char will
     *                   be inserted at the caret position.
     */
    private void showCharacterPicker(String candidates, boolean replace) {
        final boolean shouldReplace = replace;
        final SpannableStringBuilder dummyString = new SpannableStringBuilder();
        Selection.setSelection(dummyString, 0);

        CharacterPickerDialog dialog = new CharacterPickerDialog(getContext(),
                this, dummyString, candidates, true);

        dialog.setOnDismissListener(new OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                if (dummyString.length() > 0) {
                    if (shouldReplace) {
                        _fieldController.onPrintableChar(Language.BACKSPACE);
                    }
                    _fieldController.onPrintableChar(dummyString.charAt(0));
                }
            }
        });
        dialog.show();
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        if (_navMethod.onKeyUp(keyCode, event)) {
            return true;
        }

        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onTrackballEvent(MotionEvent event) {
        // TODO Test on real device
        int deltaX = Math.round(event.getX());
        int deltaY = Math.round(event.getY());
        while (deltaX > 0) {
            _fieldController.moveCaretRight(false);
            --deltaX;
        }
        while (deltaX < 0) {
            _fieldController.moveCaretLeft(false);
            ++deltaX;
        }
        while (deltaY > 0) {
            _fieldController.moveCaretDown();
            --deltaY;
        }
        while (deltaY < 0) {
            _fieldController.moveCaretUp();
            ++deltaY;
        }
        return true;
    }

    private MotionEvent mMotionEvent;
    private float mX;
    private float mY;

    public boolean isAccessibilityEnabled() {
        return isAccessibilityEnabled;
    }

    @Override
    public boolean onHoverEvent(MotionEvent event) {
        if (isAccessibilityEnabled) {
            float x = event.getX();
            float y = event.getY();

            switch (event.getAction()) {
                case MotionEvent.ACTION_HOVER_ENTER:
                    mMotionEvent = event;
                    break;
                case MotionEvent.ACTION_HOVER_MOVE:
                    _navMethod.onScroll(mMotionEvent, event, mX - x, mY - y);
                    break;
                case MotionEvent.ACTION_HOVER_EXIT:
                    _navMethod.onUp(event);
                    break;
            }
            mX = x;
            mY = y;
        }
        return super.onHoverEvent(event);
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        if (isFocused()) {
            _navMethod.onTouchEvent(event);
        } else {
            if ((event.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP
                    && isPointInView((int) event.getX(), (int) event.getY())) {
                // somehow, the framework does not automatically change the focus
                // to this view when it is touched
                requestFocus();
            }
        }
        return true;
    }

    final private boolean isPointInView(int x, int y) {
        return (x >= 0 && x < getWidth() &&
                y >= 0 && y < getHeight());
    }

    @Override
    protected void onFocusChanged(boolean gainFocus, int direction, Rect previouslyFocusedRect) {
        super.onFocusChanged(gainFocus, direction, previouslyFocusedRect);
        invalidateCaretRow();
    }

    /**
     * Not public to allow access by {@link TouchNavigationMethod}
     */
    void showIME(boolean show) {
        InputMethodManager im = (InputMethodManager) getContext()
                .getSystemService(Context.INPUT_METHOD_SERVICE);
        if (show) {
            im.showSoftInput(this, 0);
        } else {
            im.hideSoftInputFromWindow(this.getWindowToken(), 0);
        }
    }

    /**
     * Some navigation methods use sensors or have states for their widgets.
     * They should be notified of application lifecycle events so they can
     * start/stop sensing and load/store their GUI state.
     */
    void onPause() {
        _navMethod.onPause();
    }

    void onResume() {
        _navMethod.onResume();
    }

    void onDestroy() {
        _fieldController.cancelSpanning();
    }

    //*********************************************************************
    //************************ Controller logic ***************************
    //*********************************************************************

    public Parcelable getUiState() {
        return new TextFieldUiState(this);
    }

    public void restoreUiState(Parcelable state) {
        TextFieldUiState uiState = (TextFieldUiState) state;
        final int caretPosition = uiState._caretPosition;
        // If the text field is in the process of being created, it may not
        // have its width and height set yet.
        // Therefore, post UI restoration tasks to run later.
        if (uiState._selectMode) {
            final int selStart = uiState._selectBegin;
            final int selEnd = uiState._selectEnd;

            post(new Runnable() {
                @Override
                public void run() {
                    setSelectionRange(selStart, selEnd - selStart);
                    if (caretPosition < selEnd) {
                        focusSelectionStart(); //caret at the end by default
                    }
                }
            });
        } else {
            post(new Runnable() {
                @Override
                public void run() {
                    moveCaret(caretPosition);
                }
            });
        }
    }

    //*********************************************************************
    //**************** UI State for saving and restoring ******************
    //*********************************************************************
//TODO change private
    public static class TextFieldUiState implements Parcelable {
        public static final Parcelable.Creator<TextFieldUiState> CREATOR
                = new Parcelable.Creator<TextFieldUiState>() {
            @Override
            public TextFieldUiState createFromParcel(Parcel in) {
                return new TextFieldUiState(in);
            }

            @Override
            public TextFieldUiState[] newArray(int size) {
                return new TextFieldUiState[size];
            }
        };
        final int _caretPosition;
        final int _scrollX;
        final int _scrollY;
        final boolean _selectMode;
        final int _selectBegin;
        final int _selectEnd;

        public TextFieldUiState(FreeScrollingTextField textField) {
            _caretPosition = textField.getCaretPosition();
            _scrollX = textField.getScrollX();
            _scrollY = textField.getScrollY();
            _selectMode = textField.isSelectText();
            _selectBegin = textField.getSelectionStart();
            _selectEnd = textField.getSelectionEnd();
        }

        private TextFieldUiState(Parcel in) {
            _caretPosition = in.readInt();
            _scrollX = in.readInt();
            _scrollY = in.readInt();
            _selectMode = in.readInt() != 0;
            _selectBegin = in.readInt();
            _selectEnd = in.readInt();
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel out, int flags) {
            out.writeInt(_caretPosition);
            out.writeInt(_scrollX);
            out.writeInt(_scrollY);
            out.writeInt(_selectMode ? 1 : 0);
            out.writeInt(_selectBegin);
            out.writeInt(_selectEnd);
        }

    }

    private class TextFieldController
            implements Lexer.LexCallback {
        private final Lexer _lexer = new Lexer(this);
        private boolean _isInSelectionMode = false;
        private boolean _isInSelectionMode2;

        /**
         * Analyze the text for programming language keywords and redraws the
         * text view when done. The global programming language used is set with
         * the static method Lexer.setLanguage(Language)
         * <p>
         * Does nothing if the Lexer language is not a programming language
         */
        public void determineSpans() {
            _lexer.tokenize(_hDoc);
        }

        public void cancelSpanning() {
            _lexer.cancelTokenize();
        }

        @Override
        //This is usually called from a non-UI thread
        public void lexDone(final List<Pair> results) {
            post(new Runnable() {
                @Override
                public void run() {
                    _hDoc.setSpans(results);
                    invalidate();
                }
            });
        }

        //- TextFieldController -----------------------------------------------
        //---------------------------- Key presses ----------------------------

        //TODO minimise invalidate calls from moveCaret(), insertion/deletion and word wrap
        public void onPrintableChar(char c) {

            // delete currently selected text, if any
            boolean selectionDeleted = false;
            if (_isInSelectionMode) {
                selectionDelete();
                selectionDeleted = true;
            }

            int originalRow = _caretRow;
            int originalOffset = _hDoc.getRowOffset(originalRow);

            switch (c) {
                case Language.BACKSPACE:
                    if (selectionDeleted) {
                        break;
                    }

                    if (_caretPosition > 0) {
                        _textLis.onDel(c + "", _caretPosition, 1);
                        _hDoc.deleteAt(_caretPosition - 1, System.nanoTime());
                        if (_hDoc.charAt(_caretPosition - 2) == 0xd83d || _hDoc.charAt(_caretPosition - 2) == 0xd83c) {
                            _hDoc.deleteAt(_caretPosition - 2, System.nanoTime());
                            moveCaretLeft(true);
                        }

                        moveCaretLeft(true);

                        if (_caretRow < originalRow) {
                            // either a newline was deleted or the caret was on the
                            // first word and it became short enough to fit the prev
                            // row
                            invalidateFromRow(_caretRow);
                        } else if (_hDoc.isWordWrap()) {
                            if (originalOffset != _hDoc.getRowOffset(originalRow)) {
                                //invalidate previous row too if its wrapping changed
                                --originalRow;
                            }
                            //TODO invalidate damaged rows only
                            invalidateFromRow(originalRow);
                        }
                    }
                    break;

                case Language.NEWLINE:
                    if (_isAutoIndent) {
                        char[] indent = createAutoIndent();
                        _hDoc.insertBefore(indent, _caretPosition, System.nanoTime());
                        moveCaret(_caretPosition + indent.length);
                    } else {
                        _hDoc.insertBefore(c, _caretPosition, System.nanoTime());
                        moveCaretRight(true);
                    }

                    if (_hDoc.isWordWrap() && originalOffset != _hDoc.getRowOffset(originalRow)) {
                        //invalidate previous row too if its wrapping changed
                        --originalRow;
                    }

                    _textLis.onNewLine(c + "", _caretPosition, 1);

                    invalidateFromRow(originalRow);
                    break;

                default:
                    _hDoc.insertBefore(c, _caretPosition, System.nanoTime());
                    moveCaretRight(true);
                    _textLis.onAdd(c + "", _caretPosition, 1);

                    if (_hDoc.isWordWrap()) {
                        if (originalOffset != _hDoc.getRowOffset(originalRow)) {
                            //invalidate previous row too if its wrapping changed
                            --originalRow;
                        }
                        //TODO invalidate damaged rows only
                        invalidateFromRow(originalRow);
                    }
                    break;
            }

            setEdited(true);
            determineSpans();
        }

        /**
         * Return a char[] with a newline as the 0th element followed by the
         * leading spaces and tabs of the line that the caret is on
         */
        private char[] createAutoIndent() {
            int lineNum = _hDoc.findLineNumber(_caretPosition);
            int startOfLine = _hDoc.getLineOffset(lineNum);
            int whitespaceCount = 0;
            _hDoc.seekChar(startOfLine);
            while (_hDoc.hasNext()) {
                char c = _hDoc.next();
                if ((c != ' ' && c != Language.TAB) || startOfLine + whitespaceCount >= _caretPosition) {
                    break;
                }
                ++whitespaceCount;
            }

            whitespaceCount += _autoIndentWidth * AutoIndent.createAutoIndent(_hDoc.subSequence(startOfLine, _caretPosition - startOfLine));
            if (whitespaceCount < 0)
                return new char[]{Language.NEWLINE};

            char[] indent = new char[1 + whitespaceCount];
            indent[0] = Language.NEWLINE;

            _hDoc.seekChar(startOfLine);
            for (int i = 0; i < whitespaceCount; ++i) {
                indent[1 + i] = ' ';
            }
            return indent;
        }

        public void moveCaretDown() {
            if (!caretOnLastRowOfFile()) {
                int currCaret = _caretPosition;
                int currRow = _caretRow;
                int newRow = currRow + 1;
                int currColumn = getColumn(currCaret);
                int currRowLength = _hDoc.getRowSize(currRow);
                int newRowLength = _hDoc.getRowSize(newRow);

                if (currColumn < newRowLength) {
                    // Position at the same column as old row.
                    _caretPosition += currRowLength;
                } else {
                    // Column does not exist in the new row (new row is too short).
                    // Position at end of new row instead.
                    _caretPosition +=
                            currRowLength - currColumn + newRowLength - 1;
                }
                ++_caretRow;

                updateSelectionRange(currCaret, _caretPosition);
                if (!makeCharVisible(_caretPosition)) {
                    invalidateRows(currRow, newRow + 1);
                }
                _rowLis.onRowChange(newRow);
                stopTextComposing();
            }
        }

        public void moveCaretUp() {
            if (!caretOnFirstRowOfFile()) {
                int currCaret = _caretPosition;
                int currRow = _caretRow;
                int newRow = currRow - 1;
                int currColumn = getColumn(currCaret);
                int newRowLength = _hDoc.getRowSize(newRow);

                if (currColumn < newRowLength) {
                    // Position at the same column as old row.
                    _caretPosition -= newRowLength;
                } else {
                    // Column does not exist in the new row (new row is too short).
                    // Position at end of new row instead.
                    _caretPosition -= (currColumn + 1);
                }
                --_caretRow;

                updateSelectionRange(currCaret, _caretPosition);
                if (!makeCharVisible(_caretPosition)) {
                    invalidateRows(newRow, currRow + 1);
                }
                _rowLis.onRowChange(newRow);
                stopTextComposing();
            }
        }

        /**
         * @param isTyping Whether caret is moved to a consecutive position as
         *                 a result of entering text
         */
        public void moveCaretRight(boolean isTyping) {
            if (!caretOnEOF()) {
                int originalRow = _caretRow;
                ++_caretPosition;
                updateCaretRow();
                updateSelectionRange(_caretPosition - 1, _caretPosition);
                if (!makeCharVisible(_caretPosition)) {
                    invalidateRows(originalRow, _caretRow + 1);
                }

                if (!isTyping) {
                    stopTextComposing();
                }
            }
        }

        /**
         * @param isTyping Whether caret is moved to a consecutive position as
         *                 a result of deleting text
         */
        public void moveCaretLeft(boolean isTyping) {
            if (_caretPosition > 0) {
                int originalRow = _caretRow;
                --_caretPosition;
                updateCaretRow();
                updateSelectionRange(_caretPosition + 1, _caretPosition);
                if (!makeCharVisible(_caretPosition)) {
                    invalidateRows(_caretRow, originalRow + 1);
                }

                if (!isTyping) {
                    stopTextComposing();
                }
            }
        }

        public void moveCaret(int i) {
            if (i < 0 || i >= _hDoc.docLength()) {
                TextWarriorException.fail("Invalid caret position");
                return;
            }
            updateSelectionRange(_caretPosition, i);

            _caretPosition = i;
            updateAfterCaretJump();
        }

        private void updateAfterCaretJump() {
            int oldRow = _caretRow;
            updateCaretRow();
            if (!makeCharVisible(_caretPosition)) {
                invalidateRows(oldRow, oldRow + 1); //old caret row
                invalidateCaretRow(); //new caret row
            }
            stopTextComposing();
        }


        /**
         * This helper method should only be used by internal methods after setting
         * _caretPosition, in order to to recalculate the new row the caret is on.
         */
        void updateCaretRow() {
            int newRow = _hDoc.findRowNumber(_caretPosition);
            if (_caretRow != newRow) {
                _caretRow = newRow;
                _rowLis.onRowChange(newRow);
            }
        }

        public void stopTextComposing() {
            InputMethodManager im = (InputMethodManager) getContext()
                    .getSystemService(Context.INPUT_METHOD_SERVICE);
            // This is an overkill way to inform the InputMethod that the caret
            // might have changed position and it should re-evaluate the
            // caps mode to use.
            im.restartInput(FreeScrollingTextField.this);
            if (_inputConnection != null && _inputConnection.isComposingStarted()) {
                _inputConnection.resetComposingState();
            }
        }

        //- TextFieldController -----------------------------------------------
        //-------------------------- Selection mode ---------------------------
        public final boolean isSelectText() {
            return _isInSelectionMode;
        }

        /**
         * Enter or exit select mode.
         * Does not invalidate view.
         *
         * @param mode If true, enter select mode; else exit select mode
         */
        public void setSelectText(boolean mode) {
            if (!(mode ^ _isInSelectionMode)) {
                return;
            }

            if (mode) {
                _selectionAnchor = _caretPosition;
                _selectionEdge = _caretPosition;
            } else {
                _selectionAnchor = -1;
                _selectionEdge = -1;
            }
            _isInSelectionMode = mode;
            _isInSelectionMode2 = mode;
            _selModeLis.onSelectionChanged(mode, getSelectionStart(), getSelectionEnd());
        }

        public final boolean isSelectText2() {
            return _isInSelectionMode2;
        }

        public boolean inSelectionRange(int charOffset) {
            if (_selectionAnchor < 0) {
                return false;
            }

            return (_selectionAnchor <= charOffset &&
                    charOffset < _selectionEdge);
        }

        /**
         * Selects numChars count of characters starting from beginPosition.
         * Invalidates necessary areas.
         *
         * @param beginPosition
         * @param numChars
         * @param scrollToStart If true, the start of the selection will be scrolled
         *                      into view. Otherwise, the end of the selection will be scrolled.
         */
        public void setSelectionRange(int beginPosition, int numChars,
                                      boolean scrollToStart, boolean mode) {
            TextWarriorException.assertVerbose(
                    (beginPosition >= 0) && numChars <= (_hDoc.docLength() - 1) && numChars >= 0,
                    "Invalid range to select");

            if (_isInSelectionMode) {
                // unhighlight previous selection
                invalidateSelectionRows();
            } else {
                // unhighlight caret
                invalidateCaretRow();
                if (mode)
                    setSelectText(true);
                else
                    _isInSelectionMode = true;
            }

            _selectionAnchor = beginPosition;
            _selectionEdge = _selectionAnchor + numChars;

            _caretPosition = _selectionEdge;
            stopTextComposing();
            updateCaretRow();
            if (mode)
                _selModeLis.onSelectionChanged(isSelectText(), _selectionAnchor, _selectionEdge);
            boolean scrolled = makeCharVisible(_selectionEdge);

            if (scrollToStart) {
                //TODO reduce unnecessary scrolling and write a method to scroll
                // the beginning of multi-line selections as far left as possible
                scrolled = makeCharVisible(_selectionAnchor);
            }

            if (!scrolled) {
                invalidateSelectionRows();
            }
        }

        /**
         * Moves the caret to an edge of selected text and scrolls it to view.
         *
         * @param start If true, moves the caret to the beginning of
         *              the selection. Otherwise, moves the caret to the end of the selection.
         *              In all cases, the caret is scrolled to view if it is not visible.
         */
        public void focusSelection(boolean start) {
            if (_isInSelectionMode) {
                if (start && _caretPosition != _selectionAnchor) {
                    _caretPosition = _selectionAnchor;
                    updateAfterCaretJump();
                } else if (!start && _caretPosition != _selectionEdge) {
                    _caretPosition = _selectionEdge;
                    updateAfterCaretJump();
                }
            }
        }


        /**
         * Used by internal methods to update selection boundaries when a new
         * caret position is set.
         * Does nothing if not in selection mode.
         */
        private void updateSelectionRange(int oldCaretPosition, int newCaretPosition) {
            if (isAccessibilityEnabled && Build.VERSION.SDK_INT >= 16) {
                AccessibilityRecord.obtain();
                AccessibilityEvent event = AccessibilityEvent.obtain(AccessibilityEvent.TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY);
                if ((oldCaretPosition - newCaretPosition) * (oldCaretPosition - newCaretPosition) == 1)
                    event.setMovementGranularity(AccessibilityNodeInfo.MOVEMENT_GRANULARITY_CHARACTER);
                if (oldCaretPosition > newCaretPosition)
                    event.setAction(ACTION_PREVIOUS_AT_MOVEMENT_GRANULARITY);
                else
                    event.setAction(ACTION_NEXT_AT_MOVEMENT_GRANULARITY);
                event.setFromIndex(Math.min(oldCaretPosition, newCaretPosition));
                event.setToIndex(Math.max(oldCaretPosition, newCaretPosition));
                sendAccessibilityEventUnchecked(event);
            }

            if (!_isInSelectionMode) {
                return;
            }

            if (oldCaretPosition < _selectionEdge) {
                if (newCaretPosition > _selectionEdge) {
                    _selectionAnchor = _selectionEdge;
                    _selectionEdge = newCaretPosition;
                } else {
                    _selectionAnchor = newCaretPosition;
                }

            } else {
                if (newCaretPosition < _selectionAnchor) {
                    _selectionEdge = _selectionAnchor;
                    _selectionAnchor = newCaretPosition;
                } else {
                    _selectionEdge = newCaretPosition;
                }
            }
        }


        //- TextFieldController -----------------------------------------------
        //------------------------ Cut, copy, paste ---------------------------

        /**
         * Convenience method for consecutive copy and paste calls
         */
        public void cut(ClipboardManager cb) {
            copy(cb);
            selectionDelete();
        }

        /**
         * Copies the selected text to the clipboard.
         * <p>
         * Does nothing if not in select mode.
         */
        public void copy(ClipboardManager cb) {
            //TODO catch OutOfMemoryError
            if (_isInSelectionMode &&
                    _selectionAnchor < _selectionEdge) {
                CharSequence contents = _hDoc.subSequence(_selectionAnchor,
                        _selectionEdge - _selectionAnchor);
                cb.setText(contents);
            }
        }

        /**
         * Inserts text at the caret position.
         * Existing selected text will be deleted and select mode will end.
         * The deleted area will be invalidated.
         * <p>
         * After insertion, the inserted area will be invalidated.
         */
        public void paste(String text) {
            if (text == null) {
                return;
            }

            _hDoc.beginBatchEdit();
            selectionDelete();

            int originalRow = _caretRow;
            int originalOffset = _hDoc.getRowOffset(originalRow);
            _hDoc.insertBefore(text.toCharArray(), _caretPosition, System.nanoTime());
            //_textLis.onAdd(text, _caretPosition, text.length());
            _hDoc.endBatchEdit();

            _caretPosition += text.length();
            updateCaretRow();

            setEdited(true);
            determineSpans();
            stopTextComposing();

            if (!makeCharVisible(_caretPosition)) {
                int invalidateStartRow = originalRow;
                //invalidate previous row too if its wrapping changed
                if (_hDoc.isWordWrap() &&
                        originalOffset != _hDoc.getRowOffset(originalRow)) {
                    --invalidateStartRow;
                }

                if (originalRow == _caretRow && !_hDoc.isWordWrap()) {
                    //pasted text only affects caret row
                    invalidateRows(invalidateStartRow, invalidateStartRow + 1);
                } else {
                    //TODO invalidate damaged rows only
                    invalidateFromRow(invalidateStartRow);
                }
            }
        }

        /**
         * Deletes selected text, exits select mode and invalidates deleted area.
         * If the selected range is empty, this method exits select mode and
         * invalidates the caret.
         * <p>
         * Does nothing if not in select mode.
         */
        public void selectionDelete() {
            if (!_isInSelectionMode) {
                return;
            }

            int totalChars = _selectionEdge - _selectionAnchor;

            if (totalChars > 0) {
                int originalRow = _hDoc.findRowNumber(_selectionAnchor);
                int originalOffset = _hDoc.getRowOffset(originalRow);
                boolean isSingleRowSel = _hDoc.findRowNumber(_selectionEdge) == originalRow;
                _textLis.onDel("", _caretPosition, totalChars);
                _hDoc.deleteAt(_selectionAnchor, totalChars, System.nanoTime());
                _caretPosition = _selectionAnchor;
                updateCaretRow();
                setEdited(true);
                determineSpans();
                setSelectText(false);
                stopTextComposing();

                if (!makeCharVisible(_caretPosition)) {
                    int invalidateStartRow = originalRow;
                    //invalidate previous row too if its wrapping changed
                    if (_hDoc.isWordWrap() &&
                            originalOffset != _hDoc.getRowOffset(originalRow)) {
                        --invalidateStartRow;
                    }

                    if (isSingleRowSel && !_hDoc.isWordWrap()) {
                        //pasted text only affects current row
                        invalidateRows(invalidateStartRow, invalidateStartRow + 1);
                    } else {
                        //TODO invalidate damaged rows only
                        invalidateFromRow(invalidateStartRow);
                    }
                }
            } else {
                setSelectText(false);
                invalidateCaretRow();
            }
        }

        void replaceText(int from, int charCount, String text) {
            int invalidateStartRow, originalOffset;
            boolean isInvalidateSingleRow = true;
            boolean dirty = false;
            //delete selection
            if (_isInSelectionMode) {
                invalidateStartRow = _hDoc.findRowNumber(_selectionAnchor);
                originalOffset = _hDoc.getRowOffset(invalidateStartRow);

                int totalChars = _selectionEdge - _selectionAnchor;

                if (totalChars > 0) {
                    _caretPosition = _selectionAnchor;
                    _hDoc.deleteAt(_selectionAnchor, totalChars, System.nanoTime());

                    if (invalidateStartRow != _caretRow) {
                        isInvalidateSingleRow = false;
                    }
                    dirty = true;
                }

                setSelectText(false);
            } else {
                invalidateStartRow = _caretRow;
                originalOffset = _hDoc.getRowOffset(_caretRow);
            }

            //delete requested chars
            if (charCount > 0) {
                int delFromRow = _hDoc.findRowNumber(from);
                if (delFromRow < invalidateStartRow) {
                    invalidateStartRow = delFromRow;
                    originalOffset = _hDoc.getRowOffset(delFromRow);
                }

                if (invalidateStartRow != _caretRow) {
                    isInvalidateSingleRow = false;
                }

                _caretPosition = from;
                _hDoc.deleteAt(from, charCount, System.nanoTime());
                dirty = true;
            }

            //insert
            if (text != null && text.length() > 0) {
                int insFromRow = _hDoc.findRowNumber(from);
                if (insFromRow < invalidateStartRow) {
                    invalidateStartRow = insFromRow;
                    originalOffset = _hDoc.getRowOffset(insFromRow);
                }

                _hDoc.insertBefore(text.toCharArray(), _caretPosition, System.nanoTime());
                _caretPosition += text.length();
                dirty = true;
            }

            if (dirty) {
                setEdited(true);
                determineSpans();
            }

            int originalRow = _caretRow;
            updateCaretRow();
            if (originalRow != _caretRow) {
                isInvalidateSingleRow = false;
            }

            if (!makeCharVisible(_caretPosition)) {
                //invalidate previous row too if its wrapping changed
                if (_hDoc.isWordWrap() &&
                        originalOffset != _hDoc.getRowOffset(invalidateStartRow)) {
                    --invalidateStartRow;
                }

                if (isInvalidateSingleRow && !_hDoc.isWordWrap()) {
                    //replaced text only affects current row
                    invalidateRows(_caretRow, _caretRow + 1);
                } else {
                    //TODO invalidate damaged rows only
                    invalidateFromRow(invalidateStartRow);
                }
            }
        }

        //- TextFieldController -----------------------------------------------
        //----------------- Helper methods for InputConnection ----------------

        /**
         * Deletes existing selected text, then deletes charCount number of
         * characters starting at from, and inserts text in its place.
         * <p>
         * Unlike paste or selectionDelete, does not signal the end of
         * text composing to the IME.
         */
        void replaceComposingText(int from, int charCount, String text) {
            int invalidateStartRow, originalOffset;
            boolean isInvalidateSingleRow = true;
            boolean dirty = false;

            //delete selection
            if (_isInSelectionMode) {
                invalidateStartRow = _hDoc.findRowNumber(_selectionAnchor);
                originalOffset = _hDoc.getRowOffset(invalidateStartRow);

                int totalChars = _selectionEdge - _selectionAnchor;

                if (totalChars > 0) {
                    _caretPosition = _selectionAnchor;
                    _hDoc.deleteAt(_selectionAnchor, totalChars, System.nanoTime());

                    if (invalidateStartRow != _caretRow) {
                        isInvalidateSingleRow = false;
                    }
                    dirty = true;
                }

                setSelectText(false);
            } else {
                invalidateStartRow = _caretRow;
                originalOffset = _hDoc.getRowOffset(_caretRow);
            }

            //delete requested chars
            if (charCount > 0) {
                int delFromRow = _hDoc.findRowNumber(from);
                if (delFromRow < invalidateStartRow) {
                    invalidateStartRow = delFromRow;
                    originalOffset = _hDoc.getRowOffset(delFromRow);
                }

                if (invalidateStartRow != _caretRow) {
                    isInvalidateSingleRow = false;
                }

                _caretPosition = from;
                _hDoc.deleteAt(from, charCount, System.nanoTime());
                dirty = true;
            }

            //insert
            if (text != null && text.length() > 0) {
                int insFromRow = _hDoc.findRowNumber(from);
                if (insFromRow < invalidateStartRow) {
                    invalidateStartRow = insFromRow;
                    originalOffset = _hDoc.getRowOffset(insFromRow);
                }

                _hDoc.insertBefore(text.toCharArray(), _caretPosition, System.nanoTime());
                _caretPosition += text.length();
                dirty = true;
            }

            _textLis.onAdd(text, _caretPosition, text.length() - charCount);
            if (dirty) {
                setEdited(true);
                determineSpans();
            }

            int originalRow = _caretRow;
            updateCaretRow();
            if (originalRow != _caretRow) {
                isInvalidateSingleRow = false;
            }

            if (!makeCharVisible(_caretPosition)) {
                //invalidate previous row too if its wrapping changed
                if (_hDoc.isWordWrap() &&
                        originalOffset != _hDoc.getRowOffset(invalidateStartRow)) {
                    --invalidateStartRow;
                }

                if (isInvalidateSingleRow && !_hDoc.isWordWrap()) {
                    //replaced text only affects current row
                    invalidateRows(_caretRow, _caretRow + 1);
                } else {
                    //TODO invalidate damaged rows only
                    invalidateFromRow(invalidateStartRow);
                }
            }
        }

        /**
         * Delete leftLength characters of text before the current caret
         * position, and delete rightLength characters of text after the current
         * cursor position.
         * <p>
         * Unlike paste or selectionDelete, does not signal the end of
         * text composing to the IME.
         */
        void deleteAroundComposingText(int left, int right) {
            int start = _caretPosition - left;
            if (start < 0) {
                start = 0;
            }
            int end = _caretPosition + right;
            int docLength = _hDoc.docLength();
            if (end > (docLength - 1)) { //exclude the terminal EOF
                end = docLength - 1;
            }
            replaceComposingText(start, end - start, "");
        }

        String getTextAfterCursor(int maxLen) {
            int docLength = _hDoc.docLength();
            if ((_caretPosition + maxLen) > (docLength - 1)) {
                //exclude the terminal EOF
                return _hDoc.subSequence(_caretPosition, docLength - _caretPosition - 1).toString();
            }

            return _hDoc.subSequence(_caretPosition, maxLen).toString();
        }

        String getTextBeforeCursor(int maxLen) {
            int start = _caretPosition - maxLen;
            if (start < 0) {
                start = 0;
            }
            return _hDoc.subSequence(start, _caretPosition - start).toString();
        }
    }//end inner controller class

    //*********************************************************************
    //************************** InputConnection **************************
    //*********************************************************************
    /*
     * Does not provide ExtractedText related methods
	 */
    private class TextFieldInputConnection extends BaseInputConnection {
        private boolean _isComposing = false;
        private int _composingCharCount = 0;

        public TextFieldInputConnection(FreeScrollingTextField v) {
            super(v, true);
        }

        public void resetComposingState() {
            _composingCharCount = 0;
            _isComposing = false;
            _hDoc.endBatchEdit();
        }

        @Override
        public boolean performContextMenuAction(int id) {
            switch (id) {
                case android.R.id.copy:
                    copy();
                    break;
                case android.R.id.cut:
                    cut();
                    break;
                case android.R.id.paste:
                    paste();
                    break;
                case android.R.id.startSelectingText:
                case android.R.id.stopSelectingText:
                case android.R.id.selectAll:
                    selectAll();
                    break;
            }

            return false;
        }


        @Override
        public boolean sendKeyEvent(KeyEvent event) {
            switch (event.getKeyCode()) {
                case KeyEvent.KEYCODE_SHIFT_LEFT:
                    if (isSelectText())
                        selectText(false);
                    else
                        selectText(true);
                    break;
                case KeyEvent.KEYCODE_DPAD_LEFT:
                    moveCaretLeft();
                    break;
                case KeyEvent.KEYCODE_DPAD_UP:
                    moveCaretUp();
                    break;
                case KeyEvent.KEYCODE_DPAD_RIGHT:
                    moveCaretRight();
                    break;
                case KeyEvent.KEYCODE_DPAD_DOWN:
                    moveCaretDown();
                    break;
                case KeyEvent.KEYCODE_MOVE_HOME:
                    moveCaret(0);
                    break;
                case KeyEvent.KEYCODE_MOVE_END:
                    moveCaret(_hDoc.length());
                    break;
                default:
                    return super.sendKeyEvent(event);
            }
            return true;
        }

        /**
         * Only true when the InputConnection has not been used by the IME yet.
         * Can be programatically cleared by resetComposingState()
         */
        public boolean isComposingStarted() {
            return _isComposing;
        }

        @Override
        public boolean setComposingText(CharSequence text, int newCursorPosition) {
            _isComposing = true;
            if (!_hDoc.isBatchEdit()) {
                _hDoc.beginBatchEdit();
            }

            _fieldController.replaceComposingText(
                    getCaretPosition() - _composingCharCount,
                    _composingCharCount,
                    text.toString());
            _composingCharCount = text.length();

            //TODO reduce invalidate calls
            if (newCursorPosition > 1) {
                _fieldController.moveCaret(_caretPosition + newCursorPosition - 1);
            } else if (newCursorPosition <= 0) {
                _fieldController.moveCaret(_caretPosition - text.length() - newCursorPosition);
            }
            return true;
        }

        @Override
        public boolean commitText(CharSequence text, int newCursorPosition) {
            _fieldController.replaceComposingText(
                    getCaretPosition() - _composingCharCount,
                    _composingCharCount,
                    text.toString());
            _composingCharCount = 0;
            _hDoc.endBatchEdit();

            //TODO reduce invalidate calls
            if (newCursorPosition > 1) {
                _fieldController.moveCaret(_caretPosition + newCursorPosition - 1);
            } else if (newCursorPosition <= 0) {
                _fieldController.moveCaret(_caretPosition - text.length() - newCursorPosition);
            }
            _isComposing = false;
            return true;
        }


        @Override
        public boolean deleteSurroundingText(int leftLength, int rightLength) {
            if (_composingCharCount != 0) {
                Log.i("lua",
                        "Warning: Implmentation of InputConnection.deleteSurroundingText" +
                                " will not skip composing text");
            }

            _fieldController.deleteAroundComposingText(leftLength, rightLength);
            return true;
        }

        @Override
        public boolean finishComposingText() {
            resetComposingState();
            return true;
        }

        @Override
        public int getCursorCapsMode(int reqModes) {
            int capsMode = 0;

            // Ignore InputType.TYPE_TEXT_FLAG_CAP_CHARACTERS; not used in TextWarrior

            if ((reqModes & InputType.TYPE_TEXT_FLAG_CAP_WORDS)
                    == InputType.TYPE_TEXT_FLAG_CAP_WORDS) {
                int prevChar = _caretPosition - 1;
                if (prevChar < 0 || Lexer.getLanguage().isWhitespace(_hDoc.charAt(prevChar))) {
                    capsMode |= InputType.TYPE_TEXT_FLAG_CAP_WORDS;

                    //set CAP_SENTENCES if client is interested in it
                    if ((reqModes & InputType.TYPE_TEXT_FLAG_CAP_SENTENCES)
                            == InputType.TYPE_TEXT_FLAG_CAP_SENTENCES) {
                        capsMode |= InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;
                    }
                }
            }

            // Strangely, Android soft keyboard does not set TYPE_TEXT_FLAG_CAP_SENTENCES
            // in reqModes even if it is interested in doing auto-capitalization.
            // Android bug? Therefore, we assume TYPE_TEXT_FLAG_CAP_SENTENCES
            // is always set to be on the safe side.
            else {
                Language lang = Lexer.getLanguage();

                int prevChar = _caretPosition - 1;
                int whitespaceCount = 0;
                boolean capsOn = true;

                // Turn on caps mode only for the first char of a sentence.
                // A fresh line is also considered to start a new sentence.
                // The position immediately after a period is considered lower-case.
                // Examples: "abc.com" but "abc. Com"
                while (prevChar >= 0) {
                    char c = _hDoc.charAt(prevChar);
                    if (c == Language.NEWLINE) {
                        break;
                    }

                    if (!lang.isWhitespace(c)) {
                        if (whitespaceCount == 0 || !lang.isSentenceTerminator(c)) {
                            capsOn = false;
                        }
                        break;
                    }

                    ++whitespaceCount;
                    --prevChar;
                }

                if (capsOn) {
                    capsMode |= InputType.TYPE_TEXT_FLAG_CAP_SENTENCES;
                }
            }

            return capsMode;
        }

        @Override
        public CharSequence getTextAfterCursor(int maxLen, int flags) {
            return _fieldController.getTextAfterCursor(maxLen); //ignore flags
        }

        @Override
        public CharSequence getTextBeforeCursor(int maxLen, int flags) {
            return _fieldController.getTextBeforeCursor(maxLen); //ignore flags
        }

        @Override
        public boolean setSelection(int start, int end) {
            if (start == end) {
                _fieldController.moveCaret(start);
            } else {
                _fieldController.setSelectionRange(start, end - start, false, true);
            }
            return true;
        }

        @Override
        public boolean reportFullscreenMode(boolean enabled) {
            return false;
        }
    }// end inner class
}
