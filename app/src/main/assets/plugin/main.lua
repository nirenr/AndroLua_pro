require "import"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
import "java.io.File"
import "layout"
import "item"
import "autotheme"

activity.setTitle('插件')
activity.setTheme(autotheme())
activity.setContentView(loadlayout(layout))
local luadir,luapath=...
local plugindir=activity.getLuaExtDir("plugin")

local function getinfo(dir)
  local app={}
  loadfile(plugindir.."/"..dir.."/init.lua","bt",app)()
  return app
end

local pds=File(plugindir).list()
Arrays.sort(pds)
local pls={}
local pps={}
for n=0,#pds-1 do
  local s,i=pcall(getinfo,pds[n])
  if s then
    table.insert(pls,i)
    table.insert(pps,pds[n])
  end
end

function checkicon(i)
  i=plugindir.."/"..pps[i].."/icon.png"
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
  adp.add{icon=checkicon(k),title=v.appname.." "..v.appver,description=v.description or ""}
end
plist.Adapter=adp
plist.onItemClick=function(l,v,p,i)
  activity.newActivity(plugindir.."/"..pps[p+1].."/main.lua",{luadir,luapath})
end

