require "import"
import "android.app.*"
import "android.os.*"
import "android.widget.*"
import "android.view.*"
require "permission"
import "layout"
import "autotheme"

activity.setTitle('工程属性')
activity.setTheme(autotheme())
activity.setContentView(loadlayout(layout))
projectdir=...
luaproject=projectdir.."/init.lua"
app={}
loadfile(luaproject,"bt",app)()
appname.Text=app.appname or "AndroLua"
appver.Text=app.appver or "1.0"
packagename.Text=app.packagename or "com.androlua"
developer.Text=app.developer or ""
description.Text=app.description or ""
debugmode.Checked=app.debugmode==nil or app.debugmode

plist.ChoiceMode=ListView.CHOICE_MODE_MULTIPLE;
pss={}
ps={}
for k,v in pairs(permission_info) do
  table.insert(ps,k)
end
table.sort(ps)

for k,v in ipairs(ps) do
  table.insert(pss,permission_info[v])
end

adp=ArrayListAdapter(activity,android.R.layout.simple_list_item_multiple_choice,String(pss))
plist.Adapter=adp

pcs={}
for k,v in ipairs(app.user_permission or {}) do
  pcs[v]=true
end
for k,v in ipairs(ps) do
  if pcs[v] then
    plist.setItemChecked(k-1,true)
  end
end

local fs=luajava.astable(android.R.style.getFields())
local tss={"Theme"}
for k,v in ipairs(fs) do
  local nm=v.Name
  if nm:find("^Theme_") then
    table.insert(tss,nm)
  end
end

local tadp=ArrayAdapter(activity,android.R.layout.simple_list_item_1, String(tss))
tlist.Adapter=tadp

for k,v in ipairs(tss) do
  if v==app.theme then
    tlist.setSelection(k-1)
  end
end

function callback(c,j)
  print(dump(j))
end

local template=[[
appname="%s"
appver="%s"
packagename="%s"
theme="%s"
developer="%s"
description="%s"
debugmode=%s
user_permission={
  %s
}
]]
local function dump(t)
  for k,v in ipairs(t) do
    t[k]=string.format("%q",v)
  end
  return table.concat(t,",\n  ")
end

function onCreateOptionsMenu(menu)
  menu.add("保存").setShowAsAction(1)
end

function onOptionsItemSelected(item)
  if appname.Text=="" or appver.Text=="" or packagename.Text=="" or developer.Text=="" or description.Text=="" then
    Toast.makeText(activity,"项目不能为空",500).show()
    return true
  end

  local cs=plist.getCheckedItemPositions()
  local rs={}
  for n=1,#ps do
    if cs.get(n-1) then
      table.insert(rs,ps[n])
    end
  end
  local thm=tss[tlist.getSelectedItemPosition()+1]
  local ss=string.format(template,appname.Text,appver.Text,packagename.Text,thm,developer.Text,description.Text,debugmode.isChecked(),dump(rs))
  local f=io.open(luaproject,"w")
  f:write(ss)
  f:close()
  Toast.makeText(activity, "已保存.", Toast.LENGTH_SHORT ).show()
  activity.result({appname.Text})

end

lastclick=os.time()-2
function onKeyDown(e)
  local now=os.time()
  if e==4 then
    if now-lastclick>2 then
      Toast.makeText(activity, "再按一次返回.", Toast.LENGTH_SHORT ).show()
      lastclick=now
      return true
    end
  end
end
