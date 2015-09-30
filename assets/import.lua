local require=require
local table=require "table"
local packages = {}
local loaded = {}
luajava.package=packages
luajava.loaded=loaded
local _G=_G
local insert = table.insert
local new = luajava.new
local bindClass = luajava.bindClass

local function append(t,v)
    for _,_v in ipairs(t) do
        if _v==v then
            return
            end
        end
    insert(t,v)
    end


local function massage_classname (classname)
    if classname:find('_') then
        classname = classname:gsub('_','$')
        end
    return classname
    end

local function import_class (classname,packagename)
    packagename = massage_classname(packagename)
    local res,class = pcall(bindClass,packagename)
    if res then
        loaded[classname] = class
        return class
        end

    --[[packagename = packagename:gsub("%.(%w+)$","$%1")
    local res,class = pcall(bindClass,packagename)
    if res then
        loaded[classname] = class
        return class
        end]]
    end

local globalMT = {
    __index = function(T,classname)
        --  classname = massage_classname(classname)
        for i,p in ipairs(packages) do
            local class = loaded[classname] or import_class(classname,p..classname)
            if class then T[classname]=class return class end
            end
        return nil
        end
    }
--setmetatable(_G, globalMT)

local function import_require(name)
    local s,r=pcall(require,name)
    return s and r
    end


local function env_import(env)
    local _env=env
    setmetatable(_env, globalMT)
    return function (package)
        local i = package:find('%*$')
        if i then -- a wildcard; put into the package list, including the final '.'
            append(packages,package:sub(1,-2))
            else
            local classname = package:match('([%w_]+)$')
            local class =import_require(package) or import_class(classname,package)
            if class then
                _env[classname]=class
                if class ~= true then
                    _env[classname]=class
                    end
                return class
                else
                error("cannot find "..package)
                end
            end
        end
    end

import=env_import(_G)
append(packages,'')

function enum(e)
    return function()
        if e.hasMoreElements() then
            return e.nextElement()
            end
        end
    end

function each(o)
    local iter=o.iterator()
    return function()
        if iter.hasNext() then
            return iter.next()
            end
        end
    end

function dump (t)
    local _t={}
    for k,v in pairs(t) do
        table.insert(_t,tostring(k).."="..tostring(v))
        end
    t=table.concat(_t,"\n")
    return t
    end

local content=activity
local ViewGroup=bindClass("android.view.ViewGroup")
local String=bindClass("java.lang.String")
local Gravity=bindClass("android.view.Gravity")
local OnClickListener=bindClass("android.view.View$OnClickListener")
local TypedValue=luajava.bindClass("android.util.TypedValue")
local BitmapDrawable=luajava.bindClass("android.graphics.drawable.BitmapDrawable")
local LuaDrawable=luajava.bindClass "com.androlua.LuaDrawable"
local ArrayAdapter=bindClass("android.widget.ArrayAdapter")
local android_R=bindClass("android.R")
android={R=android_R}

local dm=content.getResources().getDisplayMetrics()
local id=0x7f000000
local toint={
    -- android:drawingCacheQuality
    auto=0,
    low=1,
    high=2,

    -- android:importantForAccessibility
    auto=0,
    yes=1,
    no=2,

    -- android:layerType
    none=0,
    software=1,
    hardware=2,

    -- android:layoutDirection
    ltr=0,
    rtl=1,
    inherit=2,
    locale=3,

    -- android:scrollbarStyle
    insideOverlay=0x0,
    insideInset=0x01000000,
    outsideOverlay=0x02000000,
    outsideInset=0x03000000,

    -- android:visibility
    visible=0,
    invisible=1,
    gone=2,

    wrap_content=-2,
    fill_parent=-1,
    match_parent=-1,
    wrap=-2,
    fill=-1,
    match=-1,

    -- android:orientation
    vertical=1,
    horizontal= 0,

    -- android:gravity
    axis_clip = 8,
    axis_pull_after = 4,
    axis_pull_before = 2,
    axis_specified = 1,
    axis_x_shift = 0,
    axis_y_shift = 4,
    bottom = 80,
    center = 17,
    center_horizontal = 1,
    center_vertical = 16,
    clip_horizontal = 8,
    clip_vertical = 128,
    display_clip_horizontal = 16777216,
    display_clip_vertical = 268435456,
    --    fill = 119,
    fill_horizontal = 7,
    fill_vertical = 112,
    horizontal_gravity_mask = 7,
    left = 3,
    no_gravity = 0,
    relative_horizontal_gravity_mask = 8388615,
    relative_layout_direction = 8388608,
    right = 5,
    start = 8388611,
    top = 48,
    vertical_gravity_mask = 112,
    ["end"] = 8388613,

    -- android:textAlignment
    inherit=0,
    gravity=1,
    textStart=2,
    textEnd=3,
    textCenter=4,
    viewStart=5,
    viewEnd=6,
    
    -- android:inputType
    none=0x00000000,
    text=0x00000001,
    textCapCharacters=0x00001001,
    textCapWords=0x00002001,
    textCapSentences=0x00004001,
    textAutoCorrect=0x00008001,
    textAutoComplete=0x00010001,
    textMultiLine=0x00020001,
    textImeMultiLine=0x00040001,
    textNoSuggestions=0x00080001,
    textUri=0x00000011,
    textEmailAddress=0x00000021,
    textEmailSubject=0x00000031,
    textShortMessage=0x00000041,
    textLongMessage=0x00000051,
    textPersonName=0x00000061,
    textPostalAddress=0x00000071,
    textPassword=0x00000081,
    textVisiblePassword=0x00000091,
    textWebEditText=0x000000a1,
    textFilter=0x000000b1,
    textPhonetic=0x000000c1,
    textWebEmailAddress=0x000000d1,
    textWebPassword=0x000000e1,
    number=0x00000002,
    numberSigned=0x00001002,
    numberDecimal=0x00002002,
    numberPassword=0x00000012,
    phone=0x00000003,
    datetime=0x00000004,
    date=0x00000014,
    time=0x00000024,

    }
local rules={
    layout_above=2,
    layout_alignBaseline=4,
    layout_alignBottom=8,
    layout_alignEnd=19,
    layout_alignLeft=5,
    layout_alignParentBottom=12,
    layout_alignParentEnd=21,
    layout_alignParentLeft=9,
    layout_alignParentRight=11,
    layout_alignParentStart=20,
    layout_alignParentTop=10,
    layout_alignRight=7,
    layout_alignStart=18,
    layout_alignTop=6,
    layout_alignWithParentIfMissing=0,
    layout_below=3,
    layout_centerHorizontal=14,
    layout_centerInParent=13,
    layout_centerVertical=15,
    layout_toEndOf=17,
    layout_toLeftOf=0,
    layout_toRightOf=1,
    layout_toStartOf=16
    }


local types={
    px=0,
    dp=1,
    sp=2,
    pt=3,
    ["in"]=4,
    mm=5
    }

local function checkType(v)
    local n,ty=string.match(v,"^(%d+)(%a%a)$")
    return tonumber(n),types[ty]
    end

local function checkNumber(var)
    if type(var) == "string" then
        if var=="true" then
            return true
            elseif var=="false" then
            return false
            end
        
        local s=var:find("|")
        if s then
            local i1=toint[var:sub(1,s-1)]
            local i2=toint[var:sub(s+1,-1)]
            if i1 and i2 then
                return i1 | i2
                end
            end
        if toint[var] then
            return toint[var]
            end
        
        local h=string.match(var,"^#(%x+)$")
        if h then
            local c=tonumber(h,16)
            if c then
                if #h<=6 then
                    return c|0xffffffffff000000
                    elseif #h<=8 then
                    c=math.modf((c<<32)/0xffffffff)
                    return c
                    end
                end
            end
        
        local n,ty=checkType(var)
        if ty then
            return TypedValue.applyDimension(ty,n,dm)
            end
        end
    -- return var
    end

local function checkValue(var)
    return tonumber(var) or checkNumber(var) or var
    end

local function checkValues(...)
    local vars={...}
    for n=1,#vars do
        vars[n]=checkValue(vars[n])
        end
    return unpack(vars)
    end



local function env_loadlayout(env)
    local _env=env
    local ids={}
    local Listeners={}
    return function(t,root,group)
        local view = t[1](content) --创建view
        local LayoutParams=(group or ViewGroup).LayoutParams
        local params=ViewGroup.LayoutParams(checkValue(t.layout_width) or -2,checkValue(t.layout_height) or -2) --设置layout属性
        
        if group then
            params=LayoutParams(params)
            end
         if t.layout_x then
            params.x=(checkValue(t.layout_x or 0))
            end
         if t.layout_y then
            params.y=(checkValue(t.layout_y or 0))
            end
         
        if t.layout_weight then
            params.weight=(checkValue(t.layout_weight or 0))
            end
        
        if t.layout_gravity then
            params.gravity=checkValue(t.layout_gravity)
            end
        if t.layout_margin or t.layout_marginStart or t.layout_marginEnd or t.layout_marginLeft or t.layout_marginTop or t.layout_marginRight or t.layout_marginBottom then
            params.setMargins(checkValues( t.layout_marginLeft or t.layout_margin or 0,t.layout_marginTop or t.layout_margin or 0,t.layout_marginRight or t.layout_margin or 0,t.layout_marginBottom or t.layout_margin or 0))
            end
        if t.layout_marginStart then
            params.setMarginStart(checkValue(t.layout_marginStrat))
            end
        if t.layout_marginEnd then
            params.setMarginEnd(checkValue(t.layout_marginEnd))
            end
        for k,v in pairs(t) do
            if rules[k] and v==true then
                params.addRule(rules[k])
                elseif rules[k] then
                params.addRule(rules[k],ids[v])
                end
            end
        
        if params then
            view.setLayoutParams(params)
            end

        --设置padding属性
        if t.padding or t.paddingLeft or t.paddingTop or t.paddingRight or t.paddingBottom then
            view.setPadding(checkValues(t.paddingLeft or t.padding or 0, t.paddingTop or t.padding or 0, t.paddingRight or t.padding or 0, t.paddingBottom or t.padding or 0))
            end
        if t.paddingStart or t.paddingEnd then
            view.setPaddingRelative(checkValues(t.paddingStart or t.padding or 0, t.paddingTop or t.padding or 0, t.paddingEnd or t.padding or 0, t.paddingBottom or t.padding or 0))
            end

        for k,v in pairs(t) do
            if tonumber(k) and type(v)=="table" then --创建子view
                view.addView(loadlayout(v,root,t[1]))
                elseif k=="id" then --创建view的全局变量
                rawset(root or _env,v,view)
                view.setId(id)
                ids[v]=id
                id=id+1
                elseif k=="items" and type(v)=="table" then --创建列表项目
                local adapter=ArrayAdapter(content,android_R.layout.simple_list_item_1, String(v))
                view.setAdapter(adapter)
                elseif k=="text" then
                view.setText(v)
                elseif k=="textSize" then
                if tonumber(v) then
                    view.setTextSize(tonumber(v))
                    elseif type(v)=="string" then
                    local n,ty=checkType(v)
                    if ty then
                        view.setTextSize(ty,n)
                        else
                        view.setTextSize(v)
                        end
                    else
                    view.setTextSize(v)
                    end
                
                elseif k=="onClick" then --设置onClick事件接口
                local listener
                if type(v)=="function" then
                    listener=OnClickListener{onClick=v}
                    elseif type(v)=="userdata" then
                    listener=v
                    elseif type(v)=="string" then
                    if Listeners[v] then
                        listener=Listeners[v]
                        else
                        local l=rawget(_env,v)
                        if type(l)=="function" then
                            listener=OnClickListener{onClick=l}
                            elseif type(l)=="userdata" then
                            listener=l
                            end
                        Listeners[v]=listener
                        end
                    end
                view.setOnClickListener(listener)
                elseif k=="src" then
                task([[require "import" url=... return loadbitmap(url)]],v,function(bmp)view.setImageBitmap(bmp)end)
                elseif k=="background" then
                if type(v)=="string" then
                    if v:find("#") then
                        view.setBackgroundColor(checkNumber(v))
                        elseif v:find("^%w+$") then
                        v=rawget(_env,v)
                        if type(v)=="function" then
                            view.setBackground(LuaDrawable(v))
                            elseif type(v)=="userdata" then
                            view.setBackground(v)
                            end
                        else
                        task([[require "import" url=... return loadbitmap(url)]],v,function(bmp)view.setBackground(BitmapDrawable(bmp))end)
                        end
                    elseif type(v)=="userdata" then
                    view.setBackground(v)
                    elseif type(v)=="number" then
                    view.setBackgroundColor(v)
                    end
                elseif k=="password" and (v=="true" or v==true) then
                view.setInputType(0x81)
                elseif type(k)=="string" and not(k:find("layout_")) and not(k:find("padding")) then --设置属性
                k=string.gsub(k,"^(%w)",function(s)return string.upper(s)end)
                local st,err=pcall(function(view,k,v)view["set"..k](checkValue(v))end,view,k,v)
                if st==false then
                    os.execute("log -p w -t lua layout table warning "..err)
                    end
                end
            end
        return view
        end
    end

loadlayout=env_loadlayout(_G)

local LuaAsyncTask=luajava.bindClass("com.androlua.LuaAsyncTask")
local LuaThread=luajava.bindClass("com.androlua.LuaThread")
local LuaTimer=luajava.bindClass("com.androlua.LuaTimer")
local Object=luajava.bindClass("java.lang.Object")

function print(...)
    local buf={}
    for n=1,select("#",...) do
        table.insert(buf,tostring(select(n,...)))
        end
    local msg=table.concat(buf,"\t\t")
    activity.sendMsg(msg)
    os.execute("log -p d -t lua "..msg)
    end

local function setmetamethod(t,k,v)
    getmetatable(t)[k]=v
    end
local function getmetamethod(t,k,v)
    return getmetatable(t)[k]
    end


local getjavamethod=getmetamethod(LuaThread,"__index")
local function __call(t,k)
    return function(...)
        if ... then
            t.call(k,Object{...})
            else
            t.call(k)
            end
        end
    end

local function __index(t,k)
    local s,r=pcall(getjavamethod,t,k)
    if s then
        return r
        end
    local r=__call(t,k)
    setmetamethod(t,k,r)
    return r
    end

local function __newindex(t,k,v)
    t.set(k,v)
    end

local function checkPath(str)
    if str:find("^[%w%.%_]+$") then
        if luajava.luapath then
            return string.format("%s%s.lua",luajava.luapath:match("^(.-)[^/]+$"),str)
            end
        end
    return str
    end

function thread(src,...)
    if type(src)=="string" then
    src=checkPath(src)
    end
    local luaThread
    if ... then
        luaThread=LuaThread(content,src,true,Object{...})
        else
        luaThread=LuaThread(content,src,true)
        end
    luaThread.start()
    --setmetamethod(luaThread,"__call",__call)
    setmetamethod(luaThread,"__index",__index)
    setmetamethod(luaThread,"__newindex",__newindex)
    return luaThread
    end

function task(src,...)
    local args={...}
    local callback=args[select("#",...)]
    args[select("#",...)]=nil
    local luaAsyncTask=LuaAsyncTask(content,src,callback)
    luaAsyncTask.execute(args)
    end
    
function timer(f,d,p,...)
    local luaTimer=LuaTimer(content,f,Object{...})
    if p==0 then
        luaTimer.start(d)
        else
        luaTimer.start(d,p)
        end
    return luaTimer
    end


local LuaBitmap=luajava.bindClass "com.androlua.LuaBitmap"
function loadbitmap(path)
    if path:find("^[%w%.%_]+$") then
        if luajava.luapath then
            return LuaBitmap.getLoacalBitmap(string.format("%s%s",luajava.luapath:match("^(.-)[^/]+$"),path))
            else
            return LuaBitmap.getAssetBitmap(content,path)
            end
        elseif path:find("^https*://") then
        return LuaBitmap.getHttpBitmap(path)
        else
        return LuaBitmap.getLoacalBitmap(path)
        end
    end

function utf8.sub(s,i,j)

    i=utf8.offset(s,i)
    j=((j or -1)==-1 and -1) or utf8.offset(s,j+1)-1
    return string.sub(s,i,j)

    end

import 'java.lang.*'
import 'java.util.*'

function new_env()
    local _env={
        activity=content,
        require=require,
        dump=dump,
        each=each,
        enum=enum,
        print=print,
        task=task,
        timer=timer,
        thread=thread,
        loadbitmap=loadbitmap,
        android=android,
        _G=_G
        }
    _env.import=env_import(_env)
    _env.loadlayout=env_loadlayout(_env)
    return _env
    end

return new_env()

