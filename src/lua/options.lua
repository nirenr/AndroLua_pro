local function usage()
  print("Usage:")
  print("* Generate options of your system:")
  print("  lua options.lua -g /path/to/ssl.h [verion] > options.h")
  print("* Examples:")
  print("  lua options.lua -g /usr/include/openssl/ssl.h > options.h\n")
  print("  lua options.lua -g /usr/include/openssl/ssl.h \"OpenSSL 1.0.1 14\" > options.h\n")

  print("* List options of your system:")
  print("  lua options.lua -l /path/to/ssl.h\n")
end

--
local function printf(str, ...)
  print(string.format(str, ...))
end

local function generate(options, version)
  print([[
#ifndef LSEC_OPTIONS_H
#define LSEC_OPTIONS_H

/*--------------------------------------------------------------------------
 * LuaSec 0.6a
 * Copyright (C) 2006-2015 Bruno Silvestre
 *
 *--------------------------------------------------------------------------*/

#include <openssl/ssl.h>

/* If you need to generate these options again, see options.lua */
]])
  printf([[
/* 
  OpenSSL version: %s
*/
]], version)
  print([[
struct ssl_option_s {
  const char *name;
  unsigned long code;
};
typedef struct ssl_option_s ssl_option_t;
]])

  print([[static ssl_option_t ssl_options[] = {]])

  for k, option in ipairs(options) do
    local name = string.lower(string.sub(option, 8))
    print(string.format([[#if defined(%s)]], option))
    print(string.format([[  {"%s", %s},]], name, option))
    print([[#endif]])
  end
  print([[  {NULL, 0L}]])
  print([[
};

#endif
]])
end

local function loadoptions(file)
  local options = {}
  local f = assert(io.open(file, "r"))
  for line in f:lines() do
    local op = string.match(line, "define%s+(SSL_OP_%S+)")
    if op then
      table.insert(options, op)
    end
  end
  table.sort(options, function(a,b) return a<b end)
  return options
end
--

local options
local flag, file, version = ...

version = version or "Unknown"

if not file then
  usage()
elseif flag == "-g" then
  options = loadoptions(file)
  generate(options, version)
elseif flag == "-l" then
  options = loadoptions(file)
  for k, option in ipairs(options) do
    print(option)
  end
else
  usage()
end
