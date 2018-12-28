require "import"
import "console"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "com.androlua.*"
import "java.io.*"
import "android.text.method.*"
import "android.net.*"
import "android.content.*"
import "android.graphics.drawable.*"
import "bin"
import "autotheme"

require "layout"
activity.setTitle('AndroLua+')

activity.setTheme(autotheme())

function onVersionChanged(n, o)
    local dlg = AlertDialogBuilder(activity)
    local title = "更新" .. o .. ">" .. n
    local msg = [[
    4.2.0
    支持虚类。

    4.1.9
    优化Http返回值。

    4.1.8
    优化HorizontalListView。
    布局表支持GIF背景图设置。

    4.1.7
    优化HorizontalListView。
    优化日志查看。

    4.1.6
    增加activity.stopService()方法。
    修复activity.startService()方法。

    4.1.5
    增加activity.getLuaResources()方法。
    优化LuaResources。

    4.1.4
    增加androlua://package name/path?key=value uri支持。

    4.1.3
    修复FileProvider bug。
    修复LuaBitmapDrawable bug。
    最近任务可变标题与图标。
    第一次运行获得权限。

    4.1.2
    setter支持多个参数。

    4.1.1
    Http post增加一些方法。
    浮点数比较使用近似值。

    4.1.0
    适配安卓P。

    4.0.25
    增加LuaBitmapDrawable，支持jpg，png，gif，bmp等。
    支持设置网络图片缓存时间。

    4.0.24
    优化强制类型处理。
    优化错误信息。

    4.0.23
    优化HorizontalListView。
    可以省略所有仅使用Context的构建方法的参数。

    4.0.22
    优化PageView,增加PageAdapter。
    增加HorizontalListView。

    4.0.21
    优化task最多支持1024线程，128并行。

    4.0.20
    一些更新。

    4.0.19
    修复格式化bug。
    LuaEditor增加addPackage方法。

    4.0.18
    省略Dialog的context参数。

    4.0.17
    修复获取内部类缓存bug。

    4.0.16
    修复高亮异常。

    4.0.15
    修复setter参数为table时意外错误的bug。

    4.0.14
    增加LuaPreferenceFragment。

    4.0.13
    修复bug。

    4.0.12
    编辑器增加块区间线。

    4.0.11
    优化自动换行。
    优化高亮。

    4.0.10
    优化自动换行。
    优化LuaAdapter。

    4.0.9
    优化getter/setter效率。
    优化单参数无参数方法效率。

    4.0.8
    优化布局助手。
    优化布局表对style支持。

    4.0.7
    优化LuaDialog。
    增加多属性赋值语句。

    4.0.6
    修复第一次初始化过慢bug。

    4.0.5
    增加设置luaExtDir方法。
    修复部分bug。

    4.0.4
    优化lua字符串U码使用。
    支持鼠标滚轮滚动。
    修复部分bug。

    4.0.3
    兼容旧版main入口。
    增加luajava.getContext()函数。
    activity增加部分api。
    修复部分bug。

    4.0.2
    修复部分bug。

    4.0.1
    优化import对环境影响。
    修复部分bug。

    4.0.0
    升级为lua 5.4随机数算法。
    修复lua部分bug。

    4.0.0 rc4
    增加__type元方法。
    修复switch case bug。

    4.0.0 rc3
    增加continue。

    4.0.0 rc2
    一些优化。

    4.0.0 alpha4
    使用增强的utf8模块。
    增加简化lambda表达式。

    4.0.0 alpha3
    优化switch case。

    4.0.0 alpha2
    可选择省略then do。
    增加switch case。

    4.0.0 rc1
    修复activity缓存的bug。

    4.0.0 beta4
    修复载入dex缓存的bug。

    4.0.0 beta3
    模拟滑动支持指定path
    bug修复。

    4.0.0 beta2
    修复打包不能安装bug。

    4.0.0 beta
    修复安卓8不能打包bug。

    4.0.0 alpha
    优化辅助功能，支持监听按键，模拟点击，模拟触摸，截图等功能。
    增加ColorFinder类，用于颜色查找。
    修复loadbitmap分辨率异常的bug。

    3.6.5
    修复参数转换bug。

    3.6.4
    修复导入分析不能的bug。
    修复执行子目录文件的bug。
    支持画中画模式。
    修复适配器搜索的bug。

    3.6.3
    修复参数转换bug。

    3.6.2
    适配全面屏。
    支持分屏。
    增加getUriForPath/getUriForFile方法。
    增强超级辅助。

    3.6.1
    优化辅助功能超级辅助。

    3.6.0
    修复finish activity可能导致程序退出的bug。
    修复运行内嵌子工程引用目录混乱的bug。
    增强辅助功能超级辅助。\

    3.5.9
    修复华为看不到log的bug。
    追加适配器过滤器。
    增加辅助功能超级辅助。

    3.5.8
    修复bug。

    3.5.7
    修复子目录导入bug。

    3.5.6
    增加LuaContentObserver。

    3.5.5
    增加LuaFileObserver。

    3.5.4
    增加getActivity方法。

    3.5.3
    增加LuaServer/LuaClient。
    bug修复。

    3.5.2
    修复异步Http不能获取错误请求详细信息的bug。
    import增加一次导入多个包功能。

    3.5
    增加代码加密。

    3.4.5
    增加多窗口功能。

    3.4.3
    增加与活动文件同名函数回调，方便处理传入参数。
    修复统计sdk读取app id一处bug。

    3.4.2
    增加百度移动统计与错误反馈。
    bug修复。

    3.4.1
    增加共享数据getSharedData/setSharedData方法。
    bug修复。

    3.4.0
    增加LuaResources。

    3.3.5
    增加辅助功能回调函数onAccessibilityEvent。
    修复bug。
    3.3.4

    增加LuaMultiAdapter适配器。
    优化RippleLayout。
    回滚编辑器实现。
    支持修改工程版本号和SDK版本。
    支持设置关联文件。
    优化权限选择提示。
    修复bug。

    3.3.3
    增加打包时编译自带模块。
    优化RippleLayout效果。
    优化编辑器高亮解析速度。
    优化PullingLayout效果。
    优化LuaDrawable和LuaAnimation实现。
    getter支持isxxx简写。
    LuaWebView支持打开第三方应用。
    LuaWebView支持处理SSL错误。
    bug修复。

    3.3.2
    增加LuaDialog。
    修复RippleLayout连续点击效果异常的bug。
    修复部分工程不能导入布局的bug。
    增加方法调用时函数自动转为接口。

    3.3.1
    增加RippleLayout圆形扩散效果。
    修复布局助手不能查看子视图的bug。

    3.3
    优化setter效率提高800%。
    优化适配器数据格式，支持首字母小写。
    优化编辑器效率。
    更新帮助。
    bug修复。

    3.2.6
    优化setter/getter，支持首字母小写。
    优化异步Http多线程。
    优化异步task多线程。
    优化LuaArrayAdapter图片显示效果。
    优化编辑器。
    修复LuaUtil.zip压缩文件错误的bug。
    优化bmob用户登录。
    bug修复。

    3.2.5
    去除无用权限。
    优化编辑器。
    优化RippleLayout效果。
    优化错误信息。
    LuaContext增加getGlobalData()方法。
    修复LuaThread函数调用两次的bug。
    增强loadlayout容错。
    增加导入分析。
    修复安卓7.x打包bug。
    bug修复。
    
    3.2.4
    修复悬浮窗焦点切换无效的bug。
    
    3.2.3
    关联alp文件。
    增加简单test功能。
    修复bug。
    
    3.2.2
    优化FloatWindow焦点切换。
    修复bug。
    
    3.2.1
    增加RippleLayout。
    增加LuaExpandableListAdapter适配器。
    优化ToolBar显示效果。
    修复垃圾回收bug。
    修复jar资源加载异常的bug。
    
    3.2
    更新Lua5.3.3。
    增加onVersionChanged回调函数。
    增加onResult回调函数。
    优化搜索选中效果。
    增加ide支持高亮与补全Java类。
    修复横竖屏切换bug。
    增加Http异步网络模块。
    修复在最左边删除，看不到待删除字符的问题。
    修复ToolBar不能设置空白标题的bug。
    优化PageLayouts与SlidingLayout.
    增加PullingLayout布局。
    增加线程自动回收机制。
    增加PageView。
    增加LuaFragment。
    增加级联风格调用。
    修复未实现接口函数调用出错的bug。
    增加支持自动导入libs目录so模块。
    增加支持TextView跑马灯。
    修复加载dex异常的bug。
    增加设置壁纸权限。
    优化task用法，自动导入外部代码导入的包与类。
    优化启动闪图逻辑。
    增加启动图不全屏时，自动适应空白区域颜色。
    优化内核，性能提高40%。
    优化打开工程逻辑。
    打开工程支持搜索。
    增加比例尺寸。
    优化log显示效果。
    优化第一次启动闪图效果。
    增加ide最近打开功能。
    增加记录最近打开文件光标位置功能。
    更新帮助。

    3.1
    增加可视布局设计器，
    升级内核，速度提高20%，
    http模块支持自定义UA与header
    优化luajava错误提示，
    增加工程导出/导入，
    修复打开文件的bug，
    增加后台服务，
    优化错误提示，
    修复类型转换bug，
    增加Ticker主线程回调定时器，
    编辑器自动夜间模式，
    编辑器支持自定义配色，
    增加导入dex函数，
    loadbitmap加载网络图片支持本地缓存，
    LuaArrayAdapter和LuaAdapter适配器支持异步加载图片与缓存，
    增加Java方法浏览器，
    增加导包提示，

    3.0.0
    支持打包apk的权限配置，
    增加Map对象的简洁使用，
    完善luajavaa.astable函数，全面支持array List Map，
    增加在方法调用时Lua表自动转换为Java数组或接口，
    增加LuaArrayAdapter和LuaAdapter适配器，
    LuaWebView支WebClient，在js调用Lua函数，
    timer支持设置时间间隔，
    newActivity支持传递参数，
    http增加download和upload，
    日志支持清除，
    Java方法支持table与array，map与interface自动转换，
    增强取长度运算符，可以获取Java对象大小，
    更换运行方式，
    支持打包文件夹，
    打包自动分析使用的c模块，
    增加tointeger函数，
    setContentView支持布局表参数，

    2.1.0
    去除广告，欢迎捐赠，
    修复接口方法错误无法显示错误信息的问题，
    修复import函数一处逻辑错误，
    修复onKeyDown等回调方法不能返回值的bug，
    优化luajava性能，
    优化IDE编辑器性能，
    修复IDE打开文件bug，
    增加setXXXListener控件事件快速设置，
    重写task与thread函数
    增加timer函数，
    修复数字类型转换bug，
    增加查看logcat输出功能，
    布局表支持绝对布局，
    布局表支持ListView预设项目，
    布局表支持style属性，
    布局表支持?android获取系统资源，
    修复astable索引0的bug，
    IDE增加函数导航，
    IDE增加搜索与转到，

    2.0.4
    增加luajava.astable方法，
    增加each与enum迭代器
    布局表支持相对布局，
    布局表gravity属性支持或( | )操作，
    优化IDE逻辑，

    2.0.3
    修复IDE布局bug

    2.0.2
    增加getter与setter快速调用，用于简化控件属性设置，
    修复Java方法返回null没有返回值的bug，
    更新布局表算法，支持布局间距，
    优化Java方法缓存机制，效率提高一倍，布局表效率提高8倍，

    2.0.1
    布局表增加自绘制背景，
    修复自动缩进算法错误，
    增加百度广告，仅在打包时出现，不影响使用，希望大家支持，

    2.0
    更新Lua5.3.1，
    更新luajava3.0，
    增加打包apk功能，
    增加布局表，
    增加线程，
    增加更多回调方法，
    更新支持高亮，自动缩进，自动补全编辑器，

    更多请参考帮助

  ]]
    if o == "" then
        title = "欢迎使用AndroLua+ " .. n
        msg = [[
    AndroLua+是由nirenr开发的在安卓使用Lua语言开发应用的工具，该项目基于开源项目luajava和AndroLua优化加强，修复了原版的bug，并加入了很多新的特性，使开发更加简单高效，使用该软件完全免费，如果你喜欢这个项目欢迎捐赠或者宣传他。
    在使用之前建议详细阅读程序自带帮助文档。
    用户协议
    作者不对使用该软件产生的任何直接或间接损失负责。
    勿使用该程序编写恶意程序以损害他人。
    继续使用表示你已知晓并同意该协议。
    
]] .. msg
    end
    dlg.setTitle(title)

    dlg.setMessage(msg)
    dlg.setPositiveButton("确定", nil)
    dlg.setNegativeButton("帮助", { onClick = func.help })
    dlg.setNeutralButton("捐赠", { onClick = func.donation })
    dlg.show()
end



--activity.setTheme(android.R.style.Theme_Holo_Light)
local version = Build.VERSION.SDK_INT;
local h = tonumber(os.date("%H"))
function ext(f)
    local f=io.open(f)
    if f then
        f:close()
        return true
    end
    return false
end

local theme
if h <= 6 or h >= 22 then
    theme = activity.getLuaExtDir("fonts") .. "/night.lua"
else
    theme = activity.getLuaExtDir("fonts") .. "/day.lua"
end
if not ext(theme) then
    theme = activity.getLuaExtDir("fonts") .. "/theme.lua"
end

local function day()
    if version >= 21 then
        return (android.R.style.Theme_Material_Light)
    else
        return (android.R.style.Theme_Holo_Light)
    end
end

local function night()
    if version >= 21 then
        return (android.R.style.Theme_Material)
    else
        return (android.R.style.Theme_Holo)
    end
end
local p = {}
local e = pcall(loadfile(theme, "bt", p))
if e then
    for k, v in pairs(p) do
        if k == "theme" then
            if v == "day" then
                activity.setTheme(day())
            elseif v == "night" then
                activity.setTheme(night())
            end
        else
            layout.main[2][k] = v
        end
    end
end
activity.getWindow().setSoftInputMode(0x10)

--activity.getActionBar().show()
history = {}
luahist = luajava.luadir .. "/lua.hist"
luadir = luajava.luaextdir .. "/" or "/sdcard/androlua/"
luaconf = luajava.luadir .. "/lua.conf"
luaproj = luajava.luadir .. "/lua.proj"
pcall(dofile, luaconf)
pcall(dofile, luahist)
luapath = luapath or luadir .. "new.lua"
luadir = luapath:match("^(.-)[^/]+$")
pcall(dofile, luaproj)
luaproject = luaproject
if luaproject then
    local p = {}
    local e = pcall(loadfile(luaproject .. "init.lua", "bt", p))
    if e then
        activity.setTitle(tostring(p.appname))
        Toast.makeText(activity, "打开工程." .. p.appname, Toast.LENGTH_SHORT ).show()
    end
end

activity.getActionBar().setDisplayShowHomeEnabled(false)
luabindir = luajava.luaextdir .. "/bin/"
code = [===[
require "import"
import "android.widget.*"
import "android.view.*"

]===]
pcode = [[
require "import"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "layout"
--activity.setTitle('AndroLua+')
--activity.setTheme(android.R.style.Theme_Holo_Light)
activity.setContentView(loadlayout(layout))
]]

lcode = [[
{
  LinearLayout,
  orientation="vertical",
  layout_width="fill",
  layout_height="fill",
  {
    TextView,
    text="hello AndroLua+",
    layout_width="fill",
  },
}
]]
upcode = [[
user_permission={
  "INTERNET",
  "WRITE_EXTERNAL_STORAGE",
}
]]

local BitmapDrawable = luajava.bindClass("android.graphics.drawable.BitmapDrawable")
m = {
    { MenuItem,
      title = "运行",
      id = "play",
      icon = "play", },
    { MenuItem,
      title = "撤销",
      id = "undo",
      icon = "undo", },
    { MenuItem,
      title = "重做",
      id = "redo",
      icon = "redo", },
    { MenuItem,
      title = "打开",
      id = "file_open", },
    { MenuItem,
      title = "最近",
      id = "file_history", },
    { SubMenu,
      title = "文件...",
      { MenuItem,
        title = "保存",
        id = "file_save", },
      { MenuItem,
        title = "新建",
        id = "file_new", },
      { MenuItem,
        title = "编译",
        id = "file_build", },
    },
    { SubMenu,
      title = "工程...",
      { MenuItem,
        title = "打开",
        id = "project_open", },
      { MenuItem,
        title = "打包",
        id = "project_build", },
      { MenuItem,
        title = "新建",
        id = "project_create", },
      { MenuItem,
        title = "导出",
        id = "project_export", },
      { MenuItem,
        title = "属性",
        id = "project_info", },
    },
    { SubMenu,
      title = "代码...",
      { MenuItem,
        title = "格式化",
        id = "code_format", },
      { MenuItem,
        title = "导入分析",
        id = "code_import", },
      { MenuItem,
        title = "查错",
        id = "code_check", },
    },
    { SubMenu,
      title = "转到...",
      { MenuItem,
        title = "搜索",
        id = "goto_seach", },
      { MenuItem,
        title = "转到",
        id = "goto_line", },
      { MenuItem,
        title = "导航",
        id = "goto_func", },
    },
    { MenuItem,
      title = "插件...",
      id = "plugin", },
    { SubMenu,
      title = "更多...",
      { MenuItem,
        title = "布局助手",
        id = "more_helper", },
      { MenuItem,
        title = "日志",
        id = "more_logcat", },
      { MenuItem,
        title = "Java浏览器",
        id = "more_java", },
      { MenuItem,
        title = "帮助",
        id = "more_help", },
      { MenuItem,
        title = "手册",
        id = "more_manual", },
      { MenuItem,
        title = "支持作者",
        id = "more_donation", },
      { MenuItem,
        title = "联系作者",
        id = "more_qq", },
      { MenuItem,
        title = "关于",
        id = "more_about", },
    },
}
optmenu = {}
function onCreateOptionsMenu(menu)
    loadmenu(menu, m, optmenu, 3)
end

function switch2(s)
    return function(t)
        local f = t[s]
        if not f then
            for k, v in pairs(t) do
                if s.equals(k) then
                    f = v
                    break
                end
            end
        end
        f = f or t.default2
        return f and f()
    end
end

function donothing()
    print("功能开发中")
end

luaprojectdir = luajava.luaextdir .. "/project/"
function create_project()
    local appname = project_appName.getText().toString()
    local packagename = project_packageName.getText().toString()
    local f = File(luaprojectdir .. appname)
    if f.exists() then
        print("工程已存在")
        return
    end
    if not f.mkdirs() then
        print("工程创建失败")
        return

    end
    luadir = luaprojectdir .. appname .. "/"
    write(luadir .. "init.lua", string.format("appname=\"%s\"\nappver=\"1.0\"\npackagename=\"%s\"\n%s", appname, packagename, upcode))
    write(luadir .. "main.lua", pcode)
    write(luadir .. "layout.aly", lcode)
    --project_dlg.hide()
    luapath = luadir .. "main.lua"
    read(luapath)
end

function update(s)
    bin_dlg.setMessage(s)
end

function callback(s)
    bin_dlg.hide()
    bin_dlg.Message = ""
    if not s:find("成功") then
        create_error_dlg()
        error_dlg.Message = s
        error_dlg.show()
    end
end

function reopen(path)
    local f = io.open(path, "r")
    if f then
        local str = f:read("*all")
        if tostring(editor.getText()) ~= str then
            editor.setText(str, true)
        end
        f:close()
    end
end

function read(path)

    local f = io.open(path, "r")
    if f == nil then
        --Toast.makeText(activity, "打开文件出错."..path, Toast.LENGTH_LONG ).show()
        error()
        return
    end
    local str = f:read("*all")
    f:close()
    if string.byte(str) == 0x1b then
        Toast.makeText(activity, "不能打开已编译文件." .. path, Toast.LENGTH_LONG ).show()
        return
    end
    editor.setText(str)

    activity.getActionBar().setSubtitle(".." .. path:match("(/[^/]+/[^/]+)$"))
    luapath = path
    if history[luapath] then
        editor.setSelection(history[luapath])
    end
    table.insert(history, 1, luapath)
    for n = 2, #history do
        if n > 50 then
            history[n] = nil
        elseif history[n] == luapath then
            table.remove(history, n)
        end
    end
    write(luaconf, string.format("luapath=%q", path))
    if luaproject and path:find(luaproject, 1, true) then
        --Toast.makeText(activity, "打开文件."..path, Toast.LENGTH_SHORT ).show()
        activity.getActionBar().setSubtitle(path:sub(#luaproject))
        return
    end

    local dir = luadir
    local p = {}
    local e = pcall(loadfile(dir .. "init.lua", "bt", p))
    while not e do
        dir, n = dir:gsub("[^/]+/$", "")
        if n == 0 then
            break
        end
        e = pcall(loadfile(dir .. "init.lua", "bt", p))
    end

    if e then
        activity.setTitle(tostring(p.appname))
        luaproject = dir
        activity.getActionBar().setSubtitle(path:sub(#luaproject))
        write(luaproj, string.format("luaproject=%q", luaproject))
        --Toast.makeText(activity, "打开工程."..p.appname, Toast.LENGTH_SHORT ).show()
    else
        activity.setTitle("AndroLua+")
        luaproject = nil
        write(luaproj, "luaproject=nil")
        --Toast.makeText(activity, "打开文件."..path, Toast.LENGTH_SHORT ).show()
    end
end

function write(path, str)
    local sw = io.open(path, "wb")
    if sw then
        sw:write(str)
        sw:close()
    else
        Toast.makeText(activity, "保存失败." .. path, Toast.LENGTH_SHORT ).show()
    end
    return str
end

function save()
    history[luapath] = editor.getSelectionEnd()
    local str = ""
    local f = io.open(luapath, "r")
    if f then
        str = f:read("*all")
        f:close()
    end
    local src = editor.getText().toString()
    if src ~= str then
        write(luapath, src)
    end
    return src
end

function click(s)
    func[s.getText()]()
end

function create_lua()
    luapath = luadir .. create_e.getText().toString() .. ".lua"
    if not pcall(read, luapath) then
        f = io.open(luapath, "a")
        f:write(code)
        f:close()
        table.insert(history, 1, luapath)
        editor.setText(code)
        write(luaconf, string.format("luapath=%q", luapath))
        Toast.makeText(activity, "新建文件." .. luapath, Toast.LENGTH_SHORT ).show()
    else
        Toast.makeText(activity, "打开文件." .. luapath, Toast.LENGTH_SHORT ).show()
    end
    write(luaconf, string.format("luapath=%q", luapath))
    activity.getActionBar().setSubtitle(".." .. luapath:match("(/[^/]+/[^/]+)$"))
    --create_dlg.hide()
end

function create_dir()
    luadir = luadir .. create_e.getText().toString() .. "/"
    if File(luadir).exists() then
        Toast.makeText(activity, "文件夹已存在." .. luadir, Toast.LENGTH_SHORT ).show()
    elseif File(luadir).mkdirs() then
        Toast.makeText(activity, "创建文件夹." .. luadir, Toast.LENGTH_SHORT ).show()
    else
        Toast.makeText(activity, "创建失败." .. luadir, Toast.LENGTH_SHORT ).show()
    end
end

function create_aly()
    luapath = luadir .. create_e.getText().toString() .. ".aly"
    if not pcall(read, luapath) then
        f = io.open(luapath, "a")
        f:write(lcode)
        f:close()
        table.insert(history, 1, luapath)
        editor.setText(lcode)
        write(luaconf, string.format("luapath=%q", luapath))
        Toast.makeText(activity, "新建文件." .. luapath, Toast.LENGTH_SHORT ).show()
    else
        Toast.makeText(activity, "打开文件." .. luapath, Toast.LENGTH_SHORT ).show()
    end
    write(luaconf, string.format("luapath=%q", luapath))
    activity.getActionBar().setSubtitle(".." .. luapath:match("(/[^/]+/[^/]+)$"))
    --create_dlg.hide()
end

function open(p)
    if p == luadir then
        return nil
    end
    if p:find("%.%./") then
        luadir = luadir:match("(.-)[^/]+/$")
        list(listview, luadir)
    elseif p:find("/") then
        luadir = luadir .. p
        list(listview, luadir)
    elseif p:find("%.alp$") then
        imports(luadir .. p)
        open_dlg.hide()
    else
        read(luadir .. p)
        open_dlg.hide()
    end
end

function sort(a, b)
    if string.lower(a) < string.lower(b) then
        return true
    else
        return false
    end
end

function adapter(t)
    return ArrayListAdapter(activity, android.R.layout.simple_list_item_1, String(t))
end

function list(v, p)
    local f = File(p)
    if not f then
        open_title.setText(p)
        local adapter = ArrayAdapter(activity, android.R.layout.simple_list_item_1, String {})
        v.setAdapter(adapter)
        return
    end

    local fs = f.listFiles()
    fs = fs or String[0]
    Arrays.sort(fs)
    local t = {}
    local td = {}
    local tf = {}
    if p ~= "/" then
        table.insert(td, "../")
    end
    for n = 0, #fs - 1 do
        local name = fs[n].getName()
        if fs[n].isDirectory() then
            table.insert(td, name .. "/")
        elseif name:find("%.lua$") or name:find("%.aly$") or name:find("%.alp$") then
            table.insert(tf, name)
        end
    end
    table.sort(td, sort)
    table.sort(tf, sort)
    for k, v in ipairs(tf) do
        table.insert(td, v)
    end
    open_title.setText(p)
    --local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String(td))
    --v.setAdapter(adapter)
    open_dlg.setItems(td)
end

function list2(v, p)
    local adapter = ArrayListAdapter(activity, android.R.layout.simple_list_item_1, String(history))
    v.setAdapter(adapter)
    plist = history
end

function export(pdir)
    require "import"
    import "java.util.zip.*"
    import "java.io.*"
    local function copy(input, output)
        local b = byte[2 ^ 16]
        local l = input.read(b)
        while l > 1 do
            output.write(b, 0, l)
            l = input.read(b)
        end
        input.close()
    end

    local f = File(pdir)
    local date = os.date("%y%m%d%H%M%S")
    local tmp = activity.getLuaExtDir("backup") .. "/" .. f.Name .. "_" .. date .. ".alp"
    local p = {}
    local e, s = pcall(loadfile(f.Path .. "/init.lua", "bt", p))
    if e then
        if p.mode then
            tmp = string.format("%s/%s_%s_%s-%s.%s", activity.getLuaExtDir("backup"), p.appname,p.mode, p.appver:gsub("%.", "_"), date,p.ext or "alp")
        else
            tmp = string.format("%s/%s_%s-%s.%s", activity.getLuaExtDir("backup"), p.appname, p.appver:gsub("%.", "_"), date,p.ext or "alp")
        end
    end
    local out = ZipOutputStream(FileOutputStream(tmp))
    local using={}
    local using_tmp={}
    function addDir(out, dir, f)
        local ls = f.listFiles()
        --entry=ZipEntry(dir)
        --out.putNextEntry(entry)
        for n = 0, #ls - 1 do
            local name = ls[n].getName()
            if name:find("%.apk$") or name:find("%.luac$") or name:find("^%.") then
            elseif p.mode and name:find("%.lua$") and name ~= "init.lua" then
                local ff=io.open(ls[n].Path)
                local ss=ff:read("a")
                ff:close()
                for u in ss:gmatch([[require *%b""]]) do
                    if using_tmp[u]==nil then
                        table.insert(using,u)
                        using_tmp[u]=true
                    end
                end
                local path, err = console.build(ls[n].Path)
                if path then
                    entry = ZipEntry(dir .. name)
                    out.putNextEntry(entry)
                    copy(FileInputStream(File(path)), out)
                    os.remove(path)
                else
                    error(err)
                end
            elseif p.mode and name:find("%.aly$") then
                name = name:gsub("aly$", "lua")
                local path, err = console.build_aly(ls[n].Path)
                if path then
                    entry = ZipEntry(dir .. name)
                    out.putNextEntry(entry)
                    copy(FileInputStream(File(path)), out)
                    os.remove(path)
                else
                    error(err)
                end
            elseif ls[n].isDirectory() then
                addDir(out, dir .. name .. "/", ls[n])
            else
                entry = ZipEntry(dir .. name)
                out.putNextEntry(entry)
                copy(FileInputStream(ls[n]), out)
            end
        end
    end

    addDir(out, "", f)
    local ff=io.open(f.Path.."/.using","w")
    ff:write(table.concat(using,"\n"))
    ff:close()
    entry = ZipEntry(".using")
    out.putNextEntry(entry)
    copy(FileInputStream(f.Path.."/.using"), out)

    out.closeEntry()
    out.close()
    return tmp
end

function getalpinfo(path)
    local app = {}
    loadstring(tostring(String(LuaUtil.readZip(path, "init.lua"))), "bt", "bt", app)()
    local str = string.format("名称: %s\
版本: %s\
包名: %s\
作者: %s\
说明: %s\
路径: %s",
            app.appname,
            app.appver,
            app.packagename,
            app.developer,
            app.description,
            path
    )
    return str, app.mode
end

function imports(path)
    create_imports_dlg()
    local mode
    imports_dlg.Message, mode = getalpinfo(path)
    if mode == "plugin" or path:match("^([^%._]+)_plugin") then
        imports_dlg.setTitle("导入插件")
    elseif mode == "build" or path:match("^([^%._]+)_build") then
        imports_dlg.setTitle("打包安装")
    end
    imports_dlg.show()
end

function importx(path, tp)
    require "import"
    import "java.util.zip.*"
    import "java.io.*"
    local function copy(input, output)
        local b = byte[2 ^ 16]
        local l = input.read(b)
        while l > 1 do
            output.write(b, 0, l)
            l = input.read(b)
        end
        output.close()
    end

    local f = File(path)
    local app = {}
    loadstring(tostring(String(LuaUtil.readZip(path, "init.lua"))), "bt", "bt", app)()

    local s = app.appname or f.Name:match("^([^%._]+)")
    local out = activity.getLuaExtDir("project") .. "/" .. s

    if tp == "build" then
        out = activity.getLuaExtDir("bin/.temp") .. "/" .. s
    elseif tp == "plugin" then
        out = activity.getLuaExtDir("plugin") .. "/" .. s
    end
    local d = File(out)
    if autorm then
        local n = 1
        while d.exists() do
            n = n + 1
            d = File(out .. "-" .. n)
        end
    end
    if not d.exists() then
        d.mkdirs()
    end
    out = out .. "/"
    local zip = ZipFile(f)
    local entries = zip.entries()
    for entry in enum(entries) do
        local name = entry.Name
        local tmp = File(out .. name)
        local pf = tmp.ParentFile
        if not pf.exists() then
            pf.mkdirs()
        end
        if entry.isDirectory() then
            if not tmp.exists() then
                tmp.mkdirs()
            end
        else
            copy(zip.getInputStream(entry), FileOutputStream(out .. name))
        end
    end
    zip.close()
    function callback2(s)
        LuaUtil.rmDir(File(activity.getLuaExtDir("bin/.temp")))
        bin_dlg.hide()
        bin_dlg.Message = ""
        if s==nil or not s:find("成功") then
            create_error_dlg()
            error_dlg.Message = s
            error_dlg.show()
        end
    end

    if tp == "build" then
        bin(out)
        return out
    elseif tp == "plugin" then
        Toast.makeText(activity, "导入插件." .. s, Toast.LENGTH_SHORT ).show()
        return out
    end
    luadir = out
    luapath = luadir .. "main.lua"
    read(luapath)
    Toast.makeText(activity, "导入工程." .. luadir, Toast.LENGTH_SHORT ).show()
    return out
end

func = {}
func.open = function()
    save()
    create_open_dlg()
    list(listview, luadir)
    open_dlg.show()
end
func.new = function()
    save()
    create_create_dlg()
    create_dlg.setMessage(luadir)
    create_dlg.show()
end

func.history = function()
    save()
    create_open_dlg2()
    list2(listview2)
    open_edit.Text = ""
    open_dlg2.show()
end

func.create = function()
    save()
    create_project_dlg()
    project_dlg.show()
end
func.openproject = function()
    save()
    activity.newActivity("project")
    --[[
      create_open_dlg2()
      list2(listview2, luaprojectdir)
      open_edit.Text=""
      open_dlg2.show()]]
end

func.export = function()
    save()
    if luaproject then
        local name = export(luaproject)
        Toast.makeText(activity, "工程已导出." .. name, Toast.LENGTH_SHORT ).show()
    else
        Toast.makeText(activity, "仅支持工程导出.", Toast.LENGTH_SHORT ).show()
    end
end

func.save = function()
    save()
    Toast.makeText(activity, "文件已保存." .. luapath, Toast.LENGTH_SHORT ).show()
end

func.play = function()
    if func.check(true) then
        return
    end
    save()
    if luaproject then
        activity.newActivity(luaproject .. "main.lua")
    else
        activity.newActivity(luapath)
    end
end
func.undo = function()
    editor.undo()
end
func.redo = function()
    editor.redo()
end
func.format = function()
    editor.format()
end
func.check = function(b)
    local src = editor.getText()
    src = src.toString()
    if luapath:find("%.aly$") then
        src = "return " .. src
    end
    local _, data = loadstring(src)

    if data then
        local _, _, line, data = data:find(".(%d+).(.+)")
        editor.gotoLine(tonumber(line))
        Toast.makeText(activity, line .. ":" .. data, Toast.LENGTH_SHORT ).show()
        return true
    elseif b then
    else
        Toast.makeText(activity, "没有语法错误", Toast.LENGTH_SHORT ).show()
    end
end

func.navi = function()
    create_navi_dlg()
    local str = editor.getText().toString()
    local fs = {}
    indexs = {}
    for s, i in str:gmatch("([%w%._]* *=? *function *[%w%._]*%b())()") do
        i = utf8.len(str, 1, i) - 1
        s = s:gsub("^ +", "")
        table.insert(fs, s)
        table.insert(indexs, i)
        fs[s] = i
    end
    local adapter = ArrayAdapter(activity, android.R.layout.simple_list_item_1, String(fs))
    navi_list.setAdapter(adapter)
    navi_dlg.show()
end

func.seach = function()
    editor.search()
end

func.gotoline = function()
    editor.gotoLine()
end

func.luac = function()
    save()
    local path, str = console.build(luapath)
    if path then
        Toast.makeText(activity, "编译完成: " .. path, Toast.LENGTH_SHORT ).show()
    else
        Toast.makeText(activity, "编译出错: " .. str, Toast.LENGTH_SHORT ).show()
    end
end

func.build = function()
    save()
    if not luaproject then
        Toast.makeText(activity, "仅支持工程打包.", Toast.LENGTH_SHORT ).show()
        return
    end
    bin(luaproject .. "/")
end

buildfile = function()
    Toast.makeText(activity, "正在打包..", Toast.LENGTH_SHORT ).show()
    task(bin, luaPath.getText().toString(), appName.getText().toString(), appVer.getText().toString(), packageName.getText().toString(), apkPath.getText().toString(), function(s)
        status.setText(s or "打包出错!")
    end)
end

func.info = function()
    if not luaproject then
        Toast.makeText(activity, "仅支持修改工程属性.", Toast.LENGTH_SHORT ).show()
        return
    end
    activity.newActivity("projectinfo", { luaproject })
end

func.logcat = function()
    activity.newActivity("logcat")
end

func.help = function()
    activity.newActivity("help")
end

func.java = function()
    activity.newActivity("javaapi/main")
end

func.manual = function()
    activity.newActivity("luadoc")
end

func.helper = function()
    save()
    isupdate = true
    activity.newActivity("layouthelper/main", { luaproject, luapath })
end

func.donation = function()
    xpcall(function()
        local url = "alipayqr://platformapi/startapp?saId=10000007&clientVersion=3.7.0.0718&qrcode=https://qr.alipay.com/apt7ujjb4jngmu3z9a"
        activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
    end,
            function()
                local url = "https://qr.alipay.com/apt7ujjb4jngmu3z9a";
                activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
            end)
end

key2 = [[N_9Rrnm8jJcdcXs7TQsXQBVA8Liq8mhU]]

key = [[QRDW1jiyM81x-T8RMIgeX1g_v76QSo6a]]
function joinQQGroup(key)
    import "android.content.Intent"
    import "android.net.Uri"
    local intent = Intent();
    intent.setData(Uri.parse("mqqopensdkapi://bizAgent/qm/qr?url=http%3A%2F%2Fqm.qq.com%2Fcgi-bin%2Fqm%2Fqr%3Ffrom%3Dapp%26p%3Dandroid%26k%3D" .. key));
    activity.startActivity(intent);
end

func.qq = function()
    joinQQGroup(key)
end

func.about = function()
    onVersionChanged("", "")
end

func.fiximport = function()
    save()
    activity.newActivity("javaapi/fiximport", { luaproject, luapath })
end

func.plugin = function()
    activity.newActivity("plugin/main", { luaproject, luapath })
end

function onMenuItemSelected(id, item)
    switch2(item) {
        default2 = function()
            print("功能开发中。。。")
        end,
        [optmenu.play] = func.play,
        [optmenu.undo] = func.undo,
        [optmenu.redo] = func.redo,
        [optmenu.file_open] = func.open,
        [optmenu.file_history] = func.history,
        [optmenu.file_save] = func.save,
        [optmenu.file_new] = func.new,
        [optmenu.file_build] = func.luac,
        [optmenu.project_open] = func.openproject,
        [optmenu.project_build] = func.build,
        [optmenu.project_create] = func.create,
        [optmenu.project_export] = func.export,
        [optmenu.project_info] = func.info,
        [optmenu.code_format] = func.format,
        [optmenu.code_check] = func.check,
        [optmenu.code_import] = func.fiximport,
        [optmenu.goto_line] = func.gotoline,
        [optmenu.goto_func] = func.navi,
        [optmenu.goto_seach] = func.seach,
        [optmenu.more_helper] = func.helper,
        [optmenu.more_logcat] = func.logcat,
        [optmenu.more_java] = func.java,
        [optmenu.more_help] = func.help,
        [optmenu.more_manual] = func.manual,
        [optmenu.more_donation] = func.donation,
        [optmenu.more_qq] = func.qq,
        [optmenu.more_about] = func.about,
        [optmenu.plugin] = func.plugin,
    }
end

activity.setContentView(layout.main)

function onCreate(s)
    --[[ local intent=activity.getIntent()
    local uri=intent.getData()
    if not s and uri and uri.getPath():find("%.alp$") then
      imports(uri.getPath())
    else]]
    if pcall(read, luapath) then
        last = last or 0
        if last < editor.getText().length() then
            editor.setSelection(last)
        end
    else
        luapath = activity.LuaExtDir .. "/new.lua"
        if not pcall(read, luapath) then
            write(luapath, code)
            pcall(read, luapath)
        end
    end
    --end
end

function onNewIntent(intent)
    local uri = intent.getData()
    if uri and uri.getPath():find("%.alp$") then
        imports(uri.getPath():match("/storage.+") or uri.getPath())
    end
end

function onResult(name, path)
    --print(name,path)
    if name == "project" then
        luadir = path .. "/"
        read(path .. "/main.lua")
    elseif name == "projectinfo" then
        activity.setTitle(path)
    end
end

function onActivityResult(req, res, intent)
    if res == 10000 then
        read(luapath)
        editor.format()
        return
    end
    if res ~= 0 then
        local data = intent.getStringExtra("data")
        local _, _, path, line = data:find("\n[	 ]*([^\n]-):(%d+):")
        if path == luapath then
            editor.gotoLine(tonumber(line))
        end
        local classes = require "javaapi.android"
        local c = data:match("a nil value %(global '(%w+)'%)")
        if c then
            local cls = {}
            c = "%." .. c .. "$"
            for k, v in ipairs(classes) do
                if v:find(c) then
                    table.insert(cls, string.format("import %q", v))
                end
            end
            if #cls > 0 then
                create_import_dlg()
                import_dlg.setItems(cls)
                import_dlg.show()
            end
        end

    end
end

function onStart()
    reopen(luapath)
    if isupdate then
        editor.format()
    end
    isupdate = false
end

function onStop()
    save()
    --Toast.makeText(activity, "文件已保存."..luapath, Toast.LENGTH_SHORT ).show()
    local f = io.open(luaconf, "wb")
    f:write( string.format("luapath=%q\nlast=%d", luapath, editor. getSelectionEnd() ))
    f:close()
    local f = io.open(luahist, "wb")
    f:write(string.format("history=%s", dump(history)))
    f:close()
end

--创建对话框
function create_navi_dlg()
    if navi_dlg then
        return
    end
    navi_dlg = Dialog(activity)
    navi_dlg.setTitle("导航")
    navi_list = ListView(activity)
    navi_list.onItemClick = function(parent, v, pos, id)
        editor.setSelection(indexs[pos + 1])
        navi_dlg.hide()
    end
    navi_dlg.setContentView(navi_list)
end

function create_imports_dlg()
    if imports_dlg then
        return
    end
    imports_dlg = AlertDialogBuilder(activity)
    imports_dlg.setTitle("导入")
    imports_dlg.setPositiveButton("确定", {
        onClick = function()
            local path = imports_dlg.Message:match("路径: (.+)$")
            if imports_dlg.Title == "打包安装" then
                importx(path, "build")
                imports_dlg.setTitle("导入")
            elseif imports_dlg.Title == "导入插件" then
                importx(path, "plugin")
                imports_dlg.setTitle("导入")
            else
                importx(path)
            end
        end })
    imports_dlg.setNegativeButton("取消", nil)
end

function create_delete_dlg()
    if delete_dlg then
        return
    end
    delete_dlg = AlertDialogBuilder(activity)
    delete_dlg.setTitle("删除")
    delete_dlg.setPositiveButton("确定", {
        onClick = function()
            if luapath:find(delete_dlg.Message) then
                Toast.makeText(activity, "不能删除正在打开的文件.", Toast.LENGTH_SHORT ).show()
            elseif LuaUtil.rmDir(File(delete_dlg.Message)) then
                Toast.makeText(activity, "已删除.", Toast.LENGTH_SHORT ).show()
                list(listview, luadir)
            else
                Toast.makeText(activity, "删除失败.", Toast.LENGTH_SHORT ).show()
            end
        end })
    delete_dlg.setNegativeButton("取消", nil)
end

function create_open_dlg()
    if open_dlg then
        return
    end
    open_dlg = AlertDialogBuilder(activity)
    open_dlg.setTitle("打开")
    open_title = TextView(activity)
    listview = open_dlg.ListView
    listview.FastScrollEnabled = true

    listview.addHeaderView(open_title)
    listview.setOnItemClickListener(AdapterView.OnItemClickListener {
        onItemClick = function(parent, v, pos, id)
            open(v.Text)
        end
    })

    listview.onItemLongClick = function(parent, v, pos, id)
        if v.Text ~= "../" then
            create_delete_dlg()
            delete_dlg.setMessage(luadir .. v.Text)
            delete_dlg.show()
        end
        return true
    end

    --open_dlg.setItems{"空"}
    --open_dlg.setContentView(listview)
end

function create_open_dlg2()
    if open_dlg2 then
        return
    end
    open_dlg2 = AlertDialogBuilder(activity)
    --open_dlg2.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);

    open_dlg2.setTitle("最近打开")
    open_dlg2.setView(loadlayout(layout.open2))

    --listview2=open_dlg2.ListView
    listview2.FastScrollEnabled = true
    --open_edit=EditText(activity)
    --listview2.addHeaderView(open_edit)

    open_edit.addTextChangedListener {
        onTextChanged = function(c)
            local s = tostring(c)
            if #s == 0 then
                listview2.setAdapter(adapter(plist))
            end
            local t = {}
            s = s:lower()
            for k, v in ipairs(plist) do
                if v:lower():find(s, 1, true) then
                    table.insert(t, v)
                end
            end
            listview2.setAdapter(adapter(t))
        end
    }

    listview2.setOnItemClickListener(AdapterView.OnItemClickListener {
        onItemClick = function(parent, v, pos, id)
            if File(v.Text).exists() then
                luadir = v.Text:gsub("[^/]+$", "")
                read(v.Text)
                open_dlg2.hide()
            else
                listview2.adapter.remove(pos)
                table.remove(plist, id)
                Toast.makeText(activity, "文件不存在", 1000).show()
            end
        end
    })
end

function create_create_dlg()
    if create_dlg then
        return
    end
    create_dlg = AlertDialogBuilder(activity)
    create_dlg.setMessage(luadir)
    create_dlg.setTitle("新建")
    create_e = EditText(activity)
    create_dlg.setView(create_e)
    create_dlg.setPositiveButton(".lua", { onClick = create_lua })
    create_dlg.setNegativeButton("dir", { onClick = create_dir })
    create_dlg.setNeutralButton(".aly", { onClick = create_aly })
end

function create_project_dlg()
    if project_dlg then
        return
    end
    project_dlg = AlertDialogBuilder(activity)
    project_dlg.setTitle("新建工程")
    project_dlg.setView(loadlayout(layout.project))
    project_dlg.setPositiveButton("确定", { onClick = create_project })
    project_dlg.setNegativeButton("取消", nil)
end

function create_build_dlg()
    if build_dlg then
        return
    end
    build_dlg = AlertDialogBuilder(activity)
    build_dlg.setTitle("打包")
    build_dlg.setView(loadlayout(layout.build))
    build_dlg.setPositiveButton("确定", { onClick = buildfile })
    build_dlg.setNegativeButton("取消", nil)
end

function create_bin_dlg()
    if bin_dlg then
        return
    end
    bin_dlg = ProgressDialog(activity);
    bin_dlg.setTitle("正在打包");
    bin_dlg.setMax(100);
end

import "android.content.*"
cm = activity.getSystemService(activity.CLIPBOARD_SERVICE)

function copyClip(str)
    local cd = ClipData.newPlainText("label", str)
    cm.setPrimaryClip(cd)
    Toast.makeText(activity, "已复制到剪切板", 1000).show()
end

function create_import_dlg()
    if import_dlg then
        return
    end
    import_dlg = AlertDialogBuilder(activity)
    import_dlg.Title = "可能需要导入的类"
    import_dlg.setPositiveButton("确定", nil)

    import_dlg.ListView.onItemClick = function(l, v)
        copyClip(v.Text)
        import_dlg.hide()
        return true
    end
end

function create_error_dlg()
    if error_dlg then
        return
    end
    error_dlg = AlertDialogBuilder(activity)
    error_dlg.Title = "出错"
    error_dlg.setPositiveButton("确定", nil)
end

lastclick = os.time() - 2
function onKeyDown(e)
    local now = os.time()
    if e == 4 then
        if now - lastclick > 2 then
            --print("再按一次退出程序")
            Toast.makeText(activity, "再按一次退出程序.", Toast.LENGTH_SHORT ).show()
            lastclick = now
            return true
        end
    end
end
local cd1 = ColorDrawable(0x00ffffff)
local cd2 = ColorDrawable(0x88000088)

local pressed = android.R.attr.state_pressed;
local window_focused = android.R.attr.state_window_focused;
local focused = android.R.attr.state_focused;
local selected = android.R.attr.state_selected;

function click(v)
    editor.paste(v.Text)
end

function newButton(text)
    local sd = StateListDrawable()
    sd.addState({ pressed }, cd2)
    sd.addState({ 0 }, cd1)
    local btn = TextView()
    btn.TextSize = 20;
    local pd = btn.TextSize / 2
    btn.setPadding(pd, pd / 2, pd, pd / 4)
    btn.Text = text
    btn.setBackgroundDrawable(sd)
    btn.onClick = click
    return btn
end
local ps = { "(", ")", "[", "]", "{", "}", "\"", "=", ":", ".", ",", "_", "+", "-", "*", "/", "\\", "%", "#", "^", "$", "?", "&", "|", "<", ">", "~", ";", "'" };
for k, v in ipairs(ps) do
    ps_bar.addView(newButton(v))
end

local function adds()
    require "import"
    local classes = require "javaapi.android"
    local ms = { "onCreate",
                 "onStart",
                 "onResume",
                 "onPause",
                 "onStop",
                 "onDestroy",
                 "onActivityResult",
                 "onResult",
                 "onCreateOptionsMenu",
                 "onOptionsItemSelected",
                 "onClick",
                 "onTouch",
                 "onLongClick",
                 "onItemClick",
                 "onItemLongClick",
    }
    local buf = String[#ms + #classes]
    for k, v in ipairs(ms) do
        buf[k - 1] = v
    end
    local l = #ms
    for k, v in ipairs(classes) do
        buf[l + k - 1] = string.match(v, "%w+$")
    end
    return buf
end
task(adds, function(buf)
    editor.addNames(buf)
end)

local buf={}
local tmp={}
local curr_ms=luajava.astable(LuaActivity.getMethods())
for k,v in ipairs(curr_ms) do
    v=v.getName()
    if not tmp[v] then
        tmp[v]=true
        table.insert(buf,v)
    end
end
editor.addPackage("activity",buf)


function fix(c)
    local classes = require "javaapi.android"
    if c then
        local cls = {}
        c = "%." .. c .. "$"
        for k, v in ipairs(classes) do
            if v:find(c) then
                table.insert(cls, string.format("import %q", v))
            end
        end
        if #cls > 0 then
            create_import_dlg()
            import_dlg.setItems(cls)
            import_dlg.show()
        end
    end
end

function onKeyShortcut(keyCode, event)
    local filteredMetaState = event.getMetaState() & ~KeyEvent.META_CTRL_MASK;
    if (KeyEvent.metaStateHasNoModifiers(filteredMetaState)) then
        switch(keyCode)
        case
        KeyEvent.KEYCODE_O
        func.open();
        return true;
        case
        KeyEvent.KEYCODE_P
        func.openproject();
        return true;
        case
        KeyEvent.KEYCODE_S
        func.save();
        return true;
        case
        KeyEvent.KEYCODE_E
        func.check();
        return true;
        case
        KeyEvent.KEYCODE_R
        func.play();
        return true;
        case
        KeyEvent.KEYCODE_N
        func.navi();
        return true;
        case
        KeyEvent.KEYCODE_U
        func.undo();
        return true;
        case
        KeyEvent.KEYCODE_I
        fix(editor.getSelectedText());
        return true;
    end
end
return false;
end


