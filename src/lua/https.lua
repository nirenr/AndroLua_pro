----------------------------------------------------------------------------
-- LuaSec 0.6a
-- Copyright (C) 2009-2015 PUC-Rio
--
-- Author: Pablo Musa
-- Author: Tomas Guisasola
---------------------------------------------------------------------------

local socket = require("socket")
local ssl    = require("ssl")
local ltn12  = require("ltn12")
local http   = require("http")
local url    = require("socket.url")

local try    = socket.try

--
-- Module
--
local _M = {
  _VERSION   = "0.6a",
  _COPYRIGHT = "LuaSec 0.6a - Copyright (C) 2009-2015 PUC-Rio",
  PORT       = 443,
}

-- TLS configuration
local cfg = {
  protocol = "tlsv1",
  options  = "all",
  verify   = "none",
}

--------------------------------------------------------------------
-- Auxiliar Functions
--------------------------------------------------------------------

-- Insert default HTTPS port.
local function default_https_port(u)
   return url.build(url.parse(u, {port = _M.PORT}))
end

-- Convert an URL to a table according to Luasocket needs.
local function urlstring_totable(url, body, result_table)
   url = {
      url = default_https_port(url),
      method = body and "POST" or "GET",
      sink = ltn12.sink.table(result_table)
   }
   if body then
      url.source = ltn12.source.string(body)
      url.headers = {
         ["content-length"] = #body,
         ["content-type"] = "application/x-www-form-urlencoded",
      }
   end
   return url
end

-- Forward calls to the real connection object.
local function reg(conn)
   local mt = getmetatable(conn.sock).__index
   for name, method in pairs(mt) do
      if type(method) == "function" then
         conn[name] = function (self, ...)
                         return method(self.sock, ...)
                      end
      end
   end
end

-- Return a function which performs the SSL/TLS connection.
local function tcp(params)
   params = params or {}
   -- Default settings
   for k, v in pairs(cfg) do 
      params[k] = params[k] or v
   end
   -- Force client mode
   params.mode = "client"
   -- 'create' function for LuaSocket
   return function ()
      local conn = {}
      conn.sock = try(socket.tcp())
      local st = getmetatable(conn.sock).__index.settimeout
      function conn:settimeout(...)
         return st(self.sock, ...)
      end
      -- Replace TCP's connection function
      function conn:connect(host, port)
         try(self.sock:connect(host, port))
         self.sock = try(ssl.wrap(self.sock, params))
         try(self.sock:dohandshake())
         reg(self, getmetatable(self.sock))
         return 1
      end
      return conn
  end
end

--------------------------------------------------------------------
-- Main Function
--------------------------------------------------------------------

-- Make a HTTP request over secure connection.  This function receives
--  the same parameters of LuaSocket's HTTP module (except 'proxy' and
--  'redirect') plus LuaSec parameters.
--
-- @param url mandatory (string or table)
-- @param body optional (string)
-- @return (string if url == string or 1), code, headers, status
--
local function request(url, body)
  local result_table = {}
  local stringrequest = type(url) == "string"
  if stringrequest then
    url = urlstring_totable(url, body, result_table)
  else
    url.url = default_https_port(url.url)
  end
  if http.PROXY or url.proxy then
    return nil, "proxy not supported"
  elseif url.redirect then
    return nil, "redirect not supported"
  elseif url.create then
    return nil, "create function not permitted"
  end
  -- New 'create' function to establish a secure connection
  url.create = tcp(url)
  local res, code, headers, status = http.request(url)
  if res and stringrequest then
    return table.concat(result_table), code, headers, status
  end
  return res, code, headers, status
end

--------------------------------------------------------------------------------
-- Export module
--

_M.request = request


local function formatdata(t)
  local buf={}
  for k,v in pairs(t) do
    if type(k)=="number" then
      table.insert(buf,v)
    else
      table.insert(buf,string.format("%s=%s",k,v))
    end
  end
  return table.concat(buf,"&")
end

local function checkcookie(headers)
  if not  headers then
      return nil
      end
  local cok=headers['set-cookie'] or headers['Set-Cookie']
  if cok then
    local t={}
    for k,v in string.gmatch(cok,'([^=,; ]+)=([^=,; ]+)') do
      table.insert(t,string.format('%s=%s; ',k,v))
      end
      cok=table.concat(t)
    end
  return cok
end

local function copy(t,...)
  for _,f in pairs{...} do
    for k,v in pairs(f) do
      t[k]=v
    end
  end
return t
end


_M.cookie=""
_M.header={}
_M.ua=""

function _M.post(u,d,cok,ua,hdr)
  local t = {}
  d=d or ""
  if type(d)=="table" then
    d=formatdata(d)
    end
  local url={
   url = default_https_port(u),
   method = "POST",
   headers = copy({
    ["Content-Type"] = "application/x-www-form-urlencoded",
    ["Accept-Language"] = "zh-cn,zh;q=0.5",
    ["Accept-Charset"] = "utf-8",
    ["Content-Length"] = #d,
    ['Cookie']=cok or _M.cookie,
   },hdr or _M.header),
   source = ltn12.source.string(d),
   sink = ltn12.sink.table(t)}
  url.create = tcp(url)
  local r, c, h = http.request(url)
  return table.concat(t),checkcookie(h), c, h
 end

local boundary="----qwertyuiopasdfghjklzxcvbnm"

local function formatmultidata(d,t)
  local buf={}
  for k,v in pairs(d or {}) do
    table.insert(buf,string.format('--%s\r\nContent-Disposition:form-data;name="%s"\r\n\r\n%s\r\n',boundary,k,v))
  end
  for k,v in pairs(t or {}) do
    local f=io.open(v,"r")
    local s=f:read("*all")
    f:close()
    table.insert(buf,string.format('--%s\r\nContent-Disposition:form-data;name="%s";filename="%s"\r\nContent-Type:application/octet-stream\r\n\r\n%s\r\n',boundary,k,v,s))
  end
  table.insert(buf,string.format("--%s--\r\n",boundary))
  return table.concat(buf)
end

function _M.upload(u,d,f,cok,ua,hdr)
  local t = {}
  local data=formatmultidata(d,f)
  local url={
    url = default_https_port(u),
    method = "POST",
    headers = copy({
      ["Content-Type"] = "multipart/form-data;boundary="..boundary,
      ["Accept-Language"] = "zh-cn,zh;q=0.5",
      ["Accept-Charset"] = "utf-8",
      ["Content-Length"] = #data,
      ['Cookie']=cok or _M.cookie,
      ["User-Agent"]=ua or _M.ua,
    },hdr or _M.header),
    source = ltn12.source.string(data),
    sink = ltn12.sink.table(t)}
  url.create = tcp(url)
  local r, c, h = http.request(url)
  return table.concat(t),checkcookie(h), c, h
end

function _M.get(u,cok,ua,hdr)
  local t = {}
  local url={
   url = default_https_port(u),
   method = "GET",
   headers = copy({
    ["Content-Type"] = "application/x-www-form-urlencoded",
    ["Accept-Language"] = "zh-cn,zh;q=0.5",
    ["Accept-Charset"] = "utf-8",
    ['Cookie']=cok or _M.cookie,
    ["User-Agent"]=ua or _M.ua,
  },hdr or _M.header),
   sink = ltn12.sink.table(t)}
  url.create = tcp(url)
  local r, c, h = http.request(url)
  return table.concat(t),checkcookie(h), c, h
 end

function _M.download(u,p,cok,ua,ref,hdr)
  local f = io.open(p,"w")
  local url={
   url = default_https_port(u),
   method = "GET",
   headers = copy({
    ["Content-Type"] = "application/x-download",
    ["Referer"]=ref or u:find('(http://[^/]+)'),
    ['Cookie']=cok or _M.cookie,
    ["User-Agent"]=ua or _M.ua,
   },hdr or _M.header),
   sink = ltn12.sink.file(f)}
  url.create = tcp(url)
  local r, c, h = http.request(url)
  return c, h
 end


return _M
