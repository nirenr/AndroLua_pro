require "import"
import "android.widget.*"
import "android.view.*"
import "android.app.*"
local classes=require "android"
import "clayout"
import "mlayout"
import "autotheme"

activity.Title="Java API浏览器"
activity.setTheme(autotheme())
function adapter(t)
  local ls=ArrayList()
  for k,v in ipairs(t) do
    ls.add(v)
  end
  return ArrayAdapter(activity,android.R.layout.simple_list_item_1, ls)
end

import "android.content.*"
cm=activity.getSystemService(activity.CLIPBOARD_SERVICE)

function copy(str)
  local cd = ClipData.newPlainText("label",str)
  cm.setPrimaryClip(cd)
  Toast.makeText(activity,"已复制的剪切板",1000).show()
end


dlg=Dialog(activity,autotheme())
dlg.setContentView(loadlayout(mlayout))
curr_class=nil
curt_adapter=nil
activity.setContentView(clayout)
clist.setAdapter(adapter(classes))

clist.onItemClick=function(l,v)
  local s=tostring(v.Text)
  local class=luajava.bindClass(s)
  curr_class=class
  local t={}
  local fs={}
  local ms={}
  local es={}
  local ss={}
  local gs={}
  local super=class.getSuperclass()
  super=super and " extends "..tostring(super.getName()) or ""
  table.insert(t,tostring(class)..super)

  table.insert(t,"构建方法")
  local cs=class.getConstructors()
  for n=0,#cs-1 do
    table.insert(t,tostring(cs[n]))
  end



  curr_ms=class.getMethods()
  for n=0,#curr_ms-1 do
    local str=tostring(curr_ms[n])
    table.insert(ms,str)
    local e1=str:match("%.setOn(%a+)Listener")
    local s1,s2=str:match("%.set(%a+)(%([%a$%.]+%))")
    local g1,g2=str:match("([%a$%.]+) [%a$%.]+%.get(%a+)%(%)")
    if e1 then
      table.insert(es,"on"..e1)
    elseif s1 then
      table.insert(ss,s1..s2)
    end
    if g1 then
      table.insert(gs,string.format("(%s)%s",g1,g2))
    end
  end
  table.insert(t,"公有事件")
  for k,v in ipairs(es) do
    table.insert(t,v)
  end
  table.insert(t,"公有getter")
  for k,v in ipairs(gs) do
    table.insert(t,v)
  end
  table.insert(t,"公有setter")
  for k,v in ipairs(ss) do
    table.insert(t,v)
  end

  curr_fs=class.getFields()
  table.insert(t,"公有字段")
  for n=0,#curr_fs-1 do
    table.insert(t,tostring(curr_fs[n]))
  end

  table.insert(t,"公有方法")
  for k,v in ipairs(ms) do
    table.insert(t,v)
  end

  dlg.Title=tostring(s)
  curt_adapter=adapter(t)
  mlist.setAdapter(curt_adapter)
  dlg.show()
end

clist.onItemLongClick=function(l,v)
  local s=tostring(v.Text)
  copy(s)
  return true
end

mlist.onItemLongClick=function(l,v)
  local s=tostring(v.Text)
  if s:find("%(") then
    s=s:match("(%w+)%(")
  else
    s=s:match("(%w+)$")
  end
  copy(s)
  return true
end

medit.addTextChangedListener{
  onTextChanged=function(c)
    local s=tostring(c)
    if #s==0 then
      mlist.setAdapter(curt_adapter)
      return true
    end
    local class=curr_class
    local t={}
    local fs=curr_fs
    table.insert(t,"公有字段")
    for n=0,#fs-1 do
      if fs[n].Name:find(s,1,true) then
        table.insert(t,tostring(fs[n]))
      end
    end
    local ms=curr_ms
    table.insert(t,"公有方法")
    for n=0,#ms-1 do
      if ms[n].Name:find(s,1,true) then
        table.insert(t,tostring(ms[n]))
      end
    end
    mlist.setAdapter(adapter(t))
  end
}

edit.addTextChangedListener{
  onTextChanged=function(c)
    local s=tostring(c)
    if #s==0 then
      clist.setAdapter(adapter(classes))
    end
    local t={}
    for k,v in ipairs(classes) do
      if v:find(s,1,true) then
        table.insert(t,v)
      end
    end
    clist.setAdapter(adapter(t))
  end
}
