package com.androlua;

import com.luajava.*;

public class LuaPrint extends JavaFunction {

    private LuaState L;
    private LuaContext mLuaContext;
    private StringBuilder output = new StringBuilder();

    public LuaPrint(LuaContext luaContext, LuaState L) {
        super(L);
        this.L = L;
        mLuaContext = luaContext;
    }

    @Override
    public int execute() throws LuaException {
        if (L.getTop() < 2) {
            mLuaContext.sendMsg("");
            return 0;
        }
        for (int i = 2; i <= L.getTop(); i++) {
            int type = L.type(i);
            String val = null;
            String stype = L.typeName(type);
            if (stype.equals("userdata")) {
                Object obj = L.toJavaObject(i);
                if (obj != null)
                    val = obj.toString();
            } else if (stype.equals("boolean")) {
                val = L.toBoolean(i) ? "true" : "false";
            } else {
                val = L.toString(i);
            }
            if (val == null)
                val = stype;
            output.append("\t");
            output.append(val);
            output.append("\t");
        }
        mLuaContext.sendMsg(output.toString().substring(1, output.length() - 1));
        output.setLength(0);
        return 0;
    }


}

