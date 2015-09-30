module("percent", package.seeall)

function encode(v)
   local result = v:gsub( "(%c)", function (c) return "%" .. string.format( "%02X", c:byte(1) ) end )
   return result
end

function decode(v)
   local result = v:gsub( "(%%(%x%x))", function( p, v ) return hex.pack(v) end )
   return result
end
