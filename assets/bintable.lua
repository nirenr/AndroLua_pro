----------------------------------------------------------------------------
-- bintable: binary lua table representation
--
-- Author: Eike Decker
-- License: MIT
-- Version: 0.1
-- Purpose: Store lua tables in a compact binary representation.
--          Developed for improving network transmission compression.
-- Dependencies: Lua 5.1, pack (optional)
-- Usage:
--  bintable.packtable(aluatable) -- returns a string, representing the table
--  bintable.unpackdata(dataasstring,[offsetnumber]) -- returns the table
--    -- and the length of the data string. Optionally, an string offset can 
--    -- be passed, thus the data can be part of another string. Data string
--    -- length is not known until the full data has been unpacked.
--  bintable.makefloat(number) -- converts the given number to a float number,
--    -- which needs only 4 bytes when packed, instead of 8 bytes when double
--    -- is used. Requires pack. Note that float numbers "look ugly", i.e.
--    -- 1.3 becomes 0.12999999523163
-- Supports:
--  * tables, tables as keys and values, cyclic tables
--  * numbers
--  * strings
--  * booleans
-- Known bugs:
--  * floating point numbers or large integers are not precisely stored (fixed when using pack)
--  * future versions incompatible with current version
-- Possible features: 
--  * extendable custom types (for userdata, special tables etc.)
-- Size ratio vs. lua code stored tables: 0.5-.9
----------------------------------------------------------------------------

local tostring = tostring
local tonumber = tonumber
local assert = assert
local error = error
local string = string
local math = math
local type = type
local pairs,ipairs = pairs,ipairs
local table = table

module "bintable"
local bytes = {
	type = {
		[0] = "tableend",
		"table",
		"number",
		"number",
		"number",
		"number",--5
		"number",
		"number",--7
		"nilval",
		"number",
		"string",--10
		"number",
		"string",--12
		"boolean",
		"boolean",--14
		"string","string","string","string",--18
		"string","string","string","string",--22
		"string","string","string","string","string",--27
		"string","string","string","string",--31
		"string","string","string","string",--35
		"string","string","string","string",--39
		"table",--40
		"number",
		"number","number","number","number","number","number", -- 42-47
		"number",
	},

	tablestart = 1,
	tableend = 0,
	bytep = 2, -- positive byte
	byten = 3, -- negative byte
	shortp = 4,
	shortn = 5,
	intp = 6,
	intn = 7,
	
	tableref = 40,
	
	nilval = 8,
	
	double = 9,
	longstr = 10,
	float = 11,
	number = 41,
	fixedpoint1 = 42, -- 1.1
	fixedpoint2 = 43, -- 1.11
	fixedpoint3 = 44, -- 1.111
	fixedpoint4 = 45, -- 1.1111
	fixedpoint5 = 46, -- 1.11111
	fixedpoint6 = 47, -- 1.111111
	strnum = 48, -- store it with "tostring"
	--lutend = 11, -- look up table end -- ends with a number now
	
	lutstring = 12,
	
	booleantrue = 13,
	booleanfalse = 14,
	
	str0 = 15, -- empty str
	str1 = 16, -- length 1
	str2 = 17, -- length 2
	str3 = 18, -- length 3
	str4 = 19, -- length 4
	str5 = 20, -- length 5
	str6 = 21, -- length 6
	str7 = 22, -- length 7
	str8 = 23, -- length 8
	str9 = 24, -- length 9
	str10 = 25, -- length 10
	str11 = 26, -- length 11
	str12 = 27, -- length 12
	
	lutstr1 = 28, -- string 1 in lut
	lutstr2 = 29, -- string 2 in lut
	lutstr3 = 30, -- string 3 in lut
	lutstr4 = 31, -- string 4 in lut
	lutstr5 = 32, -- string 5 in lut
	lutstr6 = 33, -- string 6 in lut
	lutstr7 = 34, -- string 7 in lut
	lutstr8 = 35, -- string 8 in lut
	lutstr9 = 36, -- string 9 in lut
	lutstr10 = 37, -- string 10 in lut
	lutstr11 = 38, -- string 11 in lut
	lutstr12 = 39, -- string 12 in lut
	
	numbern32 = 64, -- number 0 -- anything higher is a number between 0 and 255-64 
	number0 = 96,
}
compress = {}
decompress = {}
function decompress.number(data,offset)
	local b = string.byte
	local c,d,e,f,g = b(data,(offset or 1),4+(offset or 1))
	if c>=64 then return c - 64 - 32,1 end
	if c == bytes.bytep then return d,2 end
	if c == bytes.byten then return -d,2 end
	if c == bytes.shortp then return d*0x100+e,3 end
	if c == bytes.shortn then return -d*0x100-e,3 end
	if c == bytes.intp then return d*0x1000000+e*0x10000+f*0x100+g,5 end
	if c == bytes.intn then return -(d*0x1000000+e*0x10000+f*0x100+g),5 end
	if c == bytes.float then
		local l,v = string.unpack(data,"<f",(offset or 1)+1)
		return v,5
	end
	if c == bytes.double then
		local l,v = string.unpack(data,"<d",(offset or 1)+1)
		return v,9
	end
	if c == bytes.number then 
		local a,len = decompress.number(data,(offset or 1)+1)
		local b,x = decompress.number(data,(offset or 1)+len+1)
		return math.ldexp(a*1e-9,b),1+len+x
	end
	if c >= bytes.fixedpoint1 and c<=bytes.fixedpoint6 then
		local exp = 10^(-(c-bytes.fixedpoint1+1))
		local n,len = decompress.number(data,(offset or 1)+1)
		return n*exp,len+1
	end
	if c == bytes.strnum then
		local str,len = decompress.string(data,(offset or 1)+1)
		return tonumber(str),len+1
	end
	error("Illegal type: "..c)
end
function compress.number (n)
	local floor = math.floor
	local function crop (n,by)
		return floor(n/by)%0x100
	end
	if floor(n)==n and math.abs(n)<=0xffffffff then -- it's an round number
		if n>=0 then
			if n<=0xff-bytes.number0 then return string.char(bytes.number0+n) end
			if n<=0xff then return string.char(bytes.bytep,n) end
			if n<=0xffff then return string.char(bytes.shortp,floor(n/0x100),n%0x100) end
			return string.char(bytes.intp,floor(n/0x1000000),crop(n,0x10000),crop(n,0x100),n%0x100)
		else
			n = - n
			if n<=32 then return string.char(bytes.number0-n) end
			if n<=0xff then return string.char(bytes.byten,n) end
			if n<=0xffff then return string.char(bytes.shortn,floor(n/0x100),n%0x100) end
			return string.char(bytes.intn,floor(n/0x1000000),crop(n,0x10000),crop(n,0x100),n%0x100)
		end
	else -- float or double
		local asstr = tostring(n)
		if tonumber(asstr)==n and #asstr<=3 then
			return string.char(bytes.strnum)..compress.string(asstr)
		end
		for i=1,6 do
			local exp = 10^i
			local fp = n*exp
			if math.abs(fp)>0xffffffff then break end
			if floor(fp)==fp then -- fixedpoint
				return string.char(bytes.fixedpoint1+i-1)..compress.number(fp)
			end
		end
		if not string.pack then
			local a,b = math.frexp(n)
			local a = math.floor(a * 1e9)
			return string.char(bytes.number)..compress.number(a)..compress.number(b)
		else
			local function test(fmt,prefix)
				local p = string.pack(fmt,n)
				local l,u = string.unpack(p,fmt)
				return u==n and (prefix..p)
			end
			return assert(test("<f",string.char(bytes.float)) or test("<d",string.char(bytes.double)))
		end
	end
end
function decompress.boolean(data,offset)
	local b = string.byte(data,offset or 1)
	return b==bytes.booleantrue,1
end
function compress.boolean(val)
	return string.char(val and bytes.booleantrue or bytes.booleanfalse)
end
function decompress.nilval(data,offset)
	assert(string.byte(data,offset or 1)==bytes.nilval)
	return nil,1
end
function compress.nilval(val)
	return string.char(bytes.nilval)
end
function decompress.string(data,offset,lut)
	local b = string.byte(data,offset or 1)
	if b == bytes.longstr then
		local len,m = decompress.number(data,(offset or 1)+1)
		return data:sub(m+(offset or 1)+1,(m+(offset or 1)+len)),len+m+1
	elseif b-bytes.str0<=12 and b-bytes.str0>=0 then
		local len = b-bytes.str0
		return data:sub((offset or 1)+1,(offset or 1)+len),len+1
	end
	if b == bytes.lutstring then
		local n,l = decompress.number(data,(offset or 1)+1)
		return lut[n],1+l
	end
	local l = b - bytes.lutstr1+1
	assert(l>0 and l<=12,"string lut ref: "..l)
	return lut[l],1
end
function compress.string(str,lut)
	if lut and lut[str] then
		local idx = lut[str]
		if idx>12 then
			return string.char(bytes.lutstring)..compress.number(idx)
		else
			return string.char(bytes.lutstr1+(idx-1))
		end
	end
	if #str>12 then
		return string.char(bytes.longstr)..compress.number(#str)..str
	else
		return string.char(bytes.str0+#str)..str
	end
end
function decompress.type(data,offset)
	local b = string.byte(data,offset)
	return bytes.type[b] or (b>=64 and "number") or "invalid"
end

function decompress.table(data,offset,strlut,tablut,table)
	offset = offset or 1
	local b = string.byte(data,offset)
	if b == bytes.tableref then 
		local num,n = decompress.number(data,offset+1)
		return tablut[num],n+1
	end
	assert(b == bytes.tablestart)
	local pos = 1
	local function nexttype()
		return decompress.type(data,offset+pos)
	end
	local function getnext()
		local type = nexttype()
		local v,n = decompress[type](data,offset+pos,strlut,tablut)
		pos = pos + n
		return v
	end
	while nexttype()~="tableend" do
		table[getnext()] = getnext()
	end
	return table,pos + 1
end
function compress.table(tab,strlut,tablut)
	local data = {string.char(bytes.tablestart)}
	local function compr(v)
		local t = type(v)
		if t=="string" then	return compress.string(v,strlut) end
		if t=="number" then return compress.number(v) end
		if t=="boolean" then return compress.boolean(v) end
		if t=="nil" then return compress.nilval(v) end
		if t=="table" then  
			local idx = assert(tablut[v],"invalid table lookuptable")
			return string.char(bytes.tableref)..compress.number(idx)
		end
		error("unhandled type: "..t)
	end
	for i,v in pairs(tab) do
		data[#data+1] = compr(i)
		data[#data+1] = compr(v)
	end
	data[#data+1] = string.char(bytes.tableend)
	return table.concat(data)
end
function compress.datatable(tab)
	-- this is the point to start table serialization
	-- first: create a list of all strings and tables and build a string
	--        lookuptable next to a complete list of all tables
	-- second: create the data structure as follows
	--  serial list of strings in the lookuptable until the bytes.lutend 
	--  byte occures.
	--  number of all tables in this structure
	--  repeated: tablesstart .. table end, contains each key / value datas
	
	local lut = {}
	local lutcount = {}
	local tabs = {tab}
	tabs[tab] = 1
	local function check(val)
		if type(val) == "table" and not tabs[val] then
			tabs[#tabs+1] = val
			tabs[val] = #tabs
			return true
		end
		if type(val) == "string" then
			if lutcount[val] then
				lutcount[val] = lutcount[val] + 1
			else
				lutcount[val] = 1
				lut[#lut+1] = val
			end
		end
	end
	local function collecttables(tab)
		for i,v in pairs(tab) do
			if check(i) then collecttables(i) end
			if check(v) then collecttables(v) end
		end
	end
	collecttables(tab)
	table.sort(lut,function(a,b)
		if lutcount[a]>lutcount[b] then return true end
		if lutcount[a]<lutcount[b] then return false end
		return a<b
	end)
	for i,v in ipairs(lut) do
		lut[v] = i
	end
	local data = {}
	for i,str in ipairs(lut) do 
		data[#data+1] = compress.string(str)
	end
	data[#data+1] = compress.number(#tabs)
	for i,tab in ipairs(tabs) do
		data[#data+1] = compress.table(tab,lut,tabs)
	end
	return table.concat(data)
end
function decompress.datatable(data,offset)
	offset = offset or 1
	local pos = 0
	local lut = {}
	local tablut = {}
	
	local function getnext(expect,...)
		local v,n = decompress[expect](data,offset+pos,...)
		pos = pos + n
		return v
	end
	local function nexttype() return decompress.type(data,offset+pos) end
	
	while nexttype()=="string" do
		lut[#lut+1] = getnext "string"
	end
	for i=1,getnext "number" do
		tablut[i] = {}
	end
	for i=1,#tablut do
		getnext("table",lut,tablut,tablut[i])
	end
	return tablut[1],pos
end


function packtable(tab)
	assert(type(tab)=="table","Expected a table as argument #1, got "..type(tab))
	return compress.datatable(tab)
end
function unpackdata(data,offset)
	assert(type(data)=="string","Expected a string as argument #1, got "..type(data))
	assert(type(offset)=="nil" or type(offset)=="number","Expected a number or nil as argument #2, got "..type(offset))
	return decompress.datatable(data,offset)
end

function makefloat(number)
	return select(2,string.unpack(string.pack("=f",number),"=f"))
end