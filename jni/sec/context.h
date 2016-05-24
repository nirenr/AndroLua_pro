#ifndef LSEC_CONTEXT_H
#define LSEC_CONTEXT_H

/*--------------------------------------------------------------------------
 * LuaSec 0.6a
 * Copyright (C) 2006-2015 Bruno Silvestre
 *
 *--------------------------------------------------------------------------*/

#include <lua.h>
#include <openssl/ssl.h>

#include "config.h"

#define LSEC_MODE_INVALID 0
#define LSEC_MODE_SERVER  1
#define LSEC_MODE_CLIENT  2

#define LSEC_VERIFY_CONTINUE        1
#define LSEC_VERIFY_IGNORE_PURPOSE  2

typedef struct t_context_ {
  SSL_CTX *context;
  lua_State *L;
  DH *dh_param;
  int mode;
} t_context;
typedef t_context* p_context;

/* Retrieve the SSL context from the Lua stack */
SSL_CTX *lsec_checkcontext(lua_State *L, int idx);
SSL_CTX *lsec_testcontext(lua_State *L, int idx);

/* Retrieve the mode from the context in the Lua stack */
int lsec_getmode(lua_State *L, int idx);

/* Registre the module. */
LSEC_API int luaopen_sec_context(lua_State *L);

#endif
