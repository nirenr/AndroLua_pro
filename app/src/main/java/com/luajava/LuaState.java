/*
 * $Id: LuaState.java,v 1.9 2006/12/22 14:06:40 thiago Exp $
 * Copyright (C) 2003-2007 Kepler Project.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

package com.luajava;


import android.util.Log;

/**
 * LuaState if the main class of LuaJava for the Java developer.
 * LuaState is a mapping of most of Lua's C API functions.
 * LuaState also provides many other functions that will be used to manipulate
 * objects between Lua and Java.
 *
 * @author Thiago Ponte
 */
public class LuaState {

    private final static String LUAJAVA_LIB = "luajava";

    final public static int LUAI_MAXSTACK = 1000000;
    final public static int LUA_REGISTRYINDEX = -LUAI_MAXSTACK - 1000;

    final public static int LUA_RIDX_MAINTHREAD = 1;
    final public static int LUA_RIDX_GLOBALS = 2;
    final public static int LUA_RIDX_LAST = LUA_RIDX_GLOBALS;

    final public static int LUA_TNONE = -1;
    final public static int LUA_TNIL = 0;
    final public static int LUA_TBOOLEAN = 1;
    final public static int LUA_TLIGHTUSERDATA = 2;
    final public static int LUA_TNUMBER = 3;
    final public static int LUA_TSTRING = 4;
    final public static int LUA_TTABLE = 5;
    final public static int LUA_TFUNCTION = 6;
    final public static int LUA_TUSERDATA = 7;
    final public static int LUA_TTHREAD = 8;
    final public static int LUA_TINTEGER = 9;

    /**
     * Specifies that an unspecified (multiple) number of return arguments
     * will be returned by a call.
     */
    final public static int LUA_MULTRET = -1;


    /**
     *
     */
    final public static int LUA_YIELD = 1;
    /*
	 * error codes for `lua_load' and `lua_pcall'
	 */
    /**
     * a runtime error.
     */
    final public static int LUA_ERRRUN = 2;

    /**
     * syntax error during pre-compilation.
     */
    final public static int LUA_ERRSYNTAX = 3;

    /**
     * memory allocation error. For such errors, Lua does not call
     * the error handler function.
     */
    final public static int LUA_ERRMEM = 4;

    /**
     * error while running the error handler function.
     */
    final public static int LUA_ERRGCMM = 5;
    final public static int LUA_ERRERR = 6;

    final public static int LUA_OPEQ = 0;
    final public static int LUA_OPLT = 1;
    final public static int LUA_OPLE = 2;


    /**
     * Opens the library containing the luajava API
     */
    static {
        System.loadLibrary(LUAJAVA_LIB);
    }

    private long luaState;

    //private long stateId;

    protected LuaState() {
        luaState = _newstate();
        //openLuajava(stateId);
        //this.stateId = luaState.getPeer();
    }

    /**
     * Receives a existing state and initializes it
     *
     * @param luaState
     */
    protected LuaState(long luaState) {
        this.luaState = luaState;
        LuaStateFactory.insertLuaState(this);
        //openLuajava(stateId);
    }

    /**
     * Closes state and removes the object from the LuaStateFactory
     */
    public synchronized void close() {
        LuaStateFactory.removeLuaState(luaState);
        _close(luaState);
        this.luaState = 0;
    }

    @Override
    protected void finalize() {
        Log.i("luaState", "finalize: "+luaState);
        try {
           close();
        }
        catch (Exception e) {
            System.err.println("Unable to release luaState " + luaState);
        }
    }

    /**
     * Returns <code>true</code> if state is closed.
     */
    public synchronized boolean isClosed() {
        return luaState == 0;
    }

    /**
     * Return the long representing the LuaState pointer
     *
     * @return long
     */
    public long getPointer() {
        return luaState;
    }

    private com.androlua.LuaContext mContext;

    public void pushContext(com.androlua.LuaContext context) {
        mContext = context;
        pushString("_LuaContext");
        pushJavaObject(context);
        setTable(LUA_REGISTRYINDEX);
    }

    public com.androlua.LuaContext getContext() {
        return mContext;
    }

    /********************* Lua Native Interface *************************/

    private synchronized native long _newstate();

    private synchronized native void _close(long ptr);

    private synchronized native long _newthread(long ptr);

    // Stack manipulation
    private synchronized native int _getTop(long ptr);

    private synchronized native void _setTop(long ptr, int idx);

    private synchronized native void _pushValue(long ptr, int idx);

    private synchronized native void _rotate(long ptr, int idx, int n);

    private synchronized native void _copy(long ptr, int fromidx, int toidx);

    private synchronized native void _remove(long ptr, int idx);

    private synchronized native void _insert(long ptr, int idx);

    private synchronized native void _replace(long ptr, int idx);

    private synchronized native int _checkStack(long ptr, int sz);

    private synchronized native void _xmove(long from, long to, int n);

    // Access functions
    private synchronized native int _isNumber(long ptr, int idx);

    private synchronized native int _isInteger(long ptr, int idx);

    private synchronized native int _isString(long ptr, int idx);

    private synchronized native int _isCFunction(long ptr, int idx);

    private synchronized native int _isUserdata(long ptr, int idx);

    private synchronized native int _type(long ptr, int idx);

    private synchronized native String _typeName(long ptr, int tp);

    private synchronized native int _equal(long ptr, int idx1, int idx2);

    private synchronized native int _compare(long ptr, int idx1, int idx2, int op);

    private synchronized native int _rawequal(long ptr, int idx1, int idx2);

    private synchronized native int _lessThan(long ptr, int idx1, int idx2);

    private synchronized native double _toNumber(long ptr, int idx);

    private synchronized native long _toInteger(long ptr, int idx);

    private synchronized native int _toBoolean(long ptr, int idx);

    private synchronized native String _toString(long ptr, int idx);

    private synchronized native byte[] _toBuffer(long ptr, int idx);

    private synchronized native int _objlen(long ptr, int idx);

    private synchronized native int _rawlen(long ptr, int idx);

    private synchronized native long _toThread(long ptr, int idx);

    // Push functions
    private synchronized native void _pushNil(long ptr);

    private synchronized native void _pushNumber(long ptr, double number);

    private synchronized native void _pushInteger(long ptr, long integer);

    private synchronized native void _pushString(long ptr, String str);

    private synchronized native void _pushLString(long ptr, byte[] bytes, int n);

    private synchronized native void _pushBoolean(long ptr, int bool);

    // Get functions
    private synchronized native int _getTable(long ptr, int idx);

    private synchronized native int _getField(long ptr, int idx, String k);

    private synchronized native int _getI(long ptr, int idx, long n);

    private synchronized native int _rawGet(long ptr, int idx);

    private synchronized native int _rawGetI(long ptr, int idx, long n);

    private synchronized native void _createTable(long ptr, int narr, int nrec);

    private synchronized native int _getMetaTable(long ptr, int idx);

    private synchronized native int _getUserValue(long ptr, int idx);

    // Set functions
    private synchronized native void _setTable(long ptr, int idx);

    private synchronized native void _setField(long ptr, int idx, String k);

    private synchronized native void _setI(long ptr, int idx, long n);

    private synchronized native void _rawSet(long ptr, int idx);

    private synchronized native void _rawSetI(long ptr, int idx, long n);

    private synchronized native int _setMetaTable(long ptr, int idx);

    private synchronized native void _setUserValue(long ptr, int idx);

    private synchronized native void _call(long ptr, int nArgs, int nResults);

    private synchronized native int _pcall(long ptr, int nArgs, int Results, int errFunc);

    // Coroutine Functions
    private synchronized native int _yield(long ptr, int nResults);

    private synchronized native int _resume(long ptr, long from, int nargs);

    private synchronized native int _status(long ptr);

    private synchronized native int _isYieldable(long ptr);

    // Gargabe Collection Functions
    final public static int LUA_GCSTOP = 0;
    final public static int LUA_GCRESTART = 1;
    final public static int LUA_GCCOLLECT = 2;
    final public static int LUA_GCCOUNT = 3;
    final public static int LUA_GCCOUNTB = 4;
    final public static int LUA_GCSTEP = 5;
    final public static int LUA_GCSETPAUSE = 6;
    final public static int LUA_GCSETSTEPMUL = 7;

    private synchronized native int _gc(long ptr, int what, int data);

    // Miscellaneous Functions
    private synchronized native int _error(long ptr);

    private synchronized native int _next(long ptr, int idx);

    private synchronized native void _concat(long ptr, int n);

    // Some macros
    private synchronized native void _pop(long ptr, int n);

    private synchronized native void _newTable(long ptr);

    private synchronized native int _strlen(long ptr, int idx);

    private synchronized native int _isFunction(long ptr, int idx);

    private synchronized native int _isTable(long ptr, int idx);

    private synchronized native int _isNil(long ptr, int idx);

    private synchronized native int _isBoolean(long ptr, int idx);

    private synchronized native int _isThread(long ptr, int idx);

    private synchronized native int _isNone(long ptr, int idx);

    private synchronized native int _isNoneOrNil(long ptr, int idx);

    private synchronized native void _pushGlobalTable(long ptr);

    private synchronized native void _setGlobal(long ptr, String name);

    private synchronized native int _getGlobal(long ptr, String name);


    // LuaLibAux
    private synchronized native int _LdoFile(long ptr, String fileName);

    private synchronized native int _LdoString(long ptr, String string);
    //private synchronized native int _doBuffer(long ptr, byte[] buff, long sz, String n);

    private synchronized native int _LgetMetaField(long ptr, int obj, String e);

    private synchronized native int _LcallMeta(long ptr, int obj, String e);

    private synchronized native int _LargError(long ptr, int numArg, String extraMsg);

    private synchronized native String _LcheckString(long ptr, int numArg);

    private synchronized native String _LoptString(long ptr, int numArg, String def);

    private synchronized native double _LcheckNumber(long ptr, int numArg);

    private synchronized native double _LoptNumber(long ptr, int numArg, double def);

    private synchronized native int _LcheckInteger(long ptr, int numArg);

    private synchronized native int _LoptInteger(long ptr, int numArg, int def);

    private synchronized native void _LcheckStack(long ptr, int sz, String msg);

    private synchronized native void _LcheckType(long ptr, int nArg, int t);

    private synchronized native void _LcheckAny(long ptr, int nArg);

    private synchronized native int _LnewMetatable(long ptr, String tName);

    private synchronized native void _LgetMetatable(long ptr, String tName);

    private synchronized native void _Lwhere(long ptr, int lvl);

    private synchronized native int _Lref(long ptr, int t);

    private synchronized native void _LunRef(long ptr, int t, int ref);

    private synchronized native int _LloadFile(long ptr, String fileName);

    private synchronized native int _LloadBuffer(long ptr, byte[] buff, long sz, String name);

    private synchronized native int _LloadString(long ptr, String s);

    private synchronized native String _Lgsub(long ptr, String s, String p, String r);

    private synchronized native String _getUpValue(long ptr, int funcindex, int n);

    private synchronized native String _setUpValue(long ptr, int funcindex, int n);

    private synchronized native byte[] _dump(long ptr, int funcindex);

    private synchronized native void _openBase(long ptr);

    private synchronized native void _openTable(long ptr);

    private synchronized native void _openIo(long ptr);

    private synchronized native void _openOs(long ptr);

    private synchronized native void _openString(long ptr);

    private synchronized native void _openMath(long ptr);

    private synchronized native void _openDebug(long ptr);

    private synchronized native void _openPackage(long ptr);

    private synchronized native void _openLibs(long ptr);

    // Java Interface -----------------------------------------------------

    public LuaState newThread() {
        LuaState l = new LuaState(_newthread(luaState));
        LuaStateFactory.insertLuaState(l);
        return l;
    }

    // STACK MANIPULATION

    public int getTop() {
        return _getTop(luaState);
    }

    public void setTop(int idx) {
        _setTop(luaState, idx);
    }

    public void pushValue(int idx) {
        _pushValue(luaState, idx);
    }

    public void rotate(int idx, int n) {
        _rotate(luaState, idx, n);
    }

    public void copy(int fromidx, int toidx) {
        _copy(luaState, fromidx, toidx);
    }

    public void remove(int idx) {
        _remove(luaState, idx);
    }

    public void insert(int idx) {
        _insert(luaState, idx);
    }

    public void replace(int idx) {
        _replace(luaState, idx);
    }

    public int checkStack(int sz) {
        return _checkStack(luaState, sz);
    }

    public void xmove(LuaState to, int n) {
        _xmove(luaState, to.luaState, n);
    }

    // ACCESS FUNCTION

    public boolean isNumber(int idx) {
        return (_isNumber(luaState, idx) != 0);
    }

    public boolean isInteger(int idx) {
        return (_isInteger(luaState, idx) != 0);
    }

    public boolean isString(int idx) {
        return (_isString(luaState, idx) != 0);
    }

    public boolean isFunction(int idx) {
        return (_isFunction(luaState, idx) != 0);
    }

    public boolean isCFunction(int idx) {
        return (_isCFunction(luaState, idx) != 0);
    }

    public boolean isUserdata(int idx) {
        return (_isUserdata(luaState, idx) != 0);
    }

    public boolean isTable(int idx) {
        return (_isTable(luaState, idx) != 0);
    }

    public boolean isBoolean(int idx) {
        return (_isBoolean(luaState, idx) != 0);
    }

    public boolean isNil(int idx) {
        return (_isNil(luaState, idx) != 0);
    }

    public boolean isThread(int idx) {
        return (_isThread(luaState, idx) != 0);
    }

    public boolean isNone(int idx) {
        return (_isNone(luaState, idx) != 0);
    }

    public boolean isNoneOrNil(int idx) {
        return (_isNoneOrNil(luaState, idx) != 0);
    }

    public int type(int idx) {
        return _type(luaState, idx);
    }

    public String typeName(int tp) {
        return _typeName(luaState, tp);
    }

    public int equal(int idx1, int idx2) {
        return _equal(luaState, idx1, idx2);
    }

    public int compare(int idx1, int idx2, int op) {
        return _compare(luaState, idx1, idx2, op);
    }

    public int rawequal(int idx1, int idx2) {
        return _rawequal(luaState, idx1, idx2);
    }

    public int lessThan(int idx1, int idx2) {
        return _lessThan(luaState, idx1, idx2);
    }

    public double toNumber(int idx) {
        return _toNumber(luaState, idx);
    }

    public long toInteger(int idx) {
        return _toInteger(luaState, idx);
    }

    public boolean toBoolean(int idx) {
        return (_toBoolean(luaState, idx) != 0);
    }

    public String toString(int idx) {
        return _toString(luaState, idx);
    }

    public byte[] toBuffer(int idx) {
        return _toBuffer(luaState, idx);
    }

    public int strLen(int idx) {
        return _strlen(luaState, idx);
    }

    public int objLen(int idx) {
        return _objlen(luaState, idx);
    }

    public int rawLen(int idx) {
        return _rawlen(luaState, idx);
    }

    public LuaState toThread(int idx) {
        return new LuaState(_toThread(luaState, idx));
    }

    //PUSH FUNCTIONS

    public void pushNil() {
        _pushNil(luaState);
    }

    public void pushNumber(double db) {
        _pushNumber(luaState, db);
    }

    public void pushInteger(long integer) {
        _pushInteger(luaState, integer);
    }

    public void pushString(String str) {
        if (str == null)
            _pushNil(luaState);
        else
            _pushString(luaState, str);
    }

    public void pushString(byte[] bytes) {
        if (bytes == null)
            _pushNil(luaState);
        else
            _pushLString(luaState, bytes, bytes.length);
    }

    public void pushBoolean(boolean bool) {
        _pushBoolean(luaState, bool ? 1 : 0);
    }

    // GET FUNCTIONS

    public int getTable(int idx) {
        return _getTable(luaState, idx);
    }

    public int getField(int idx, String k) {
        return _getField(luaState, idx, k);
    }

    public int getI(int idx, long n) {
        return _getI(luaState, idx, n);
    }

    public int rawGet(int idx) {
        return _rawGet(luaState, idx);
    }

    public int rawGetI(int idx, long n) {
        return _rawGetI(luaState, idx, n);
    }

    public void createTable(int narr, int nrec) {
        _createTable(luaState, narr, nrec);
    }

    public void newTable() {
        _newTable(luaState);
    }

    // if returns 0, there is no metatable
    public int getMetaTable(int idx) {
        return _getMetaTable(luaState, idx);
    }

    public int getUserValue(int idx) {
        return _getUserValue(luaState, idx);
    }

    // SET FUNCTIONS

    public void setTable(int idx) {
        _setTable(luaState, idx);
    }

    public void setField(int idx, String k) {
        _setField(luaState, idx, k);
    }

    public void setI(int idx, long n) {
        _setI(luaState, idx, n);
    }

    public void rawSet(int idx) {
        _rawSet(luaState, idx);
    }

    public void rawSetI(int idx, long n) {
        _rawSetI(luaState, idx, n);
    }

    // if returns 0, cannot set the metatable to the given object
    public int setMetaTable(int idx) {
        return _setMetaTable(luaState, idx);
    }

    public void setUserValue(int idx) {
        _setUserValue(luaState, idx);
    }

    public void call(int nArgs, int nResults) {
        _call(luaState, nArgs, nResults);
    }

    // returns 0 if ok of one of the error codes defined
    public int pcall(int nArgs, int nResults, int errFunc) {
        return _pcall(luaState, nArgs, nResults, errFunc);
    }

    public int yield(int nResults) {
        return _yield(luaState, nResults);
    }

    public int resume(LuaState from, int nArgs) {
        return _resume(luaState, from.getPointer(), nArgs);
    }

    public int status() {
        return _status(luaState);
    }

    public int isYieldable() {
        return _isYieldable(luaState);
    }

    public int gc(int what, int data) {
        return _gc(luaState, what, data);
    }


    public int next(int idx) {
        return _next(luaState, idx);
    }

    public int error() {
        return _error(luaState);
    }

    public void concat(int n) {
        _concat(luaState, n);
    }


    // FUNCTION FROM lauxlib
    // returns 0 if ok
    public int LdoFile(String fileName) {
        return _LdoFile(luaState, fileName);
    }

    // returns 0 if ok
    public int LdoString(String str) {
        return _LdoString(luaState, str);
    }

    public int LgetMetaField(int obj, String e) {
        return _LgetMetaField(luaState, obj, e);
    }

    public int LcallMeta(int obj, String e) {
        return _LcallMeta(luaState, obj, e);
    }


    public int LargError(int numArg, String extraMsg) {
        return _LargError(luaState, numArg, extraMsg);
    }

    public String LcheckString(int numArg) {
        return _LcheckString(luaState, numArg);
    }

    public String LoptString(int numArg, String def) {
        return _LoptString(luaState, numArg, def);
    }

    public double LcheckNumber(int numArg) {
        return _LcheckNumber(luaState, numArg);
    }

    public double LoptNumber(int numArg, double def) {
        return _LoptNumber(luaState, numArg, def);
    }

    public int LcheckInteger(int numArg) {
        return _LcheckInteger(luaState, numArg);
    }

    public int LoptInteger(int numArg, int def) {
        return _LoptInteger(luaState, numArg, def);
    }

    public void LcheckStack(int sz, String msg) {
        _LcheckStack(luaState, sz, msg);
    }

    public void LcheckType(int nArg, int t) {
        _LcheckType(luaState, nArg, t);
    }

    public void LcheckAny(int nArg) {
        _LcheckAny(luaState, nArg);
    }

    public int LnewMetatable(String tName) {
        return _LnewMetatable(luaState, tName);
    }

    public void LgetMetatable(String tName) {
        _LgetMetatable(luaState, tName);
    }

    public void Lwhere(int lvl) {
        _Lwhere(luaState, lvl);
    }

    public int Lref(int t) {
        return _Lref(luaState, t);
    }

    public void LunRef(int t, int ref) {
        _LunRef(luaState, t, ref);
    }

    public int LloadFile(String fileName) {
        return _LloadFile(luaState, fileName);
    }

    public int LloadString(String s) {
        return _LloadString(luaState, s);
    }

    public int LloadBuffer(byte[] buff, String name) {
        return _LloadBuffer(luaState, buff, buff.length, name);
    }

    public String Lgsub(String s, String p, String r) {
        return _Lgsub(luaState, s, p, r);
    }

    public String getUpValue(int funcindex, int n) {
        return _getUpValue(luaState, funcindex, n);
    }

    public String setUpValue(int funcindex, int n) {
        return _setUpValue(luaState, funcindex, n);
    }

    public byte[] dump(int funcindex) {
        return _dump(luaState, funcindex);
    }

    //IMPLEMENTED C MACROS

    public void pop(int n) {
        //setTop(- (n) - 1);
        _pop(luaState, n);
    }


    public synchronized void pushGlobalTable() {
        _pushGlobalTable(luaState);
    }

    public synchronized int getGlobal(String global) {
        return _getGlobal(luaState, global);
    }

    public synchronized void setGlobal(String name) {
        _setGlobal(luaState, name);
    }

    // Functions to open lua libraries
    public void openBase() {
        _openBase(luaState);
    }

    public void openTable() {
        _openTable(luaState);
    }

    public void openIo() {
        _openIo(luaState);
    }

    public void openOs() {
        _openOs(luaState);
    }

    public void openString() {
        _openString(luaState);
    }

    public void openMath() {
        _openMath(luaState);
    }

    public void openDebug() {
        _openDebug(luaState);
    }

    public void openPackage() {
        _openPackage(luaState);
    }

    public void openLibs() {
        _openLibs(luaState);
        _openLuajava(luaState);
        pushPrimitive();
    }


    /********************** Luajava API Library **********************/

    /**
     * Initializes lua State to be used by luajava
     *
     * @param stateId
     */
    private synchronized native void _openLuajava(long stateId);

    /**
     * Gets a Object from a userdata
     *
     * @param L
     * @param idx index of the lua stack
     * @return Object
     */
    private synchronized native Object _getObjectFromUserdata(long L, int idx) throws LuaException;

    /**
     * Returns whether a userdata contains a Java Object
     *
     * @param L
     * @param idx index of the lua stack
     * @return boolean
     */
    private synchronized native boolean _isObject(long L, int idx);

    /**
     * Pushes a Java Object into the state stack
     *
     * @param L
     * @param obj
     */
    private synchronized native void _pushJavaObject(long L, Object obj);

    /**
     * Pushes a JavaFunction into the state stack
     *
     * @param L
     * @param func
     */
    private synchronized native void _pushJavaFunction(long L, JavaFunction func) throws LuaException;

    /**
     * Returns whether a userdata contains a Java Function
     *
     * @param L
     * @param idx index of the lua stack
     * @return boolean
     */
    private synchronized native boolean _isJavaFunction(long L, int idx);

    public void openLuajava() {
        _openLuajava(luaState);
        pushPrimitive();
    }

    /**
     * Gets a Object from Lua
     *
     * @param idx index of the lua stack
     * @return Object
     * @throws LuaException if the lua object does not represent a java object.
     */
    public Object getObjectFromUserdata(int idx) throws LuaException {
        return _getObjectFromUserdata(luaState, idx);
    }

    /**
     * Tells whether a lua index contains a java Object
     *
     * @param idx index of the lua stack
     * @return boolean
     */
    public boolean isObject(int idx) {
        return _isObject(luaState, idx);
    }

    /**
     * Pushes a Java Object into the lua stack.<br>
     * This function does not check if the object is from a class that could
     * be represented by a lua type. Eg: java.lang.String could be a lua string.
     *
     * @param obj Object to be pushed into lua
     */
    public void pushJavaObject(Object obj) {
        LuaJavaAPI.pushJavaObject();
        _pushJavaObject(luaState, obj);
    }

    /**
     * Pushes a JavaFunction into the state stack
     *
     * @param func
     */
    public void pushJavaFunction(JavaFunction func) throws LuaException {
        _pushJavaFunction(luaState, func);
    }

    /**
     * Returns whether a userdata contains a Java Function
     *
     * @param idx index of the lua stack
     * @return boolean
     */
    public boolean isJavaFunction(int idx) {
        return _isJavaFunction(luaState, idx);
    }

    /**
     * Pushes into the stack any object value.<br>
     * This function checks if the object could be pushed as a lua type, if not
     * pushes the java object.
     *
     * @param obj
     */
    public void pushObjectValue(Object obj) throws LuaException {
        if (obj == null) {
            pushNil();
        } else if (obj instanceof Boolean) {
            Boolean bool = (Boolean) obj;
            pushBoolean(bool);
        } else if (obj instanceof Long) {
            pushInteger((Long) obj);
        } else if (obj instanceof Integer) {
            pushInteger((Integer) obj);
        } else if (obj instanceof Short) {
            pushInteger((Short) obj);
        } else if (obj instanceof Character) {
            pushInteger((Character) obj);
        } else if (obj instanceof Byte) {
            pushInteger((Byte) obj);
        } else if (obj instanceof Float) {
            pushNumber((Float) obj);
        } else if (obj instanceof Double) {
            pushNumber((Double) obj);
        } else if (obj instanceof String) {
            pushString((String) obj);
        } else if (obj instanceof LuaString) {
            pushString(((LuaString) obj).toByteArray());
        } else if (obj instanceof JavaFunction) {
            JavaFunction func = (JavaFunction) obj;
            pushJavaFunction(func);
        } else if (obj instanceof LuaObject) {
            LuaObject ref = (LuaObject) obj;
            if (ref.getLuaState() == this)
                ref.push();
            else
                pushJavaObject(ref);
        } else {
            pushJavaObject(obj);
        }
    }

    /**
     * Function that returns a Java Object equivalent to the one in the given
     * position of the Lua Stack.
     *
     * @param idx Index in the Lua Stack
     * @return Java object equivalent to the Lua one
     */
    public synchronized Object toJavaObject(int idx) throws LuaException {
        Object obj = null;

        if (isBoolean(idx)) {
            obj = toBoolean(idx);
        } else if (type(idx) == LuaState.LUA_TSTRING) {
            obj = toString(idx);
        } else if (isFunction(idx)) {
            obj = getLuaObject(idx).getFunction();
        } else if (isTable(idx)) {
            obj = getLuaObject(idx).getTable();
        } else if (type(idx) == LuaState.LUA_TNUMBER) {
            if (isInteger(idx))
                obj = toInteger(idx);
            else
                obj = toNumber(idx);
        } else if (isUserdata(idx)) {
            if (isObject(idx)) {
                obj = getObjectFromUserdata(idx);
            } else {
                obj = getLuaObject(idx);
            }
        } else if (isNil(idx)) {
            obj = null;
        }

        return obj;
    }

    /**
     * Creates a reference to an object in the variable globalName
     *
     * @param globalName
     * @return LuaObject
     */
    public LuaObject getLuaObject(String globalName) {
        pushGlobalTable();
        pushString(globalName);
        rawGet(-2);
        LuaObject obj = getLuaObject(-1);
        pop(2);
        return obj;
        //return new LuaObject(this, globalName);
    }

    /**
     * Creates a reference to an object inside another object
     *
     * @param parent The Lua Table or Userdata that contains the Field.
     * @param name   The name that index the field
     * @return LuaObject
     * @throws LuaException if parent is not a table or userdata
     */
    public LuaObject getLuaObject(LuaObject parent, String name)
            throws LuaException {
        return new LuaObject(parent, name);
    }

    /**
     * This constructor creates a LuaObject from a table that is indexed by a number.
     *
     * @param parent The Lua Table or Userdata that contains the Field.
     * @param name   The name (number) that index the field
     * @return LuaObject
     * @throws LuaException When the parent object isn't a Table or Userdata
     */
    public LuaObject getLuaObject(LuaObject parent, Number name)
            throws LuaException {
        return new LuaObject(parent, name);
    }

    /**
     * This constructor creates a LuaObject from a table that is indexed by any LuaObject.
     *
     * @param parent The Lua Table or Userdata that contains the Field.
     * @param name   The name (LuaObject) that index the field
     * @return LuaObject
     * @throws LuaException When the parent object isn't a Table or Userdata
     */
    public LuaObject getLuaObject(LuaObject parent, LuaObject name)
            throws LuaException {
        if (parent.getLuaState().getPointer() != luaState ||
                parent.getLuaState().getPointer() != name.getLuaState().getPointer())
            throw new LuaException("Object must have the same LuaState as the parent!");

        return new LuaObject(parent, name);
    }

    /**
     * Creates a reference to an object in the <code>index</code> position
     * of the stack
     *
     * @param index position on the stack
     * @return LuaObject
     */
    public LuaObject getLuaObject(int index) {
        if (isFunction(index))
            return new LuaFunction(this, index);
        else if (isTable(index))
            return new LuaTable(this, index);
        else
            return new LuaObject(this, index);
    }

    /**
     * When you call a function in lua, it may return a number, and the
     * number will be interpreted as a <code>Double</code>.<br>
     * This function converts the number into a type specified by
     * <code>retType</code>
     *
     * @param db      lua number to be converted
     * @param retType type to convert to
     * @return The converted number
     */
    public static Number convertLuaNumber(Double db, Class<?> retType) {
        // checks if retType is a primitive type
        if (retType.isPrimitive()) {
            if (retType == Integer.TYPE) {
                return db.intValue();
            } else if (retType == Long.TYPE) {
                return db.longValue();
            } else if (retType == Float.TYPE) {
                return db.floatValue();
            } else if (retType == Double.TYPE) {
                return db.doubleValue();
            } else if (retType == Byte.TYPE) {
                return db.byteValue();
            } else if (retType == Short.TYPE) {
                return db.shortValue();
            }
        } else if (Number.class.isAssignableFrom(retType)) {
            // Checks all possibilities of number types
            if (retType.isAssignableFrom(Long.class)) {
                return new Long(db.longValue());
            } else if (retType.isAssignableFrom(Integer.class)) {
                return new Integer(db.intValue());
            } else if (retType.isAssignableFrom(Float.class)) {
                return new Float(db.floatValue());
            } else if (retType.isAssignableFrom(Double.class)) {
                return db;
            } else if (retType.isAssignableFrom(Byte.class)) {
                return new Byte(db.byteValue());
            } else if (retType.isAssignableFrom(Short.class)) {
                return new Short(db.shortValue());
            }
        }

        // if all checks fail, return null
        return null;
    }

    public static Number convertLuaNumber(Long lg, Class<?> retType) {
        // checks if retType is a primitive type
        if (retType.isPrimitive()) {
            if (retType == Integer.TYPE) {
                return lg.intValue();
            } else if (retType == Long.TYPE) {
                return lg.longValue();
            } else if (retType == Float.TYPE) {
                return lg.floatValue();
            } else if (retType == Double.TYPE) {
                return lg.doubleValue();
            } else if (retType == Byte.TYPE) {
                return lg.byteValue();
            } else if (retType == Short.TYPE) {
                return lg.shortValue();
            }
        } else if (Number.class.isAssignableFrom(retType)) {
            // Checks all possibilities of number types
            if (retType.isAssignableFrom(Long.class)) {
                return new Long(lg.longValue());
            } else if (retType.isAssignableFrom(Integer.class)) {
                return new Integer(lg.intValue());
            } else if (retType.isAssignableFrom(Float.class)) {
                return new Float(lg.floatValue());
            } else if (retType.isAssignableFrom(Double.class)) {
                return lg;
            } else if (retType.isAssignableFrom(Byte.class)) {
                return new Byte(lg.byteValue());
            } else if (retType.isAssignableFrom(Short.class)) {
                return new Short(lg.shortValue());
            }
        }

        // if all checks fail, return null
        return null;
    }

    public void pushPrimitive() {
        pushJavaObject(Boolean.TYPE);
        setGlobal("boolean");
        pushJavaObject(Byte.TYPE);
        setGlobal("byte");
        pushJavaObject(Character.TYPE);
        setGlobal("char");
        pushJavaObject(Short.TYPE);
        setGlobal("short");
        pushJavaObject(Integer.TYPE);
        setGlobal("int");
        pushJavaObject(Long.TYPE);
        setGlobal("long");
        pushJavaObject(Float.TYPE);
        setGlobal("float");
        pushJavaObject(Double.TYPE);
        setGlobal("double");
    }

    public LuaFunction getFunction(String name) {
        LuaObject obj = getLuaObject(name);
        if (obj.isFunction())
            return obj.getFunction();
        return null;
    }
    public LuaFunction getFunction(int idx) {
        LuaObject obj = getLuaObject(idx);
        if (obj.isFunction())
            return obj.getFunction();
        return null;
    }
}
