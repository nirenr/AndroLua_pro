require "import"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "java.io.File"
import "layout"
import "item"
import "autotheme"

activity.setTitle('工程')
activity.setTheme(autotheme())
activity.setContentView(loadlayout(layout))
local luadir,luapath=...
local plugindir=activity.getLuaExtDir("project")

local function getinfo(dir)
  local app={}
  loadfile(plugindir.."/"..dir.."/init.lua","bt",app)()
  return app
end

local pds=File(plugindir).list()
--Arrays.sort(pds)
local pls={}

for n=0,#pds-1 do
  local s,i=pcall(getinfo,pds[n])
  if s then
    i.path="/"..pds[n]
    table.insert(pls,i) end
end

function sort(a,b)
  return string.lower(a.appname) < string.lower(b.appname)
end

table.sort(pls,sort)
function checkicon(i)
  --i=plugindir.."/"..pps[i].."/icon.png"
  local f=io.open(i)
  if f then
    f:close()
    return i
  else
    return R.drawable.icon
  end
end

adp=LuaAdapter(activity,item)
for k,v in ipairs(pls) do
  local i=plugindir..v.path.."/icon.png"
  adp.add{icon=checkicon(i),title=v.appname.." "..v.appver,path=v.path,description=v.description or ""}
end
_adp=adp
plist.Adapter=adp
plist.onItemClick=function(l,v,p,i)
  --activity.overridePendingTransition(android.R.anim.slide_in_left, android.R.anim.slide_out_right);	
  activity.result({plugindir..v.Tag.path.Text})
end

function shortcut(path)
  import "android.content.Intent"
  import "android.net.Uri"
  intent = Intent(); 
  intent.setClass(activity, activity.getClass());
  intent.setData(Uri.parse("file://"..path))
  addShortcut = Intent("com.android.launcher.action.INSTALL_SHORTCUT"); 
  icon = Intent.ShortcutIconResource.fromContext(activity, 
  R.drawable.icon); 
  addShortcut.putExtra(Intent.EXTRA_SHORTCUT_NAME, "工程列表"); 
  addShortcut.putExtra(Intent.EXTRA_SHORTCUT_INTENT, intent); 
  addShortcut.putExtra("duplicate", 0); 
  addShortcut.putExtra(Intent.EXTRA_SHORTCUT_ICON_RESOURCE, icon); 
  activity.sendBroadcast(addShortcut); 
end
--shortcut(activity.getLuaPath())


function update(pls)
  local adp=LuaAdapter(activity,item)
  for k,v in ipairs(pls) do
    local i=plugindir..v.path.."/icon.png"
    adp.add{icon=checkicon(i),title=v.appname.." "..v.appver,path=v.path,description=v.description or ""}
  end
  plist.Adapter=adp
end

function onCreateOptionsMenu(menu)
  local item=menu.add("搜索")
  item.setShowAsAction(1)
  item.setActionView(edit)
end

edit=EditText()
edit.Hint="输入关键字"
edit.Width=activity.Width/2
edit.SingleLine=true
edit.addTextChangedListener{
  onTextChanged=function(c)
    local s=tostring(c)
    if #s==0 then
      plist.Adapter=adp
      return
    end
    local t={}
    s=s:lower()
    for k,v in ipairs(pls) do
      if v.appname:lower():find(s,1,true) then
        table.insert(t,v)
      end
    end
    update(t)
  end
}

