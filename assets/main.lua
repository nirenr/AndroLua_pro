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

require "layout"
activity.setTitle('AndroLua+')

--activity.setTheme(android.R.style.Theme_Holo_Light)
local version = Build.VERSION.SDK_INT;
local h=tonumber(os.date("%H"))
if version >= 21 then
  if h<=6 or h>=22 then
    activity.setTheme(android.R.style.Theme_Material)
  else
    activity.setTheme(android.R.style.Theme_Material_Light)
  end
else
  if h<=6 or h>=22 then
    activity.setTheme(android.R.style.Theme_Holo)
  else
    activity.setTheme(android.R.style.Theme_Holo_Light)
  end
end

--activity.getActionBar().show()
luadir=luajava.luaextdir.."/" or "/sdcard/androlua/"
luaconf=luajava.luadir.."/lua.conf"
pcall(dofile,luaconf)
luapath=luapath or luadir.."new.lua"
luadir=luapath:match("^(.-)[^/]+$")
activity.getActionBar().setDisplayShowHomeEnabled(false)
luabindir=luajava.luaextdir.."/bin/"
code=[===[
require "import"
import "android.widget.*"
import "android.view.*"

]===]
pcode=[[
require "import"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "layout"
activity.setTitle('AndroLua+')
activity.setTheme(android.R.style.Theme_Holo_Light)
activity.setContentView(loadlayout(layout))
]]


lcode=[[
{
  LinearLayout,
  orientation="vertical",
  {
    TextView,
    text="hello AndroLua+",
    layout_width="fill"
  },
}
]]
upcode=[[
user_permission={
  "INTERNET",
  "WRITE_EXTERNAL_STORAGE",
}
]]
about=[[
AndroLua是基于LuaJava开发的安卓平台轻量级脚本编程语言工具，既具有Lua简洁优雅的特质，又支持绝大部分安卓API，可以使你在手机上快速编写小型应用。
官方QQ群：236938279
http://jq.qq.com/?_wv=1027&k=dcofRr
百度贴吧：
http://c.tieba.baidu.com/mo/m?kw=androlua
项目地址：
http://sf.net/p/androlua
点击链接支持我的工作，一块也可以哦：
https://qr.alipay.com/apt7ujjb4jngmu3z9a
]]


local BitmapDrawable=luajava.bindClass("android.graphics.drawable.BitmapDrawable")
m={
  {MenuItem,
    title="运行",
    id="play",
    icon="play",},
  {MenuItem,
    title="撤销",
    id="undo",
    icon="undo",},
  {MenuItem,
    title="重做",
    id="redo",
    icon="redo",},
  {MenuItem,
    title="打开",
    id="file_open",},
  {SubMenu,
    title="文件...",
    {MenuItem,
      title="保存",
      id="file_save",},
    {MenuItem,
      title="新建",
      id="file_new",},
    {MenuItem,
      title="编译",
      id="file_build",},
  },
  {SubMenu,
    title="工程...",
    {MenuItem,
      title="打开",
      id="project_open",},
    {MenuItem,
      title="打包",
      id="project_build",},
    {MenuItem,
      title="新建",
      id="project_create",},
    {MenuItem,
      title="导出",
      id="project_export",},
  },
  {SubMenu,
    title="代码...",
    {MenuItem,
      title="格式化",
      id="code_format",},
    {MenuItem,
      title="查错",
      id="code_check",},
  },
  {SubMenu,
    title="转到...",
    {MenuItem,
      title="搜索",
      id="goto_seach",},
    {MenuItem,
      title="转到",
      id="goto_line",},
    {MenuItem,
      title="导航",
      id="goto_func",},
  },
  {SubMenu,
    title="更多...",
    {MenuItem,
      title="布局助手",
      id="more_helper",},
    {MenuItem,
      title="日志",
      id="more_logcat",},
    {MenuItem,
      title="Java浏览器",
      id="more_java",},
    {MenuItem,
      title="帮助",
      id="more_help",},
    {MenuItem,
      title="手册",
      id="more_manual",},
    {MenuItem,
      title="支持作者",
      id="more_donation",},
   {MenuItem,
      title="联系作者",
      id="more_qq",},
  {MenuItem,
      title="论坛",
      id="more_bbs",},
 }
}
optmenu={}
items={"运行","撤销","重做","打开","保存","新建","新建工程", "格式化","查错","导航","转到","搜索","编译","打包","日志","帮助","手册",}
function onCreateOptionsMenu(menu)
  loadmenu(menu,m,optmenu)
end

function switch(s)
  return function(t)
    local f=t[s]
    if not f then
      for k,v in pairs(t) do
        if s.equals(k) then
          f=v
          break
        end
      end
    end
    f=f or t.default
    return f and f()
  end
end

function donothing()
  print("功能开发中")
end



luaprojectdir=luajava.luaextdir.."/project/"
function create_project()
  local appname=project_appName.getText().toString()
  local packagename=project_packageName.getText().toString()
  local f=File(luaprojectdir..appname)
  if f.exists() then
    print("工程已存在")
    return
  end
  if not f.mkdirs() then
    print("工程创建失败")
    return
  end
  luadir=luaprojectdir..appname.."/"
  write(luadir.."init.lua",string.format("appname=\"%s\"\nappver=\"1.0\"\npackagename=\"%s\"\n%s",appname,packagename,upcode))
  write(luadir.."main.lua",pcode)
  write(luadir.."layout.aly",lcode)
  project_dlg.hide()
  luapath=luadir.."main.lua"
  read(luapath)
end

function update(s)
  bin_dlg.setMessage(s)
end

function callback(s)
  if s:find("成功") then
    bin_dlg.hide()
  end
end
function read(path)
  local f=io.open(path,"r")
  editor.setText(f:read("*all"))
  f:close()
  activity.getActionBar().setSubtitle(".."..path:match("(/[^/]+/[^/]+)$"))
  local p={}
  local e=pcall(loadfile(luadir.."init.lua","bt",p))
  if e then
    activity.setTitle(p.appname)
  else
    activity.setTitle("AndroLua+")
  end
  luapath=path
  write(luaconf,string.format("luapath=%q",path))
  --Toast.makeText(activity, "打开文件."..path, Toast.LENGTH_SHORT ).show()
end

function write(path,str)
  local sw=io.open(path,"wb")
  if sw then
    sw:write(str)
    sw:close()
  else
    Toast.makeText(activity, "保存失败."..path, Toast.LENGTH_SHORT ).show()
  end
  return str
end

function save()
  local src=editor.getText().toString()
  write(luapath,src)
  return src
end

function click(s)
  func[s.getText()]()
end

function create_lua()
  luapath=luadir.. create_e.getText().toString()..".lua"
  if not pcall(read,luapath) then
    f=io.open(luapath,"a")
    f:write(code)
    f:close()
    editor.setText(code)
    write(luaconf,string.format("luapath=%q",luapath))
    Toast.makeText(activity, "新建文件."..luapath, Toast.LENGTH_SHORT ).show()
  else
    Toast.makeText(activity, "打开文件."..luapath, Toast.LENGTH_SHORT ).show()
  end
  write(luaconf,string.format("luapath=%q",luapath))
  activity.getActionBar().setSubtitle(".."..luapath:match("(/[^/]+/[^/]+)$"))
  create_dlg.hide()
end

function create_aly()
  luapath=luadir.. create_e.getText().toString()..".aly"
  if not pcall(read,luapath) then
    f=io.open(luapath,"a")
    f:write(lcode)
    f:close()
    editor.setText(lcode)
    write(luaconf,string.format("luapath=%q",luapath))
    Toast.makeText(activity, "新建文件."..luapath, Toast.LENGTH_SHORT ).show()
  else
    Toast.makeText(activity, "打开文件."..luapath, Toast.LENGTH_SHORT ).show()
  end
  write(luaconf,string.format("luapath=%q",luapath))
  activity.getActionBar().setSubtitle(".."..luapath:match("(/[^/]+/[^/]+)$"))
  create_dlg.hide()
end

function open(p)
  if p==luadir then
    return nil
  end
  if p:find("%.%./") then
    luadir=luadir:match("(.-)[^/]+/$")
    list(listview, luadir)
  elseif p:find("/") then
    luadir=luadir..p
    list(listview, luadir)
  elseif p:find("%.alp$") then
    luadir=imports(luadir..p)
    luapath=luadir.."main.lua"
    read(luapath)
    Toast.makeText(activity, "导入工程."..luadir, Toast.LENGTH_SHORT ).show()
    open_dlg.hide()
  else
    luapath=luadir..p
    read(luapath)
    Toast.makeText(activity, "打开文件."..luapath, Toast.LENGTH_SHORT ).show()
    open_dlg.hide()
  end
end



function sort(a,b)
  if string.lower(a) < string.lower(b) then
    return true
  else
    return false
  end
end

function list(v,p)
  local f=File(p)
  if not f then
    open_title.setText(p)
    local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String{})
    v.setAdapter(adapter)
    return
  end

  local fs=f.listFiles()
  fs=fs or String[0]
  Arrays.sort(fs)
  local t={}
  local td={}
  local tf={}
  if p~="/" then
    table.insert(td,"../")
  end
  for n=0,#fs-1 do
    local name=fs[n].getName()
    if fs[n].isDirectory() then
      table.insert(td,name.."/")
    elseif name:find("%.lua$") or name:find("%.aly$") or name:find("%.alp$") then
      table.insert(tf,name)
    end
  end
  table.sort(td,sort)
  table.sort(tf,sort)
  for k,v in ipairs(tf) do
    table.insert(td,v)
  end
  open_title.setText(p)
  --local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String(td))
  --v.setAdapter(adapter)
  open_dlg.setItems(td)
end

function list2(v,p)
  local f=File(p)
  if not f then
    open_title.setText(p)
    local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String{})
    v.setAdapter(adapter)
    return
  end

  local fs=f.listFiles()
  fs=fs or String[0]
  Arrays.sort(fs)
  local t={}
  local td={}
  local tf={}
  for n=0,#fs-1 do
    local name=fs[n].getName()
    if fs[n].isDirectory() then
      table.insert(td,name)
    --elseif name:find("%.lua$") or name:find("%.aly$") or name:find("%.alp$") then
      --table.insert(tf,name)
    end
  end
  table.sort(td,sort)
  --local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String(td))
  --v.setAdapter(adapter)
  open_dlg2.setItems(td)
end


bin=function(luapath,appname,appver,packagename,apkpath)
  require "import"
  import "console"

  import "java.util.zip.*"
  import "java.io.*"
  import "mao.res.*"
  import "apksigner.*"
  local b=byte[2^16]
  function copy(input,output)
    local l=input.read(b)
    while l>1 do
      output.write(b,0,l)
      l=input.read(b)
    end
  end

  local temp=File(apkpath).getParentFile();
  if (not temp.exists()) then

    if (not temp.mkdirs()) then

      error("create file " .. temp.getName() .. " fail");
    end
  end
  
  
  local tmp=luajava.luadir.."/tmp.apk"
  local info=activity.getApplicationInfo()
  local ver=activity.getPackageManager().getPackageInfo(activity.getPackageName(),0).versionName

  --local zip=ZipFile(info.publicSourceDir)
  local zipFile=File(info.publicSourceDir)
  local fis=FileInputStream(zipFile);
  local checksum = CheckedInputStream(fis, Adler32());
  local zis = ZipInputStream(BufferedInputStream(checksum));

  local out=ZipOutputStream(FileOutputStream(tmp))
  local f=File(luapath)
  local errbuffer={}
  local replace={
    ["lib/armeabi/libalog.so"]=true,
    ["lib/armeabi/libbson.so"]=true,
    ["lib/armeabi/libcanvas.so"]=true,
    ["lib/armeabi/libcjson.so"]=true,
    ["lib/armeabi/libgl.so"]=true,
    ["lib/armeabi/libcrypt.so"]=true,
    ["lib/armeabi/libjni.so"]=true,
    ["lib/armeabi/libmd5.so"]=true,
    ["lib/armeabi/libluv.so"]=true,
    ["lib/armeabi/libregex.so"]=true,
    ["lib/armeabi/libsensor.so"]=true,
    ["lib/armeabi/libxml.so"]=true,
    ["lib/armeabi/libtcc.so"]=true,
    ["lib/armeabi/libzlib.so"]=true,
    ["lib/armeabi/libzip.so"]=true,
    ["assets/main.alp"]=true,
  }

  function checklib(path)
    local f=io.open(path)
    local s=f:read("*a")
    f:close()
    for m in s:gmatch("require *\"([%w_]+)") do
      replace[string.format("lib/armeabi/lib%s.so",m)]=nil
    end
    for m in s:gmatch("import *\"([%w_]+)") do
      replace[string.format("lib/armeabi/lib%s.so",m)]=nil
    end
  end


  local mainpath


  function addDir(out,dir,f)
    local ls=f.listFiles()
    for n=0,#ls-1 do
      local name=ls[n].getName()
      if name:find("%.lua$") then
        checklib(luapath..dir..name)
        local path,err=console.build(luapath..dir..name)
        if path then
          entry=ZipEntry("assets/"..dir..name)
          out.putNextEntry(entry)
          replace["assets/"..dir..name]=true
          copy(FileInputStream(File(path)),out)
          os.remove(path)
        else
          table.insert(errbuffer,err)
        end
      elseif name:find("%.aly$") then
        local path,err=console.build_aly(luapath..dir..name)
        if path then
          name=name:gsub("aly$","lua")
          entry=ZipEntry("assets/"..dir..name)
          out.putNextEntry(entry)
          replace["assets/"..dir..name]=true
          copy(FileInputStream(File(path)),out)
          os.remove(path)
        else
          table.insert(errbuffer,err)
        end
      elseif ls[n].isDirectory() then
        addDir(out,dir..name.."/",ls[n])
      elseif ls[n].isFile() and (not ((name:find("%.luac$")) or (name:find("%.apk$")))) then
        entry=ZipEntry("assets/"..dir..name)
        out.putNextEntry(entry)
        replace["assets/"..dir..name]=true
        copy(FileInputStream(ls[n]),out)
      end
    end
  end

  this.update("正在编译...");
  if f.isDirectory() then
    require "permission"
    mainpath=luapath.."main.lua"
    dofile(luapath.."init.lua")
    for k,v in ipairs(user_permission or permission) do
      permission[v]=false
    end
    addDir(out,"",f)
    local wel=File(luapath.."welcome.png")
    if wel.exists() then
      entry=ZipEntry("res/drawable/welcome.png")
      out.putNextEntry(entry)
      replace["res/drawable/welcome.png"]=true
      copy(FileInputStream(wel),out)
    end
    local wel=File(luapath.."icon.png")
    if wel.exists() then
      entry=ZipEntry("res/drawable/icon.png")
      out.putNextEntry(entry)
      replace["res/drawable/icon.png"]=true
      copy(FileInputStream(wel),out)
    end
  else
    entry=ZipEntry("assets/main.lua")
    out.putNextEntry(entry)
    checklib(luapath)
    local path,err=console.build(luapath)
    if path then
      copy(FileInputStream(File(path)),out)
      os.remove(path)
    else
      table.insert(errbuffer,err)
    end
  end

  this.update("正在打包...");
  local entry = zis.getNextEntry();
  while entry do
    local name=entry.getName()
    if replace[name] then
    elseif name:find("^assets/") then
    else
      out.putNextEntry(entry)
      if entry.getName() == "AndroidManifest.xml" then
        local list=ArrayList()
        local xml=AXmlDecoder.read(list, zis)
        local req={
          [activity.getPackageName()]=packagename,
          [info.nonLocalizedLabel]=appname,
          [ver]=appver,
          [".*\\\\.alp"]="",
          [".*\\\\.lua"]="",
          [".*\\\\.luac"]="",
        }
        for n=0,list.size()-1 do
          local v=list.get(n)
          if req[v] then
            list.set(n,req[v])
          end
          if permission and permission[v:match("([%w_]+)$")] then
            list.set(n,"")
          end
        end
        xml.write(list,out)
      elseif not entry.isDirectory() then
        copy(zis,out)
      end
    end
    entry = zis.getNextEntry()
  end

  zis.close();
  out.closeEntry()
  out.close()

  if #errbuffer==0 then
    this.update("正在签名...");
    Signer.sign(tmp,apkpath)
    os.remove(tmp)
    import "android.net.*"
    import "android.content.*"
    i = Intent(Intent.ACTION_VIEW);
    i.setDataAndType(Uri.parse("file://"..apkpath), "application/vnd.android.package-archive");
    i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
    this.update("正在打开...");
    activity.startActivity(i);
    return "打包成功:"..apkpath
  else
    os.remove(tmp)
    this.update("打包出错:\n "..table.concat(errbuffer,"\n"));
    return "打包出错:\n "..table.concat(errbuffer,"\n")
  end
end


function export(pdir)
  require "import"
  import "java.util.zip.*"
  import "java.io.*"
  function copy(input,output)
    local b=byte[2^16]
    local l=input.read(b)
    while l>1 do
      output.write(b,0,l)
      l=input.read(b)
    end
    input.close()
  end

  local f=File(pdir)
  local tmp=activity.getLuaExtDir("project").."/"..f.Name..".alp"
  local out=ZipOutputStream(FileOutputStream(tmp))

  function addDir(out,dir,f)
    local ls=f.listFiles()
    --entry=ZipEntry(dir)
    --out.putNextEntry(entry)
    for n=0,#ls-1 do
      local name=ls[n].getName()
      if name:find("%.apk$") then
      elseif ls[n].isDirectory() then
        addDir(out,dir..name.."/",ls[n])
      elseif ls[n].isFile() and (not ((name:find("%.luac$")) or (name:find("%.apk$")))) then
        entry=ZipEntry(dir..name)
        out.putNextEntry(entry)
        copy(FileInputStream(ls[n]),out)
      end
    end
  end

  addDir(out,"",f)
  out.closeEntry()
  out.close()
  return tmp
end


function imports(path)
  require "import"
  import "java.util.zip.*"
  import "java.io.*"
  function copy(input,output)
    local b=byte[2^16]
    local l=input.read(b)
    while l>1 do
      output.write(b,0,l)
      l=input.read(b)
    end
    output.close()
  end

  local f=File(path)
  local s=f.Name:match("^(.+)%.alp$")
  local out=activity.getLuaExtDir("project").."/"..s
  local d=File(out)
  if autorm then
    local n=1
    while d.exists() do
      n=n+1
      d=File(out.."-"..n)
    end
  end
  if not d.exists() then
    d.mkdirs()
  end
  out=out.."/"
  local zip=ZipFile(f)
  local entries=zip.entries()
  for entry in enum(entries) do
    local name=entry.Name
    local tmp=File(out..name)
    local pf=tmp.ParentFile
    if not pf.exists() then
      pf.mkdirs()
    end
    if entry.isDirectory() then
      if not tmp.exists() then
        tmp.mkdirs()
      end
    else
      copy(zip.getInputStream(entry),FileOutputStream(out..name))
    end
  end
  zip.close()
  return out
end

func={}
func.open=function()
  save()
  list(listview, luadir)
  open_dlg.show()
end
func.new=function()
  save()
  create_dlg.setMessage(luadir)
  create_dlg.show()
end

func.create=function()
  save()
  project_dlg.show()
end
func.openproject=function()
  save()
  list2(listview2, luaprojectdir)
  open_dlg2.show()
end

func.export=function()
  save()
  local name=export(luadir)
  Toast.makeText(activity, "工程已导出."..name, Toast.LENGTH_SHORT ).show()
end

func.save=function()
  save()
  Toast.makeText(activity, "文件已保存."..luapath, Toast.LENGTH_SHORT ).show()
end

func.play=function()
  if func.check(true) then
    return
  end
  save()
  local p={}
  local e=pcall(loadfile(luadir.."init.lua","bt",p))
  if e then
    activity.newActivity(luadir.."main.lua")
  else
    activity.newActivity(luapath)
  end
end
func.undo=function()
  editor.undo()
end
func.redo=function()
  editor.redo()
end
func.format=function()
  editor.format()
end
func.check= function (b)
  local src=editor.getText()
  src=src.toString()
  if luapath:find("%.aly$") then
    src="return "..src
  end
  local _,data=loadstring(src)

  if data then
    local _,_,line,data=data:find(".(%d+).(.+)")
    editor.gotoLine(tonumber(line))
    Toast.makeText(activity,line..":".. data, Toast.LENGTH_SHORT ).show()
    return true
  elseif b then
  else
    Toast.makeText(activity, "没有语法错误", Toast.LENGTH_SHORT ).show()
  end
end

func.navi=function ()
  local str=editor.getText().toString()
  local fs={}
  indexs={}
  for s,i in str:gmatch("([%w%._]* *=? *function *[%w%._]*%b())()") do
    i=utf8.len(str,1,i)-1
    s=s:gsub("^ +","")
    table.insert(fs,s)
    table.insert(indexs,i)
    fs[s]=i
  end
  local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String(fs))
  navi_list.setAdapter(adapter)
  navi_dlg.show()
end

func.seach=function ()
  editor.search()
end

func.gotoline=function ()
  editor.gotoLine()
end

func.luac=function()
  save()
  local path,str=console.build(luapath)
  if path then
    Toast.makeText(activity, "编译完成: "..path, Toast.LENGTH_SHORT ).show()
  else
    Toast.makeText(activity, "编译出错: "..str, Toast.LENGTH_SHORT ).show()
  end
end

func.build=function()
  save()
  local p={}
  local e=pcall(loadfile(luadir.."init.lua","bt",p))
  if e then
    --Toast.makeText(activity, "正在打包..", Toast.LENGTH_SHORT ).show()
    activity.newTask(bin,update,callback).execute{luadir,p.appname,p.appver,p.packagename,luabindir..p.appname.."_"..p.appver..".apk"}
    bin_dlg.show()
    return nil
  end
  apkname=luapath:match("(%w+)%.lua$")
  apkname=apkname or "demo"
  packagename="com.androlua."..apkname
  local luadir="/sdcard/androlua/"
  apkpath=luadir..apkname..".apk"

  luaPath.setText(luapath)
  appName.setText(apkname)
  appVer.setText("1.0")
  packageName.setText(packagename)
  apkPath.setText(apkpath)
  build_dlg.show()
end

buildfile=function()
  Toast.makeText(activity, "正在打包..", Toast.LENGTH_SHORT ).show()
  task(bin,luaPath.getText().toString(),appName.getText().toString(),appVer.getText().toString(),packageName.getText().toString(),apkPath.getText().toString(),function(s)status.setText(s or "打包出错!")end)
end

func.logcat=function()
  activity.newActivity("logcat")
end

func.help=function()
  activity.newActivity("help")
end

func.java=function()
  activity.newActivity("java")
end

func.manual=function()
  activity.newActivity("luadoc")
end

func.helper=function()
  save()
  update=true
  activity.newActivity("layouthelper",{luadir,luapath})
end

func.donation=function()
  local url="alipayqr://platformapi/startapp?saId=10000007&clientVersion=3.7.0.0718&qrcode=https://qr.alipay.com/apt7ujjb4jngmu3z9a"
  activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
end

func.qq=function()
  local url="mqqwpa://im/chat?chat_type=wpa&uin=946049229"
  activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
end

func.bbs=function()
  local url="http://androlua.com"
  activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
end

function onMenuItemSelected(id,item)
  switch(item){
    default=function()
      print("功能开发中。。。")
    end,
    [optmenu.play]=func.play,
    [optmenu.undo]=func.undo,
    [optmenu.redo]=func.redo,
    [optmenu.file_open]=func.open,
    [optmenu.file_save]=func.save,
    [optmenu.file_new]=func.new,
    [optmenu.file_build]=func.luac,
    [optmenu.project_open]=func.openproject,
    [optmenu.project_build]=func.build,
    [optmenu.project_create]=func.create,
    [optmenu.project_export]=func.export,
    [optmenu.code_format]=func.format,
    [optmenu.code_check]=func.check,
    [optmenu.goto_line]=func.gotoline,
    [optmenu.goto_func]=func.navi,
    [optmenu.goto_seach]=func.seach,
    [optmenu.more_helper]=func.helper,
    [optmenu.more_logcat]=func.logcat,
    [optmenu.more_java]=func.java,
    [optmenu.more_help]=func.help,
    [optmenu.more_manual]=func.manual,
    [optmenu.more_donation]=func.donation,
    [optmenu.more_qq]=func.qq,
    [optmenu.more_bbs]=func.bbs,
  }
end

activity.setContentView(layout.main)

function onCreate(s)
  local intent=activity.getIntent()
  local uri=intent.getData()
  if not s and uri and uri.getPath():find("%.alp$") then
    luadir=imports(uri.getPath())
    luapath=luadir.."main.lua"
    read(luapath)
    Toast.makeText(activity, "导入工程."..luadir, Toast.LENGTH_SHORT ).show()
  else
    if pcall(read,luapath) then
      last=last or 0
      if last < editor.getText().length() then
        editor.setSelection(last)
      end
    else
      luapath=activity.LuaExtDir.."/new.lua"
      pcall(read,luapath)
    end
  end

end

function onNewIntent(intent)
  local uri=intent.getData()
  if uri and uri.getPath():find("%.alp$") then
    luadir=imports(uri.getPath())
    luapath=luadir.."main.lua"
    read(luapath)
    Toast.makeText(activity, "导入工程."..luadir, Toast.LENGTH_SHORT ).show()
  end
end

function onActivityResult(req,res,intent)
  if res==10000 then
    read(luapath)
    editor.format()
    return
  end
  if res~=0 then
    local data=intent.getStringExtra("data")
    local _,_,line=data:find(":(%d+):")
    editor.gotoLine (tonumber(line))
local classes=require "android"
local c=data:match("attempt to call a nil value %(global '(%w+)'%)")
if c then
  local cls={}
  c="%."..c.."$"
  for k,v in ipairs(classes) do
    if v:find(c) then
      table.insert(cls,string.format("import %q",v))
    end
  end
  if #cls>0 then
    local import_dlg=AlertDialogBuilder(activity)
    import_dlg.Title="可能需要导入的类"
    adp=LuaArrayAdapter(activity,{EditText,layout_width="fill"},String(cls))
    import_dlg.setAdapter(adp)
    import_dlg.setPositiveButton("确定",nil)
    import_dlg.show()
  end
end

  end
end

function onStart()
if update then
    read(luapath)
    editor.format()
end
update=false
end

function onStop()
  save()
  --Toast.makeText(activity, "文件已保存."..luapath, Toast.LENGTH_SHORT ).show()
  f=io.open(luaconf,"wb")
  f:write( string.format("luapath=%q\nlast=%d",luapath, editor. getSelectionEnd() ))
  f:close()
end

--创建对话框
navi_dlg=Dialog(activity)
navi_dlg.setTitle("导航")
navi_list=ListView(activity)
navi_list.onItemClick=function(parent, v, pos,id)
  editor.setSelection(indexs[pos+1])
  navi_dlg.hide()
end
navi_dlg.setContentView(navi_list)

open_dlg=AlertDialogBuilder(activity)
open_dlg.setTitle("打开")
open_title=TextView(activity)
listview=open_dlg.ListView
listview.addHeaderView(open_title)
listview.setOnItemClickListener(AdapterView.OnItemClickListener{
  onItemClick=function(parent, v, pos,id)
    open(v.Text)
  end
})
--open_dlg.setItems{"空"}
--open_dlg.setContentView(listview)


open_dlg2=AlertDialogBuilder(activity)
open_dlg2.setTitle("打开工程")
listview2=open_dlg2.ListView
listview2.setOnItemClickListener(AdapterView.OnItemClickListener{
  onItemClick=function(parent, v, pos,id)
    luadir=luaprojectdir..tostring(v.Text).."/"
    read(string.format("%smain.lua",luadir))
    open_dlg2.hide()
    Toast.makeText(activity, "打开工程."..tostring(v.Text), Toast.LENGTH_SHORT ).show()
  end
})
--open_dlg2.setItems{"空"}

--open_dlg2.setContentView(listview2)


create_dlg=AlertDialogBuilder(activity)
create_dlg.setMessage(luadir)
create_dlg.setTitle("新建")
create_e=EditText(activity)
create_dlg.setView(create_e)
create_dlg.setPositiveButton(".lua",{onClick=create_lua})
create_dlg.setNegativeButton("取消",nil)
create_dlg.setNeutralButton(".aly",{onClick=create_aly})

project_dlg=AlertDialogBuilder(activity)
project_dlg.setTitle("新建工程")
project_dlg.setView(loadlayout(layout.project))
project_dlg.setPositiveButton("确定",{onClick=create_project})
project_dlg.setNegativeButton("取消",nil)


build_dlg=AlertDialogBuilder(activity)
build_dlg.setTitle("打包")
build_dlg.setView(loadlayout(layout.build))
build_dlg.setPositiveButton("确定",{onClick=buildfile})
build_dlg.setNegativeButton("取消",nil)

bin_dlg=ProgressDialog(activity);
bin_dlg.setTitle("正在打包");
bin_dlg.setMax(100);


lastclick=os.time()-2
function onKeyDown(e)
  local now=os.time()
  if e==4 then
    if now-lastclick>2 then
      --print("再按一次退出程序")
      Toast.makeText(activity, "再按一次退出程序.", Toast.LENGTH_SHORT ).show()
      lastclick=now
      return true
    end
  end
end
local cd1=ColorDrawable(0x00ffffff)
local cd2=ColorDrawable(0x88000088)

local pressed = android.R.attr.state_pressed;
local window_focused = android.R.attr.state_window_focused;
local focused = android.R.attr.state_focused;
local selected = android.R.attr.state_selected;

function click(v)
  editor.paste(v.Text)
end

function newButton(text)
  local sd=StateListDrawable()
  sd.addState({pressed},cd2)
  sd.addState({0},cd1)
  local btn=TextView()
  btn.TextSize=20;
  local pd=btn.TextSize/2
  btn.setPadding(pd,pd/2,pd,pd/4)
  btn.Text=text
  btn.setBackground(sd)
  btn.onClick=click
  return btn
end
local ps={"(",")","[","]","{","}","\"","=",":",".",",","_","+","-","*","/","\\","%","#","^","$","<",">","~"};
for k,v in ipairs(ps) do
  ps_bar.addView(newButton(v))
end

