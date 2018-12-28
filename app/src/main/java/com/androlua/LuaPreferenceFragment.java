package com.androlua;

import android.os.Bundle;
import android.preference.Preference;
import android.preference.PreferenceFragment;
import android.preference.PreferenceScreen;

import com.luajava.LuaException;
import com.luajava.LuaJavaAPI;
import com.luajava.LuaObject;
import com.luajava.LuaState;
import com.luajava.LuaTable;

import java.util.Set;

/**
 * Created by nirenr on 2018/08/05 0005.
 */

public class LuaPreferenceFragment extends PreferenceFragment implements Preference.OnPreferenceChangeListener, Preference.OnPreferenceClickListener {

    private LuaTable<Integer, LuaTable> mPreferences;
    private Preference.OnPreferenceChangeListener mOnPreferenceChangeListener;
    private Preference.OnPreferenceClickListener mOnPreferenceClickListener;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
            getPreferenceManager().setStorageDeviceProtected();
        }
        setPreferenceScreen(getPreferenceManager().createPreferenceScreen(getActivity()));
        //addPreferencesFromResource(R.xml.preference_screen);
        init(mPreferences);
    }

    public void setPreference(LuaTable<Integer,LuaTable> preferences){
        mPreferences=preferences;

    }

    private void init(LuaTable<Integer,LuaTable> preferences){
        PreferenceScreen ps = getPreferenceScreen();
        int len=preferences.length();
        LuaState L = preferences.getLuaState();
        for (int i = 1; i <= len; i++) {
            LuaTable p = preferences.get(i);
            try {
                LuaObject cls = p.getI(1);
                if(cls.isNil()){
                    throw new IllegalArgumentException("Fist value Must be a Class<Preference>, checked import package.");
                }
                Preference pf =  (Preference)cls.call(getActivity());
                Set<LuaTable.LuaEntry> set = p.entrySet();
                for(LuaTable.LuaEntry et:set){
                    Object key = et.getKey();
                    if(key instanceof String){
                        try {
                            LuaJavaAPI.javaSetter(L,pf,(String) key,et.getValue());
                        } catch (LuaException e) {
                            e.printStackTrace();
                        }
                    }
                }
                pf.setOnPreferenceChangeListener(this);
                pf.setOnPreferenceClickListener(this);
                ps.addPreference(pf);
            } catch (Exception e) {
                L.getContext().sendError("LuaPreferenceFragment",e);
            }
        }
    }

    public void  setOnPreferenceChangeListener(Preference.OnPreferenceChangeListener listener){
        mOnPreferenceChangeListener=listener;
    }

    public void  setOnPreferenceClickListener(Preference.OnPreferenceClickListener listener){
        mOnPreferenceClickListener=listener;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if(mOnPreferenceChangeListener!=null)
            return mOnPreferenceChangeListener.onPreferenceChange(preference, newValue);
        return true;
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if(mOnPreferenceClickListener!=null)
            return mOnPreferenceClickListener.onPreferenceClick(preference);
        return false;
    }
}
