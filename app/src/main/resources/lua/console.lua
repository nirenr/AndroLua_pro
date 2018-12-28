module(...,package.seeall)
--by nirenr

local function ps(str)
  str = str:gsub("%b\"\"",""):gsub("%b\'\'","")
  local _,f= str:gsub ('%f[%w]function%f[%W]',"")
  local _,t= str:gsub ('%f[%w]then%f[%W]',"")
  local _,i= str:gsub ('%f[%w]elseif%f[%W]',"")
  local _,d= str:gsub ('%f[%w]do%f[%W]',"")
  local _,e= str:gsub ('%f[%w]end%f[%W]',"")
  local _,r= str:gsub ('%f[%w]repeat%f[%W]',"")
  local _,u= str:gsub ('%f[%w]until%f[%W]',"")
  local _,a= str:gsub ("{","")
  local _,b= str:gsub ("}","")
  return (f+t+d+r+a)*4-(i+e+u+b)*4
end


local function _format()
  local p=0
  return function(str)
    str=str:gsub("[ \t]+$","")
    str=string.format('%s%s',string.rep(' ',p),str)
    p=p+ps(str)
    return str
  end
end


function format(Text)
  local t=os.clock()
  local Format=_format()
  Text=Text:gsub('[ \t]*([^\r\n]+)',function(str)return Format(str)end)
  print('操作完成,耗时:'..os.clock()-t)
  return Text
end


function build(path)
  if path then
    local str,st=loadfile(path)
    if st then
      return nil,st
    end
    local path=path..'c'

    local st,str=pcall(string.dump,str,true)
    if st then
      f=io.open(path,'wb')
      f:write(str)
      f:close()
      return path
    else
      os.remove(path)
      return nil,str
    end
  end
end

function build_aly(path2)
  if path2 then
    local f,st=io.open(path2)
    if st then
      return nil,st
    end
    local str=f:read("*a")
    f:close()
    str=string.format("local layout=%s\nreturn layout",str)
    local path=path2..'c'
    str,st=loadstring(str,path2:match("[^/]+/[^/]+$"),"bt")
    if st then
      return nil,st:gsub("%b[]",path2,1)
    end

    local st,str=pcall(string.dump,str,true)
    if st then
      f=io.open(path,'wb')
      f:write(str)
      f:close()
      return path
    else
      os.remove(path)
      return nil,str
    end
  end
end

