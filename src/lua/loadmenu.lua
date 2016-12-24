local require=require
local table=require "table"
luajava.package=luajava.package or {}
luajava.loaded=luajava.loaded or {}
luajava.ids=luajava.ids or {id=0x7f000000}
local packages = luajava.package
local loaded = luajava.loaded
local ids = luajava.ids
local _G=_G
local insert = table.insert
local new = luajava.new
local bindClass = luajava.bindClass
local LuaDrawable=luajava.bindClass "com.androlua.LuaDrawable"
local loadbitmap=require "loadbitmap"

local function loadmenu(menu,t,root)
    root=root or _G
    for k,v in ipairs(t) do
      local id=ids.id
      ids.id=ids.id+1
      if v[1]== MenuItem then
        local item=menu.add(v.group or 0,id,v.order or 0,v.title)
        if v.id then
          rawset(root,v.id,item)
          ids[v.id]=id
        end
        item.setShowAsAction(1)
        if v.icon then
          item.setIcon(BitmapDrawable(loadbitmap(v.icon)))
        end
        if v.enabled==false then
          item.setEnabled(v.enabled)
        end
        if v.visible==false then
          item.setVisible(v.visible)
        end
      elseif v[1]== SubMenu then
        local item=menu.addSubMenu(v.group or 0,id,v.order or 0,v.title)
        item.HeaderTitle=v.title
        loadmenu(item,v,root)
      end
    end
  end
  return loadmenu

