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
local theme
if h<=6 or h>=22 then
  theme=activity.getLuaExtDir("fonts").."/night.lua"
else
  theme=activity.getLuaExtDir("fonts").."/day.lua"
end
local p={}
local e=pcall(loadfile(theme,"bt",p))
if e then
  for k,v in pairs(p) do
    layout.main[2][k]=v
  end
end
activity.getWindow().setSoftInputMode(0x10)

--activity.getActionBar().show()
luadir=luajava.luaextdir.."/" or "/sdcard/androlua/"
luaconf=luajava.luadir.."/lua.conf"
luaproj=luajava.luadir.."/lua.proj"
pcall(dofile,luaconf)
luapath=luapath or luadir.."new.lua"
luadir=luapath:match("^(.-)[^/]+$")
pcall(dofile,luaproj)
luaproject=luaproject
if luaproject then
  local p={}
  local e=pcall(loadfile(luaproject.."init.lua","bt",p))
  if e then
    activity.setTitle(p.appname)
    Toast.makeText(activity, "打开工程."..p.appname, Toast.LENGTH_SHORT ).show()
  end
end

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
    {MenuItem,
      title="属性",
      id="project_info",},
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
  {MenuItem,
    title="插件...",
    id="plugin",},
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
  },
}
optmenu={}
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
  bin_dlg.hide()
  bin_dlg.Message=""
  if not s:find("成功") then
    create_error_dlg()
    error_dlg.Message=s
    error_dlg.show()
  end
end

function reopen(path)
  local f=io.open(path,"r")
  if f then
    local str=f:read("*all")
    if tostring(editor.getText())~=str then
      editor.setText(str,true)
    end
    f:close()
  end
end

function read(path)
  local f=io.open(path,"r")
  if f==nil then
    --Toast.makeText(activity, "打开文件出错."..path, Toast.LENGTH_LONG ).show()
    error()
    return
  end
  local str=f:read("*all")
  f:close()
  if string.byte(str,1)==0x1b then
    Toast.makeText(activity, "不能打开已编译文件."..path, Toast.LENGTH_LONG ).show()
    return 
  end
  editor.setText(str)
  activity.getActionBar().setSubtitle(".."..path:match("(/[^/]+/[^/]+)$"))
  luapath=path
  write(luaconf,string.format("luapath=%q",path))
  if luaproject and path:find(luaproject,1,true) then
    --Toast.makeText(activity, "打开文件."..path, Toast.LENGTH_SHORT ).show()
    return
  end

  local p={}
  local e=pcall(loadfile(luadir.."init.lua","bt",p))
  if e then
    activity.setTitle(p.appname)
    luaproject=luadir
    write(luaproj,string.format("luaproject=%q",luaproject))
    --Toast.makeText(activity, "打开工程."..p.appname, Toast.LENGTH_SHORT ).show()
  else
    activity.setTitle("AndroLua+")
    luaproject=nil
    write(luaproj,"luaproject=nil")
    --Toast.makeText(activity, "打开文件."..path, Toast.LENGTH_SHORT ).show()
  end
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
  local str=""
  local f=io.open(luapath,"r")
  if f then
    str=f:read("*all")
    f:close()
  end
  local src=editor.getText().toString()
  if src~=str then
    write(luapath,src)
  end
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
    imports(luadir..p)
    open_dlg.hide()
  else
    read(luadir..p)
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

function adapter(t)
  return ArrayListAdapter(activity,android.R.layout.simple_list_item_1, String(t))
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
  local adapter=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String(td))
  v.setAdapter(adapter)
  plist=td
  --open_dlg2.setItems(td)
end


bin=function(luapath,appname,appver,packagename,apkpath)
  require "import"
  import "console"
  compile "mao"
  compile "sign"
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
  replace={}
  checked={}

  local libs=File(activity.ApplicationInfo.nativeLibraryDir).list()
  libs=luajava.astable(libs)
  for k,v in ipairs(libs) do
    libs[k]="lib/armeabi/"..libs[k]
    replace[libs[k]]=true
  end
  local mdp=activity.Application.MdDir
  function getmodule(dir)
    local mds=File(activity.Application.MdDir..dir).listFiles()
    mds=luajava.astable(mds)
    for k,v in ipairs(mds) do
      if mds[k].isDirectory() then
        getmodule(dir..mds[k].Name.."/")
      else
        mds[k]="lua"..dir..mds[k].Name
        replace[mds[k]]=true
      end
    end
  end
  getmodule("/")

  function checklib(path)
    if checked[path] then
      return
    end
    local cp,lp
    checked[path]=true
    local f=io.open(path)
    local s=f:read("*a")
    f:close()
    for m,n in s:gmatch("require *%(?\"([%w_]+)%.?([%w_]*)") do
      cp=string.format("lib/armeabi/lib%s.so",m)
      if n~="" then
        lp=string.format("lua/%s/%s.lua",m,n)
      else
        lp=string.format("lua/%s.lua",m)
      end
      if replace[cp] then
        replace[cp]=false
      end
      if replace[lp] then
        checklib(mdp.."/"..m..".lua")
        replace[lp]=false
      end
    end
    for m,n in s:gmatch("import *\"([%w_]+)%.?([%w_]*)") do
      cp=string.format("lib/armeabi/lib%s.so",m)
      if n~="" then
        lp=string.format("lua/%s/%s.lua",m,n)
      else
        lp=string.format("lua/%s.lua",m)
      end
      if replace[cp] then
        replace[cp]=false
      end
      if replace[lp] then
        checklib(mdp.."/"..m..".lua")
        replace[lp]=false
      end
    end
  end

  replace["lib/armeabi/libluajava.so"]= false




  function addDir(out,dir,f)
    local ls=f.listFiles()
    for n=0,#ls-1 do
      local name=ls[n].getName()
      if name:find("%.apk$") or name:find("%.luac$") or name:find("^%.") then
      elseif name:find("%.lua$") then
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
      else
        entry=ZipEntry("assets/"..dir..name)
        out.putNextEntry(entry)
        replace["assets/"..dir..name]=true
        copy(FileInputStream(ls[n]),out)
      end
    end
  end

  local function check(h)
    local function hash(s)
      local l=0
      local b={string.byte(s,1,-1)}
      for k,v in ipairs(b) do
        l=l+k+v
      end
      return l
    end
    require "init"
    for k,v in pairs(h) do
      if hash(_G[k])~=v then
        os.exit(0)
      end
    end
  end

  local function hash(s)
    local l=0
    local b={string.byte(s,1,-1)}
    for k,v in ipairs(b) do
      l=l+k+v
    end
    return l
  end

  local p={}
  local e=pcall(loadfile(luapath.."init.lua","bt",p))
  local t={}
  table.insert(t,"\n\nlocal ____h={")
  for k,v in pairs(p) do
    if type(v)=="string" then
      table.insert(t,string.format("%s=%d,",k,hash(v)))
    end
  end
  table.insert(t,"}\n")
  table.insert(t,string.format("loadstring(%q)(____h)\n\n",string.dump(check,true)))
  local f1=io.open(luapath.."main.lua")
  local s1=f1:read("a")
  f1:close()
  function addcheck()
    local f=io.open(luapath.."main.lua","w")
    f:write(s1..table.concat(t))
    f:close()
  end

  function removecheck()
    local f=io.open(luapath.."main.lua","w")
    f:write(s1)
    f:close()
  end

  --addcheck()

  this.update("正在编译...");
  if f.isDirectory() then
    require "permission"
    dofile(luapath.."init.lua")
    if user_permission then
      for k,v in ipairs(user_permission) do
        user_permission[v]=true
      end
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
    return "error"
  end
  --removecheck()

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
          elseif user_permission then
            local p=v:match("%.permission%.([%w_]+)$")
            if p and (not user_permission[p]) then
              list.set(n,"")
            end
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
  local date=os.date("%y%m%d%H%M%S")
  local tmp=activity.getLuaExtDir("backup").."/"..f.Name.."_"..date..".alp"
  local p={}
  local e,s=pcall(loadfile(f.Path.."/init.lua","bt",p))
  if e then
    if p.mode=="plugin" then
      tmp=string.format("%s/%s_plugin_%s-%s.alp",activity.getLuaExtDir("backup"),p.appname,p.appver:gsub("%.","_"),date)
    else
      tmp=string.format("%s/%s_%s-%s.alp",activity.getLuaExtDir("backup"),p.appname,p.appver:gsub("%.","_"),date)
    end
  end
  local out=ZipOutputStream(FileOutputStream(tmp))

  function addDir(out,dir,f)
    local ls=f.listFiles()
    --entry=ZipEntry(dir)
    --out.putNextEntry(entry)
    for n=0,#ls-1 do
      local name=ls[n].getName()
      if name:find("%.apk$") or name:find("%.luac$") or name:find("^%.") then
      elseif p.mode and name:find("%.lua$") and name~="init.lua" then
        local path,err=console.build(ls[n].Path)
        if path then
          entry=ZipEntry(dir..name)
          out.putNextEntry(entry)
          copy(FileInputStream(File(path)),out)
          os.remove(path)
        else
          error(err)
        end
      elseif p.mode and name:find("%.aly$") then
        name=name:gsub("aly$","lua")
        local path,err=console.build_aly(ls[n].Path)
        if path then
          entry=ZipEntry(dir..name)
          out.putNextEntry(entry)
          copy(FileInputStream(File(path)),out)
          os.remove(path)
        else
          error(err)
        end
      elseif ls[n].isDirectory() then
        addDir(out,dir..name.."/",ls[n])
      else
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

function getalpinfo(path)
  local app={}
  loadstring(tostring(String(LuaUtil.readZip(path,"init.lua"))),"bt","bt",app)()
  local str=string.format("名称: %s\
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
  return str,app.mode
end

function imports(path)
  create_imports_dlg()
  local mode
  imports_dlg.Message,mode=getalpinfo(path)
  if mode=="plugin" or path:match("^([^%._]+)_plugin") then
    imports_dlg.setTitle("导入插件")
  elseif mode=="build" or path:match("^([^%._]+)_build") then
    imports_dlg.setTitle("打包安装")
  end
  imports_dlg.show()
end

function importx(path,tp)
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
  local app={}
  loadstring(tostring(String(LuaUtil.readZip(path,"init.lua"))),"bt","bt",app)()

  local s=app.appname or f.Name:match("^([^%._]+)")
  local out=activity.getLuaExtDir("project").."/"..s

  if tp=="build" then
    out=activity.getLuaExtDir("bin/.temp").."/"..s
  elseif tp=="plugin" then
    out=activity.getLuaExtDir("plugin").."/"..s
  end
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
  function callback2(s)
    LuaUtil.rmDir(File(activity.getLuaExtDir("bin/.temp")))
    bin_dlg.hide()
    bin_dlg.Message=""
    if not s:find("成功") then
      create_error_dlg()
      error_dlg.Message=s
      error_dlg.show()
    end
  end

  if tp=="build" then
    local p={}
    local e,s=pcall(loadfile(out.."init.lua","bt",p))
    if e then
      activity.newTask(bin,update,callback2).execute{out,p.appname,p.appver,p.packagename,luabindir..p.appname.."_"..p.appver..".apk"}
      create_bin_dlg()
      bin_dlg.show()
    else
      Toast.makeText(activity, "工程配置文件错误."..s, Toast.LENGTH_SHORT ).show()
    end
    return out
  elseif tp=="plugin" then
    Toast.makeText(activity, "导入插件."..s, Toast.LENGTH_SHORT ).show()
    return out
  end
  luadir=out
  luapath=luadir.."main.lua"
  read(luapath)
  Toast.makeText(activity, "导入工程."..luadir, Toast.LENGTH_SHORT ).show()
  return out
end

func={}
func.open=function()
  save()
  create_open_dlg()
  list(listview, luadir)
  open_dlg.show()
end
func.new=function()
  save()
  create_create_dlg()
  create_dlg.setMessage(luadir)
  create_dlg.show()
end

func.create=function()
  save()
  create_project_dlg()
  project_dlg.show()
end
func.openproject=function()
  save()
  create_open_dlg2()
  open_edit=""
  list2(listview2, luaprojectdir)
  open_dlg2.show()
end

func.export=function()
  save()
  if luaproject then
    local name=export(luaproject)
    Toast.makeText(activity, "工程已导出."..name, Toast.LENGTH_SHORT ).show()
  else
    Toast.makeText(activity, "仅支持工程导出.", Toast.LENGTH_SHORT ).show()
  end
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
  if luaproject then
    activity.newActivity(luaproject.."main.lua")
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
  create_navi_dlg()
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
  if not luaproject then
    Toast.makeText(activity, "仅支持工程打包.", Toast.LENGTH_SHORT ).show()
    return
  end

  local p={}
  local e,s=pcall(loadfile(luaproject.."/init.lua","bt",p))
  if e then
    activity.newTask(bin,update,callback).execute{luaproject,p.appname,p.appver,p.packagename,luabindir..p.appname.."_"..p.appver..".apk"}
    create_bin_dlg()
    bin_dlg.show()
  else
    Toast.makeText(activity, "工程配置文件错误."..s, Toast.LENGTH_SHORT ).show()
  end
end

buildfile=function()
  Toast.makeText(activity, "正在打包..", Toast.LENGTH_SHORT ).show()
  task(bin,luaPath.getText().toString(),appName.getText().toString(),appVer.getText().toString(),packageName.getText().toString(),apkPath.getText().toString(),function(s)status.setText(s or "打包出错!")end)
end

func.info=function()
  if not luaproject then
    Toast.makeText(activity, "仅支持修改工程属性.", Toast.LENGTH_SHORT ).show()
    return
  end
  activity.newActivity("projectinfo/main",{luaproject})
end

func.logcat=function()
  activity.newActivity("logcat")
end

func.help=function()
  activity.newActivity("help")
end

func.java=function()
  activity.newActivity("javaapi/main")
end

func.manual=function()
  activity.newActivity("luadoc")
end

func.helper=function()
  save()
  isupdate=true
  activity.newActivity("layouthelper/main",{luaproject,luapath})
end

func.donation=function()
  xpcall(function()
    local url="alipayqr://platformapi/startapp?saId=10000007&clientVersion=3.7.0.0718&qrcode=https://qr.alipay.com/apt7ujjb4jngmu3z9a"
    activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
  end,
  function()
    local url = "https://qr.alipay.com/apt7ujjb4jngmu3z9a";
    activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
  end)
end

func.qq=function()
  local url="mqqwpa://im/chat?chat_type=wpa&uin=946049229"
  activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
end

func.bbs=function()
  local url="http://androlua.com"
  activity.startActivity(Intent(Intent.ACTION_VIEW, Uri.parse(url)));
end

func.plugin=function()
  activity.newActivity("plugin/main",{luaproject,luapath})
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
    [optmenu.project_info]=func.info,
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
    [optmenu.plugin]=func.plugin,
  }
end

activity.setContentView(layout.main)

function onCreate(s)
  --[[ local intent=activity.getIntent()
  local uri=intent.getData()
  if not s and uri and uri.getPath():find("%.alp$") then
    imports(uri.getPath())
  else]]
  if pcall(read,luapath) then
    last=last or 0
    if last < editor.getText().length() then
      editor.setSelection(last)
    end
  else
    luapath=activity.LuaExtDir.."/new.lua"
    if not pcall(read,luapath) then
      write(luapath,code)
      pcall(read,luapath)
    end
  end
  --end
end

function onNewIntent(intent)
  local uri=intent.getData()
  if uri and uri.getPath():find("%.alp$") then
    imports(uri.getPath())
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
    local classes=require "javaapi.android"
    local c=data:match("a nil value %(global '(%w+)'%)")
    if c then
      local cls={}
      c="%."..c.."$"
      for k,v in ipairs(classes) do
        if v:find(c) then
          table.insert(cls,string.format("import %q",v))
        end
      end
      if #cls>0 then
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
  isupdate=false
end

function onStop()
  save()
  --Toast.makeText(activity, "文件已保存."..luapath, Toast.LENGTH_SHORT ).show()
  f=io.open(luaconf,"wb")
  f:write( string.format("luapath=%q\nlast=%d",luapath, editor. getSelectionEnd() ))
  f:close()
end

--创建对话框
function create_navi_dlg()
  if navi_dlg then
    return
  end
  navi_dlg=Dialog(activity)
  navi_dlg.setTitle("导航")
  navi_list=ListView(activity)
  navi_list.onItemClick=function(parent, v, pos,id)
    editor.setSelection(indexs[pos+1])
    navi_dlg.hide()
  end
  navi_dlg.setContentView(navi_list)
end

function create_imports_dlg()
  if imports_dlg then
    return
  end
  imports_dlg=AlertDialogBuilder(activity)
  imports_dlg.setTitle("导入")
  imports_dlg.setPositiveButton("确定",{
    onClick=function()
      local path=imports_dlg.Message:match("路径: (.+)$")
      if imports_dlg.Title=="打包安装" then
        importx(path,"build")
        imports_dlg.setTitle("导入")
      elseif imports_dlg.Title=="导入插件" then
        importx(path,"plugin")
        imports_dlg.setTitle("导入")
      else
        importx(path)
      end
    end})
  imports_dlg.setNegativeButton("取消",nil)
end

function create_delete_dlg()
  if delete_dlg then
    return
  end
  delete_dlg=AlertDialogBuilder(activity)
  delete_dlg.setTitle("删除")
  delete_dlg.setPositiveButton("确定",{
    onClick=function()
      if luapath:find(delete_dlg.Message) then
        Toast.makeText(activity, "不能删除正在打开的文件.", Toast.LENGTH_SHORT ).show()
      elseif LuaUtil.rmDir(File(delete_dlg.Message)) then
        Toast.makeText(activity, "已删除.", Toast.LENGTH_SHORT ).show()
        list(listview,luadir)
      else
        Toast.makeText(activity, "删除失败.", Toast.LENGTH_SHORT ).show()
      end
    end})
  delete_dlg.setNegativeButton("取消",nil)
end

function create_open_dlg()
  if open_dlg then
    return
  end
  open_dlg=AlertDialogBuilder(activity)
  open_dlg.setTitle("打开")
  open_title=TextView(activity)
  listview=open_dlg.ListView
  listview.FastScrollEnabled=true

  listview.addHeaderView(open_title)
  listview.setOnItemClickListener(AdapterView.OnItemClickListener{
    onItemClick=function(parent, v, pos,id)
      open(v.Text)
    end
  })

  listview.onItemLongClick=function(parent, v, pos,id)
    if v.Text~="../" then
      create_delete_dlg()
      delete_dlg.setMessage(luadir..v.Text)
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
  open_dlg2=AlertDialogBuilder(activity)
  --open_dlg2.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_ALT_FOCUSABLE_IM);

  open_dlg2.setTitle("打开工程")
  open_dlg2.setView(loadlayout(layout.open2))

  --listview2=open_dlg2.ListView
  listview2.FastScrollEnabled=true
  --open_edit=EditText(activity)
  --listview2.addHeaderView(open_edit)

  open_edit.addTextChangedListener{
    onTextChanged=function(c)
      local s=tostring(c)
      if #s==0 then
        listview2.setAdapter(adapter(plist))
      end
      local t={}
      s=s:lower()
      for k,v in ipairs(plist) do
        if v:lower():find(s,1,true) then
          table.insert(t,v)
        end
      end
      listview2.setAdapter(adapter(t))
    end
  }

  listview2.setOnItemClickListener(AdapterView.OnItemClickListener{
    onItemClick=function(parent, v, pos,id)
      luadir=luaprojectdir..tostring(v.Text).."/"
      read(string.format("%smain.lua",luadir))
      open_dlg2.hide()
      Toast.makeText(activity, "打开工程."..tostring(v.Text), Toast.LENGTH_SHORT ).show()
    end
  })
end

function create_create_dlg()
  if create_dlg then
    return
  end
  create_dlg=AlertDialogBuilder(activity)
  create_dlg.setMessage(luadir)
  create_dlg.setTitle("新建")
  create_e=EditText(activity)
  create_dlg.setView(create_e)
  create_dlg.setPositiveButton(".lua",{onClick=create_lua})
  create_dlg.setNegativeButton("取消",nil)
  create_dlg.setNeutralButton(".aly",{onClick=create_aly})
end

function create_project_dlg()
  if project_dlg then
    return
  end
  project_dlg=AlertDialogBuilder(activity)
  project_dlg.setTitle("新建工程")
  project_dlg.setView(loadlayout(layout.project))
  project_dlg.setPositiveButton("确定",{onClick=create_project})
  project_dlg.setNegativeButton("取消",nil)
end

function create_build_dlg()
  if build_dlg then
    return
  end
  build_dlg=AlertDialogBuilder(activity)
  build_dlg.setTitle("打包")
  build_dlg.setView(loadlayout(layout.build))
  build_dlg.setPositiveButton("确定",{onClick=buildfile})
  build_dlg.setNegativeButton("取消",nil)
end

function create_bin_dlg()
  if bin_dlg then
    return
  end
  bin_dlg=ProgressDialog(activity);
  bin_dlg.setTitle("正在打包");
  bin_dlg.setMax(100);
end

import "android.content.*"
cm=activity.getSystemService(activity.CLIPBOARD_SERVICE)

function copy(str)
  local cd = ClipData.newPlainText("label",str)
  cm.setPrimaryClip(cd)
  Toast.makeText(activity,"已复制到剪切板",1000).show()
end

function create_import_dlg()
  if import_dlg then
    return
  end
  import_dlg=AlertDialogBuilder(activity)
  import_dlg.Title="可能需要导入的类"
  import_dlg.setPositiveButton("确定",nil)

  import_dlg.ListView.onItemLongClick=function(l,v)
    copy(v.Text)
    return true
  end
end

function create_error_dlg()
  if error_dlg then
    return
  end
  error_dlg=AlertDialogBuilder(activity)
  error_dlg.Title="出错"
  error_dlg.setPositiveButton("确定",nil)
end

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
local ps={"(",")","[","]","{","}","\"","=",":",".",",","_","+","-","*","/","\\","%","#","^","$","?","<",">","~",";","'"};
for k,v in ipairs(ps) do
  ps_bar.addView(newButton(v))
end

local function adds()
  require "import"
  local classes=require "javaapi.android"
  local ms={"onCreate",
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
  }
  local buf=String[#ms+#classes]
  for k,v in ipairs(ms) do
    buf[k-1]=v
  end
  local l=#ms
  for k,v in ipairs(classes) do
    buf[l+k-1]=string.match(v,"%w+$")
  end
  return buf
end
task(adds,function(buf)editor.addNames(buf)end)

