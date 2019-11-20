/*
 * Copyright (c) 2013 Tah Wei Hoon.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Apache License Version 2.0,
 * with full text available at http://www.apache.org/licenses/LICENSE-2.0.html
 *
 * This software is provided "as is". Use at your own risk.
 */
package com.myopicmobile.textwarrior.common;

import java.io.File;

/**
 * Singleton class containing the symbols and operators of the Javascript language
 */
public class LanguageLua extends Language {
	private static Language _theOne = null;
	
	//private final static String functionTarget   = "_ENV|_G|_VERSION|assert|collectgarbage|coroutine|create|isyieldable|resume|running|status|wrap|yield|debug|gethook|getinfo|getlocal|getmetatable|getregistry|getupvalue|getuservalue|sethook|setlocal|setmetatable|setupvalue|setuservalue|traceback|upvalueid|upvaluejoin|dofile|error|getfenv|getmetatable|io|close|flush|input|lines|open|output|popen|read|stderr|stdin|stdout|tmpfile|type|write|ipairs|load|loadfile|loadstring|luajava|bindClass|clear|coding|createArray|createProxy|instanceof|loadLib|loaded|luapath|new|newInstance|package|math|abs|acos|asin|atan|atan2|ceil|cos|cosh|deg|exp|floor|fmod|frexp|huge|ldexp|log|log10|max|maxinteger|min|mininteger|modf|pi|pow|rad|random|randomseed|sin|sinh|sqrt|tan|tanh|tointeger|type|ult|module|next|os|clock|date|difftime|execute|exit|getenv|remove|rename|setlocale|time|tmpname|package|config|cpath|loaded|loaders|loadlib|path|preload|searchers|searchpath|seeall|pairs|pcall|print|rawequal|rawget|rawlen|rawset|require|select|setfenv|setmetatable|string|byte|char|dump|find|format|gfind|gmatch|gsub|len|lower|match|pack|packsize|rep|reverse|sub|unpack|upper|table|concat|foreach|foreachi|insert|maxn|move|pack|remove|sort|unpack|tonumber|tostring|type|unpack|char|charpattern|utf8|codepoint|codes|len|offset|xpcall";
	//private final static String functionTarget   = "_ENV|_G|_VERSION|assert|collectgarbage|coroutine.create|coroutine.isyieldable|coroutine.resume|coroutine.running|coroutine.status|coroutine.wrap|coroutine.yield|debug.debug|debug.gethook|debug.getinfo|debug.getlocal|debug.getmetatable|debug.getregistry|debug.getupvalue|debug.getuservalue|debug.sethook|debug.setlocal|debug.setmetatable|debug.setupvalue|debug.setuservalue|debug.traceback|debug.upvalueid|debug.upvaluejoin|dofile|error|getfenv|getmetatable|io.close|io.flush|io.input|io.lines|io.open|io.output|io.popen|io.read|io.stderr|io.stdin|io.stdout|io.tmpfile|io.type|io.write|ipairs|load|loadfile|loadstring|luajava.bindClass|luajava.clear|luajava.coding|luajava.createArray|luajava.createProxy|luajava.instanceof|luajava.loadLib|luajava.loaded|luajava.luapath|luajava.new|luajava.newInstance|luajava.package|math.abs|math.acos|math.asin|math.atan|math.atan2|math.ceil|math.cos|math.cosh|math.deg|math.exp|math.floor|math.fmod|math.frexp|math.huge|math.ldexp|math.log|math.log10|math.max|math.maxinteger|math.min|math.mininteger|math.modf|math.pi|math.pow|math.rad|math.random|math.randomseed|math.sin|math.sinh|math.sqrt|math.tan|math.tanh|math.tointeger|math.type|math.ult|module|next|os.clock|os.date|os.difftime|os.execute|os.exit|os.getenv|os.remove|os.rename|os.setlocale|os.time|os.tmpname|package.config|package.cpath|package.loaded|package.loaders|package.loadlib|package.path|package.preload|package.searchers|package.searchpath|package.seeall|pairs|pcall|print|rawequal|rawget|rawlen|rawset|require|select|setfenv|setmetatable|string.byte|string.char|string.dump|string.find|string.format|string.gfind|string.gmatch|string.gsub|string.len|string.lower|string.match|string.pack|string.packsize|string.rep|string.reverse|string.sub|string.unpack|string.upper|table.concat|table.foreach|table.foreachi|table.insert|table.maxn|table.move|table.pack|table.remove|table.sort|table.unpack|tonumber|tostring|type|unpack|utf8.char|utf8.charpattern|utf8.codepoint|utf8.codes|utf8.len|utf8.offset|xpcall";
	
	private final static String keywordTarget ="and|break|case|continue|default|defer|do|else|elseif|end|false|for|function|goto|if|in|lambda|local|nil|not|or|repeat|return|switch|then|true|until|when|while";
	private final static String globalTarget="self|__add|__band|__bnot|__bor|__bxor|__call|__close|__concat|__div|__eq|__gc|__idiv|__index|__le|__len|__lt|__mod|__mul|__newindex|__pow|__shl|__shr|__sub|__tostring|__unm|_ENV|_G|assert|collectgarbage|dofile|error|findtable|getmetatable|ipairs|load|loadfile|loadstring|module|next|pairs|pcall|print|rawequal|rawget|rawlen|rawset|require|select|self|setmetatable|tointeger|tonumber|tostring|type|unpack|xpcall";
	
	private final static String packageName="coroutine|debug|io|luajava|math|os|package|string|table|utf8";
	private final static String package_coroutine = "create|isyieldable|resume|running|status|wrap|yield";
	private final static String package_debug = "debug|gethook|getinfo|getlocal|getmetatable|getregistry|getupvalue|getuservalue|sethook|setlocal|setmetatable|setupvalue|setuservalue|traceback|upvalueid|upvaluejoin";
	private final static String package_io = "close|flush|info|input|isdir|lines|ls|mkdir|open|output|popen|read|readall|stderr|stdin|stdout|tmpfile|type|write";
	private final static String package_luajava = "astable|bindClass|clear|coding|createArray|createProxy|getContext|instanceof|loadLib|loaded|luapath|new|newArray|newInstance|override|package|tostring";
	private final static String package_math = "abs|acos|asin|atan|atan2|ceil|cos|cosh|deg|exp|floor|fmod|frexp|huge|ldexp|log|log10|max|maxinteger|min|mininteger|modf|pi|pow|rad|random|randomseed|sin|sinh|sqrt|tan|tanh|tointeger|type|ult";
	private final static String package_os = "clock|date|difftime|execute|exit|getenv|remove|rename|setlocale|time|tmpname";
	private final static String package_package = "config|cpath|loaded|loaders|loadlib|path|preload|searchers|searchpath|seeall";
	private final static String package_string = "byte|char|dump|find|format|gfind|gmatch|gsub|len|lower|match|pack|packsize|rep|reverse|sub|unpack|upper";
	private final static String package_table = "clear|clone|concat|const|find|foreach|foreachi|gfind|insert|maxn|move|pack|remove|size|sort|unpack";
	private final static String package_utf8 = "byte|char|charpattern|charpos|codepoint|codes|escape|find|fold|gfind|gmatch|gsub|insert|len|lower|match|ncasecmp|next|offset|remove|reverse|sub|title|upper|width|widthindex";
	private final static String extFunctionTarget="activity|call|compile|dump|each|enum|import|loadbitmap|loadlayout|loadmenu|service|set|task|thread|timer";
	private final static String functionTarget   = globalTarget+"|"+extFunctionTarget+"|"+packageName;;
	private final static String[] keywords = keywordTarget.split("\\|");
	
	private final static String[] names = functionTarget.split("\\|");

	private final static char[] LUA_OPERATORS = {
		'(', ')', '{', '}', ',', ';', '=', '+', '-',
		'/', '*', '&', '!', '|', ':', '[', ']', '<', '>',
		'?', '~', '%', '^'
	};
	public static Language getInstance(){
		if(_theOne == null){
			_theOne = new LanguageLua();
		}
		return _theOne;
	}
	
	private LanguageLua(){
		super.setOperators(LUA_OPERATORS);
		super.setKeywords(keywords);
		super.setNames(names);
		super.addBasePackage("io",package_io.split("\\|"));
		super.addBasePackage("string",package_string.split("\\|"));
		super.addBasePackage("luajava",package_luajava.split("\\|"));
		super.addBasePackage("os",package_os.split("\\|"));
		super.addBasePackage("table",package_table.split("\\|"));
		super.addBasePackage("math",package_math.split("\\|"));
		super.addBasePackage("utf8",package_utf8.split("\\|"));
		super.addBasePackage("coroutine",package_coroutine.split("\\|"));
		super.addBasePackage("package",package_package.split("\\|"));
		super.addBasePackage("debug",package_debug.split("\\|"));
	}
	
	/**
	 * Whether the word after c is a token
	 */
	public boolean isWordStart2(char c){
		return (c=='.');
	}
	
	public boolean isLineAStart(char c){
		return false;
	}
	
	/**
	 * Whether c0c1L is a token, where L is a sequence of characters until the end of the line
	 */
	public boolean isLineStart(char c0, char c1){
		return (c0 == '-' && c1 == '-');
	}

	/**
	 * Whether c0c1 signifies the start of a multi-line token
	 */
	public boolean isMultilineStartDelimiter(char c0, char c1){
		return (c0 == '[' && c1 == '[');
	}

	/**
	 * Whether c0c1 signifies the end of a multi-line token
	 */
	public boolean isMultilineEndDelimiter(char c0, char c1){
		return (c0 == ']' && c1 == ']');
	}
	
}
