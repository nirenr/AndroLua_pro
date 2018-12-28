package com.androlua;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.res.AssetFileDescriptor;
import android.content.res.AssetManager;
import android.content.res.ColorStateList;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.content.res.TypedArray;
import android.content.res.XmlResourceParser;
import android.graphics.Movie;
import android.graphics.Typeface;
import android.graphics.drawable.Drawable;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.util.Log;
import android.util.SparseArray;
import android.util.SparseBooleanArray;
import android.util.SparseIntArray;
import android.util.TypedValue;

import com.luajava.LuaException;
import com.luajava.LuaMetaTable;

import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;

/**
 * Created by Administrator on 2017/04/24 0024.
 */

@SuppressLint("UseSparseArrays")
public class LuaResources extends Resources implements LuaMetaTable {
    private static int mId = 0x7f050000;
    private final HashMap<Integer, String> mTextMap = new HashMap<>();
    private final HashMap<Integer, Drawable> mDrawableMap = new HashMap<>();
    private final HashMap<Integer, Integer> mColorMap = new HashMap();
    private final HashMap<Integer, String[]> mTextArrayMap = new HashMap<>();
    private final HashMap<Integer, int[]> mIntArrayMap = new HashMap<>();
    private final HashMap<Integer, Typeface> mTypefaceMap = new HashMap<>();
    private final HashMap<Integer, Integer> mIntMap = new HashMap();
    private final HashMap<Integer, Float> mFloatMap = new HashMap<>();
    private final HashMap<Integer, Boolean> mBooleanMap = new HashMap();

    private final HashMap<String, Integer> mIdMap = new HashMap<String, Integer>();
    private Resources mSuperResources;

    /**
     * Create a new Resources object on top of an existing set of assets in an
     * AssetManager.
     *
     * @param assets  Previously created AssetManager.
     * @param metrics Current display metrics to consider when
     *                selecting/computing resource values.
     * @param config  Desired device configuration to consider when
     */
    public LuaResources(AssetManager assets, DisplayMetrics metrics, Configuration config) {
        super(assets, metrics, config);
    }

    public void setText(int id, String text) {
        mTextMap.put(id, text);
    }

    public void setString(int id, String text) {
        mTextMap.put(id, text);
    }

    public void setTextArray(int id, String[] text) {
        mTextArrayMap.put(id, text);
    }

    public void setStringArray(int id, String[] text) {
        mTextArrayMap.put(id, text);
    }

    public void setIntArray(int id, int[] arr) {
        mIntArrayMap.put(id, arr);
    }

    public void setBoolean(int id, Boolean bool) {
        mBooleanMap.put(id, bool);
    }

    public void setDrawable(int id, Drawable drawable) {
        mDrawableMap.put(id, drawable);
    }

    public void setColor(int id, int color) {
        mColorMap.put(id, color);
    }

    public void setFont(int id, Typeface font) throws NotFoundException {
        mTypefaceMap.put(id, font);
    }

    @Override
    public CharSequence getText(int id) throws NotFoundException {
        String text = mTextMap.get(id);
        if (text != null)
            return text;
        return mSuperResources.getText(id);
    }

    @Override
    public CharSequence getText(int id, CharSequence def) {
        String text = mTextMap.get(id);
        if (text != null)
            return text;
        return mSuperResources.getText(id, def);
    }

    @Override
    public Drawable getDrawable(int id) throws NotFoundException {
        Drawable drawable = mDrawableMap.get(id);
        if (drawable != null)
            return drawable;
        return mSuperResources.getDrawable(id);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    @Override
    public Drawable getDrawable(int id, Theme theme) throws NotFoundException {
        Drawable drawable = mDrawableMap.get(id);
        if (drawable != null)
            return drawable;
        return mSuperResources.getDrawable(id, theme);
    }

    @Override
    public int getColor(int id) throws NotFoundException {
        Integer color = mColorMap.get(id);
        if (color != null)
            return color;
        return mSuperResources.getColor(id);
    }

    @Override
    public Object __call(Object... arg) throws LuaException {
        return null;
    }

    public int put(String key, Object value) {
        if (value == null)
            throw new NullPointerException();
        int id = mId++;
        if (value instanceof Drawable) {
            setDrawable(id, (Drawable) value);
        } else if (value instanceof String) {
            setText(id, (String) value);
        } else if (value instanceof String[]) {
            setTextArray(id, (String[]) value);
        } else if (value instanceof Number) {
            setColor(id, ((Number) value).intValue());
        } else if (value instanceof int[]) {
            setIntArray(id, ((int[]) value));
        } else {
            throw new IllegalArgumentException();
        }
        mIdMap.put(key, id);
        return id;
    }

    public Object get(String key) {
        return mIdMap.get(key);
    }

    @Override
    public Object __index(String key) {
        return get(key);
    }

    @Override
    public void __newIndex(String key, Object value) {
        put(key, value);
    }

    public void setSuperResources(Resources superRes) {
        mSuperResources = superRes;
    }

    @Override
    public boolean getBoolean(int id) throws NotFoundException {
        Boolean bool = mBooleanMap.get(id);
        if (bool != null)
            return bool;
        return mSuperResources.getBoolean(id);
    }

    @Override
    public CharSequence[] getTextArray(int id) throws NotFoundException {
        String[] text = mTextArrayMap.get(id);
        if (text != null)
            return text;
        return mSuperResources.getTextArray(id);
    }

    @TargetApi(Build.VERSION_CODES.M)
    @Override
    public ColorStateList getColorStateList(int id, Theme theme) throws NotFoundException {
        return mSuperResources.getColorStateList(id, theme);
    }

    @Override
    public Configuration getConfiguration() {
        return mSuperResources.getConfiguration();
    }

    @Override
    public DisplayMetrics getDisplayMetrics() {
        return mSuperResources.getDisplayMetrics();
    }


    @Override
    public float getDimension(int id) throws NotFoundException {
        return mSuperResources.getDimension(id);
    }

    @Override
    public float getFraction(int id, int base, int pbase) {
        return mSuperResources.getFraction(id, base, pbase);
    }

    @Override
    public int getDimensionPixelOffset(int id) throws NotFoundException {
        return mSuperResources.getDimensionPixelOffset(id);
    }

    @Override
    public int getDimensionPixelSize(int id) throws NotFoundException {
        return mSuperResources.getDimensionPixelSize(id);
    }

    @Override
    public int getIdentifier(String name, String defType, String defPackage) {
        return mSuperResources.getIdentifier(name, defType, defPackage);
    }

    @Override
    public int getInteger(int id) throws NotFoundException {
        Integer i = mIntMap.get(id);
        if (i != null)
            return i;
        return mSuperResources.getInteger(id);
    }

    @Override
    public ColorStateList getColorStateList(int id) throws NotFoundException {
        return mSuperResources.getColorStateList(id);
    }

    @Override
    public Drawable getDrawableForDensity(int id, int density) throws NotFoundException {
        return mSuperResources.getDrawableForDensity(id, density);
    }

    @TargetApi(Build.VERSION_CODES.M)
    @Override
    public int getColor(int id, Theme theme) throws NotFoundException {
        Integer color = mColorMap.get(id);
        if (color != null)
            return color;
        return mSuperResources.getColor(id, theme);
    }

    @Override
    public int[] getIntArray(int id) throws NotFoundException {
        int[] arr = mIntArrayMap.get(id);
        if (arr != null)
            return arr;
        return mSuperResources.getIntArray(id);
    }

    @Override
    public Movie getMovie(int id) throws NotFoundException {
        return mSuperResources.getMovie(id);
    }

    @Override
    public String getResourceEntryName(int resid) throws NotFoundException {
        return mSuperResources.getResourceEntryName(resid);
    }

    @Override
    public String getResourceName(int resid) throws NotFoundException {
        return mSuperResources.getResourceName(resid);
    }

    @Override
    public String getResourcePackageName(int resid) throws NotFoundException {
        return mSuperResources.getResourcePackageName(resid);
    }

    @Override
    public String getResourceTypeName(int resid) throws NotFoundException {
        return mSuperResources.getResourceTypeName(resid);
    }

    @Override
    public String getString(int id) throws NotFoundException {
        return getText(id).toString();
    }

    @Override
    public String getString(int id, Object... formatArgs) throws NotFoundException {
        return String.format(getString(id), formatArgs);
    }

    @Override
    public String[] getStringArray(int id) throws NotFoundException {
        String[] arr = mTextArrayMap.get(id);
        if (arr != null)
            return arr;
        return mSuperResources.getStringArray(id);
    }

    @TargetApi(Build.VERSION_CODES.O)
    @Override
    public Typeface getFont(int id) throws NotFoundException {
        Typeface font = mTypefaceMap.get(id);
        if (font != null)
            return font;
        return mSuperResources.getFont(id);
    }

    @Override
    public void getValue(int id, TypedValue outValue, boolean resolveRefs) throws NotFoundException {
        mSuperResources.getValue(id, outValue, resolveRefs);
    }

    @Override
    public void getValue(String name, TypedValue outValue, boolean resolveRefs) throws NotFoundException {
        mSuperResources.getValue(name, outValue, resolveRefs);
    }

    @Override
    public XmlResourceParser getAnimation(int id) throws NotFoundException {
        return mSuperResources.getAnimation(id);
    }

    @Override
    public void getValueForDensity(int id, int density, TypedValue outValue, boolean resolveRefs) throws NotFoundException {
        mSuperResources.getValueForDensity(id, density, outValue, resolveRefs);
    }

    @Override
    public XmlResourceParser getLayout(int id) throws NotFoundException {
        return mSuperResources.getLayout(id);
    }

    @Override
    public XmlResourceParser getXml(int id) throws NotFoundException {
        return mSuperResources.getXml(id);
    }

    @NonNull
    @Override
    public CharSequence getQuantityText(int id, int quantity) throws NotFoundException {
        return mSuperResources.getQuantityText(id, quantity);
    }

    @Override
    public Drawable getDrawableForDensity(int id, int density, @Nullable Theme theme) {
        return mSuperResources.getDrawableForDensity(id, density, theme);
    }

    @NonNull
    @Override
    public String getQuantityString(int id, int quantity) throws NotFoundException {
        return mSuperResources.getQuantityString(id, quantity);
    }

    @NonNull
    @Override
    public String getQuantityString(int id, int quantity, Object... formatArgs) throws NotFoundException {
        return mSuperResources.getQuantityString(id, quantity, formatArgs);
    }

    @Override
    public AssetFileDescriptor openRawResourceFd(int id) throws NotFoundException {
        return mSuperResources.openRawResourceFd(id);
    }

    @Override
    public InputStream openRawResource(int id) throws NotFoundException {
        return mSuperResources.openRawResource(id);
    }

    @Override
    public InputStream openRawResource(int id, TypedValue value) throws NotFoundException {
        return mSuperResources.openRawResource(id, value);
    }

    @Override
    public TypedArray obtainAttributes(AttributeSet set, int[] attrs) {
        return mSuperResources.obtainAttributes(set, attrs);
    }

    @NonNull
    @Override
    public TypedArray obtainTypedArray(int id) throws NotFoundException {
        return mSuperResources.obtainTypedArray(id);
    }

    @Override
    public void parseBundleExtra(String tagName, AttributeSet attrs, Bundle outBundle) throws XmlPullParserException {
        mSuperResources.parseBundleExtra(tagName, attrs, outBundle);
    }

    @Override
    public void parseBundleExtras(XmlResourceParser parser, Bundle outBundle) throws IOException, XmlPullParserException {
        mSuperResources.parseBundleExtras(parser, outBundle);
    }
}
