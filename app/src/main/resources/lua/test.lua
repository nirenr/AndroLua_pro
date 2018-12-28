local Log=luajava.bindClass "android.util.Log"
local config={}
--添加要调试的函数
local func={}

local NIL={}
setmetatable(NIL,{__tostring=function()return "nil" end})
local function printstack(mask,line)
  local m=2
  local dbs={}
  local info=debug.getinfo(m)
  if info==nil then
    return
  end
  if not line and func and func[info.name]==nil then
    return
  end
  dbs.info=info
  local func=info.func
  local nups=info.nups
  local ups={}
  dbs.upvalues=ups
  for n=1,nups do
    local n,v=debug.getupvalue(func,n)
    if v==nil then
      v=NIL
    end
    if string.byte(n)==40 then
      if ups[n]==nil then
        ups[n]={}
      end
      table.insert(ups[n],v)
    else
      ups[n]=v
    end
  end

  local lps={}
  dbs.localvalues=lps
  lps.vararg={}
  --lps.temporary={}
  for n=-1,-255,-1 do
    local k,v=debug.getlocal(m,n)
    if k==nil then
      break
    end
    if v==nil then
      v=NIL
    end
    table.insert(lps.vararg,v)
  end
  for n=1,255 do
    local n,v=debug.getlocal(m,n)
    if n==nil then
      break
    end
    if v==nil then
      v=NIL
    end
    if string.byte(n)==40 then
      if lps[n]==nil then
        lps[n]={}
      end
      table.insert(lps[n],v)
    else
      lps[n]=v
    end
  end

  Log.i("test",string.format("%s %s\n%s",mask,line or info.name, dump(dbs)))
end

--print(string.byte("("))
if pcall(loadfile(activity.LuaDir.."/.test.lua","bt",config)) then
  if config.line then
    debug.sethook(printstack,"crl",config.line)
  else
    debug.sethook(printstack,"cr")
  end
  func={}
  for k,v in ipairs(config.func) do
    func[v]=true
  end
end
