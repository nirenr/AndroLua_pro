/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2016 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv.h"
#include "lluv_utils.h"
#include "lluv_timer.h"
#include "lluv_error.h"
#include "lluv_idle.h"
#include "lluv_loop.h"
#include "lluv_fs.h"
#include "lluv_fbuf.h"
#include "lluv_handle.h"
#include "lluv_stream.h"
#include "lluv_tcp.h"
#include "lluv_pipe.h"
#include "lluv_tty.h"
#include "lluv_udp.h"
#include "lluv_prepare.h"
#include "lluv_check.h"
#include "lluv_poll.h"
#include "lluv_signal.h"
#include "lluv_fs_event.h"
#include "lluv_fs_poll.h"
#include "lluv_process.h"
#include "lluv_misc.h"
#include "lluv_dns.h"

#define LLUV_COPYRIGHT     "Copyright (C) 2014-2018 Alexey Melnichuk"
#define LLUV_MODULE_NAME   "lluv"
#define LLUV_LICENSE       "MIT"

#define LLUV_VERSION_MAJOR 0
#define LLUV_VERSION_MINOR 1
#define LLUV_VERSION_PATCH 9
// #define LLUV_VERSION_COMMENT "dev"

static const char* LLUV_REGISTRY = LLUV_PREFIX" Registry";
static const char* LLUV_HANDLES  = LLUV_PREFIX" Handles";

static int lluv_push_version(lua_State *L){
  lua_pushinteger(L, LLUV_VERSION_MAJOR);
  lua_pushliteral(L, ".");
  lua_pushinteger(L, LLUV_VERSION_MINOR);
  lua_pushliteral(L, ".");
  lua_pushinteger(L, LLUV_VERSION_PATCH);
#ifdef LLUV_VERSION_COMMENT
  if(LLUV_VERSION_COMMENT[0]){
    lua_pushliteral(L, "-"LLUV_VERSION_COMMENT);
    lua_concat(L, 6);
  }
  else
#endif
  lua_concat(L, 5);
  return 1;
}

static int lluv_debug_registry(lua_State *L){
  lua_rawgetp(L, LUA_REGISTRYINDEX, LLUV_REGISTRY);
  lua_rawgetp(L, LUA_REGISTRYINDEX, LLUV_HANDLES);
  return 2;
}

static const struct luaL_Reg lluv_functions[] = {
  {"__registry", lluv_debug_registry},

  {NULL,NULL}
};

static int luaopen_lluv_impl(lua_State *L, int safe){
#define NUPVALUES 2
#define LLUV_PUSH_UPVALUES(L) lua_pushvalue(L, -3); lua_pushvalue(L, -3)

  lua_rawgetp(L, LUA_REGISTRYINDEX, LLUV_REGISTRY);
  if(!lua_istable(L, -1)){ /* registry */
    lua_pop(L, 1);
    lua_newtable(L);             lua_pushvalue(L, -1); lua_rawsetp(L, LUA_REGISTRYINDEX, LLUV_REGISTRY);
    lluv_new_weak_table(L, "kv");lua_pushvalue(L, -1); lua_rawsetp(L, LUA_REGISTRYINDEX, LLUV_HANDLES);
  }

  lua_newtable(L); /* registry, handles, library  */

  LLUV_PUSH_UPVALUES(L); lluv_loop_initlib     (L, NUPVALUES);

  LLUV_PUSH_UPVALUES(L); luaL_setfuncs(L, lluv_functions, NUPVALUES);
  LLUV_PUSH_UPVALUES(L); lluv_error_initlib    (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_fs_initlib       (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_handle_initlib   (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_stream_initlib   (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_timer_initlib    (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_fbuf_initlib     (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_idle_initlib     (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_tcp_initlib      (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_pipe_initlib     (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_tty_initlib      (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_udp_initlib      (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_prepare_initlib  (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_check_initlib    (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_poll_initlib     (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_signal_initlib   (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_fs_event_initlib (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_fs_poll_initlib  (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_process_initlib  (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_misc_initlib     (L, NUPVALUES, safe);
  LLUV_PUSH_UPVALUES(L); lluv_dns_initlib      (L, NUPVALUES, safe);

  lua_remove(L, -2); /* registry */
  lua_remove(L, -2); /* handles  */

  lluv_push_version(L);
  lua_setfield(L, -2, "_VERSION");

  lua_pushliteral(L, LLUV_MODULE_NAME);
  lua_setfield(L, -2, "_NAME");

  lua_pushliteral(L, LLUV_COPYRIGHT);
  lua_setfield(L, -2, "_COPYRIGHT");

  lua_pushliteral(L, LLUV_LICENSE);
  lua_setfield(L, -2, "_LICENSE");

  return 1;

#undef LLUV_PUSH_UPVALUES
#undef NUPVALUES
}

LLUV_EXPORT_API
int luaopen_lluv_safe(lua_State *L){
  return luaopen_lluv_impl(L, 1);
}

LLUV_EXPORT_API
int luaopen_lluv_unsafe(lua_State *L){
  return luaopen_lluv_impl(L, 0);
}

LLUV_EXPORT_API
int luaopen_lluv(lua_State *L){
  return 
#ifdef LLUV_DEFAULT_UNSAFE
    luaopen_lluv_unsafe(L);
#else
    luaopen_lluv_safe(L);
#endif
}


