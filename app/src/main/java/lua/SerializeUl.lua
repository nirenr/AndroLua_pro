function SerializeUltra(tbl,saved,TopName)--序列化table函数
local saved = saved or {}
local TopName = TopName or 'temp'
local function BasicSerialze(o)
if type(o) == type(0) then
return tostring(o)
else
return string.format("%q",o)--格式化处理
end
end
local ret = {}
local function Serialize(tbl,saved,name,level)
local name = name or "temp"
local level = level or 0
--print (level)
level = level + 1
if level > 20 then --限制为最高20层table
error("too deep to servialize!")
end
table.insert(ret,name..' = ')
if type(tbl) == "table" then
if saved[tbl] then
table.insert(ret,saved[tbl]..";\n")
else
saved[tbl] = name
table.insert(ret,"{};\n")
for k,v in pairs(tbl) do
local _k = BasicSerialze(k)
if _k == nil then
error("servialize with an error key!")
end
local tname = string.format("%s[%s]",name,_k)
Serialize(v,saved,tname,level)
end
end
else
table.insert(ret,string.format("%s;\n",BasicSerialze(tbl)))
end
end
Serialize(tbl,saved,TopName)
return string.format("local %sreturn %s",table.concat(ret),TopName)
end