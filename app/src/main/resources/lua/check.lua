function reg(env,id,key,field)
  local m=getmetatable(env)
  if m==nil or m.__env==nil then
    error(string.format("can not reg then table %s",env),2)
  end
  m.__env[key]={
    id=id,
    field=field
  }
end

function unreg(key)
  local m=getmetatable(env)
  if m==nil or m.__env==nil then
    error(string.format("can not unreg then table %s",env),2)
  end
  m.__env[key]=nil
end

function new(env,k,t,v)
  local m=getmetatable(env)
  if m==nil or m.__env==nil then
    error(string.format("can not new value %s %s %s",k,t,v),2)
  end
  if t==nil then
    error(string.format("%q no type",k),2)
  end
  m.__env[k]={
    type=t or type(v),
    value=v
  }
end

function final(env,k,t,v)
  local m=getmetatable(env)
  if m==nil or m.__env==nil then
    error(string.format("can not final value %s %s %s",k,t,v),2)
  end
  local _env=m.__env
  if _env[k] then
    _env[k].final=true
    return
  end
  if t==nil then
    error(string.format("%q no type",k),2)
  end
  _env[k]={
    type=t or type(v),
    value=v,
    final=true
  }
end

function check(e)
  local env=e
  local _env={}
  local old=getmetatable(env)
  setmetatable(env,{
    __old=old,
    __env=_env,
    __newindex=function(t,k,v)
      local e=_env[k]
      if e then
        if e.id then
          e.value=v
          e.id[e.field]=v
        elseif e.type then
          if e.final and e.value then
            error(string.format("\"%s\" is final,value=%s",k,e.value),2)
          end
          if type(v) == e.type then
            e.value=v
          else
            error(string.format("type error \"%s\",%s (%s expected, got %s)",k,v,e.type,type(v)),2)
          end
        end
      elseif old and old.__newindex then
        old.__newindex(t,k,v)
      else
        _env[k]={
          type=type(v),
          value=v
        }
      end
    end,
    __index=function(t,k)
      local e=_env[k]
      if e then
        return _env[k].value
      elseif old and old.__index then
        return old.__index(t,k)
      end
    end
  })
end

function uncheck(e)
  local m=getmetatable(e)
  if m then
    setmetatable(e,m.__old)
  end
end