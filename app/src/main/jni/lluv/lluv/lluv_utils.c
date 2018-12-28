/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2017 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv.h"
#include "lluv_error.h"
#include "lluv_loop.h"
#include "lluv_handle.h"
#include "lluv_loop.h"
#include "lluv_req.h"
#include <memory.h>
#include <stdlib.h>
#include <assert.h>

const char *LLUV_MEMORY_ERROR_MARK = LLUV_PREFIX" Error mark";

#ifdef _WIN32
#  ifndef S_ISDIR
#    define S_ISDIR(mode)  (mode&_S_IFDIR)
#  endif
#  ifndef S_ISREG
#    define S_ISREG(mode)  (mode&_S_IFREG)
#  endif
#  ifndef S_ISLNK
#    define S_ISLNK(mode)  (0)
#  endif
#  ifndef S_ISSOCK
#    define S_ISSOCK(mode)  (0)
#  endif
#  ifndef S_ISFIFO
#    define S_ISFIFO(mode)  (0)
#  endif
#  ifndef S_ISCHR
#    define S_ISCHR(mode)  (mode&_S_IFCHR)
#  endif
#  ifndef S_ISBLK
#    define S_ISBLK(mode)  (0)
#  endif
#endif

LLUV_INTERNAL void* lluv_alloc(lua_State* L, size_t size){
  (void)L;
  return malloc(size);
}

LLUV_INTERNAL void lluv_free(lua_State* L, void *ptr){
  (void)L;
  free(ptr);
}

LLUV_INTERNAL int lluv_lua_call(lua_State* L, int narg, int nret){
  int ret, error_handler = lua_isnil(L, LLUV_ERROR_HANDLER_INDEX) ? 0 : LLUV_ERROR_HANDLER_INDEX;

  // On Lua it is possible use upvalueindex directly. (Tested On Lua 5.1-5.3)
  // But it is fail on LuaJIT.
  // But Lua manual says `In the current implementation, this index cannot be a pseudo-index`
  // May be use runtime check for LuaJIT?

  if(error_handler){
    lua_pushvalue(L, error_handler);
    error_handler = lua_absindex(L, -(narg+2));
    lua_insert(L, error_handler);
  }

  ret = lua_pcall(L, narg, nret, error_handler);

  if(error_handler){
    lua_remove(L, error_handler);
  }

  if(!ret) return 0;

  if(ret == LUA_ERRMEM) lua_pushlightuserdata(L, (void*)LLUV_MEMORY_ERROR_MARK);
  lua_replace(L, LLUV_ERROR_MARK_INDEX);
  {
    lluv_loop_t* loop = lluv_opt_loop(L, LLUV_LOOP_INDEX, 0);
    uv_stop(loop->handle);
  }
  return ret;
}

LLUV_INTERNAL int lluv__index(lua_State *L, const char *meta, lua_CFunction inherit){
  assert(lua_gettop(L) == 2);

  lutil_getmetatablep(L, meta);
  lua_pushvalue(L, 2); lua_rawget(L, -2);
  if(!lua_isnil(L, -1)) return 1;
  lua_settop(L, 2);
  if(inherit) return inherit(L);
  return 0;
}

LLUV_INTERNAL void lluv_check_callable(lua_State *L, int idx){
  idx = lua_absindex(L, idx);
  luaL_checktype(L, idx, LUA_TFUNCTION);
}

LLUV_INTERNAL void lluv_check_none(lua_State *L, int idx){
  idx = lua_absindex(L, idx);
  luaL_argcheck (L, lua_isnone(L, idx), idx, "too many parameters");
}

LLUV_INTERNAL void lluv_check_args_with_cb(lua_State *L, int n){
  lluv_check_none(L, n + 1);
  lluv_check_callable(L, -1);
}

LLUV_INTERNAL void lluv_push_status(lua_State *L, int status){
  if(status >= 0)
    lua_pushnil(L);
  else
    lluv_error_create(L, LLUV_ERR_UV, (uv_errno_t)status, NULL);
}

LLUV_INTERNAL void lluv_alloc_buffer_cb(uv_handle_t* h, size_t suggested_size, uv_buf_t *buf){
//  *buf = lluv_buf_init(malloc(suggested_size), suggested_size);
  lluv_handle_t *handle = lluv_handle_byptr(h);
  lluv_loop_t     *loop = lluv_loop_by_handle(h);

  if(!IS_(loop, BUFFER_BUSY)){
    SET_(loop, BUFFER_BUSY);
    buf->base = loop->buffer; buf->len = loop->buffer_size;
  }
  else{
    *buf = lluv_buf_init(lluv_alloc(handle->L, suggested_size), suggested_size);
  }
}

LLUV_INTERNAL void lluv_free_buffer(uv_handle_t* h, const uv_buf_t *buf){
  if(buf->base){
    lluv_handle_t *handle = lluv_handle_byptr(h);
    lluv_loop_t     *loop = lluv_loop_by_handle(h);

    if(buf->base == loop->buffer){
      assert(IS_(loop, BUFFER_BUSY));
      UNSET_(loop, BUFFER_BUSY);
    }
    else{
      lluv_free(handle->L, &buf->base[0]);
    }
  }
}

LLUV_INTERNAL int lluv_to_addr(lua_State *L, const char *addr, int port, struct sockaddr_storage *sa){
  int err;
  char tmp[40];

  UNUSED_ARG(L);

  if((addr[0] == '*')&&(addr[1] == '\0')){
    static const char *zero_ip = "0.0.0.0";
    addr = zero_ip;
  }
  else if(addr[0] == '['){
    size_t len = strnlen(addr, 40);
    if((addr[len] == '\0')&&(addr[len-1] == ']')){
      memcpy(tmp, &addr[1], len-2);
      tmp[len-2] = '\0';
      addr = tmp;
    }
    else{
      return UV_EINVAL;
    }
  }

  memset(sa, 0, sizeof(*sa));

  err = uv_ip4_addr(addr, port, (struct sockaddr_in*)sa);
  if(err < 0){
    err = uv_ip6_addr(addr, port, (struct sockaddr_in6*)sa);
  }
  return err;
}

LLUV_INTERNAL int lluv_check_addr(lua_State *L, int i, struct sockaddr_storage *sa){
  const char *addr  = luaL_checkstring(L, i);
  lua_Integer port  = luaL_checkint(L, i + 1);
  return lluv_to_addr(L, addr, port, sa);
}

LLUV_INTERNAL int lluv_push_addr(lua_State *L, const struct sockaddr_storage *addr){
  char buf[INET6_ADDRSTRLEN + 1];

  switch (((struct sockaddr*)addr)->sa_family){
    case AF_INET:{
      struct sockaddr_in *sa = (struct sockaddr_in*)addr;
      uv_ip4_name(sa, buf, sizeof(buf));
      lua_pushstring(L, buf);
      lua_pushinteger(L, ntohs(sa->sin_port));
      return 2;
    }

    case AF_INET6:{
      struct sockaddr_in6 *sa = (struct sockaddr_in6*)addr;
      uv_ip6_name(sa, buf, sizeof(buf));
      lua_pushstring(L, buf);
      lua_pushinteger(L, ntohs(sa->sin6_port));
      lutil_pushint64(L, ntohl(sa->sin6_flowinfo));
      lutil_pushint64(L, sa->sin6_scope_id);
      return 4;
    }
  }

  return 0;
}

LLUV_INTERNAL void lluv_push_stat(lua_State* L, const uv_stat_t* s){
#define SET_FIELD_INT(F,V)  lutil_pushint64(L, s->V);         lua_setfield(L, -2, F)
#define SET_FIELD_MODE(F,V) lua_pushboolean(L, V(s->st_mode));lua_setfield(L, -2, F)
#define SET_FIELD_TIME(F,V) lluv_push_timespec(L, &s->V); lua_setfield(L, -2, F)

  lua_newtable(L);
  SET_FIELD_INT( "dev"    , st_dev    );
  SET_FIELD_INT( "ino"    , st_ino    );
  SET_FIELD_INT( "mode"   , st_mode   );
  SET_FIELD_INT( "nlink"  , st_nlink  );
  SET_FIELD_INT( "uid"    , st_uid    );
  SET_FIELD_INT( "gid"    , st_gid    );
  SET_FIELD_INT( "rdev"   , st_rdev   );
  SET_FIELD_INT( "size"   , st_size   );
  SET_FIELD_INT( "blksize", st_blksize);
  SET_FIELD_INT( "blocks" , st_blocks );

  SET_FIELD_MODE("is_file"             , S_ISREG  );
  SET_FIELD_MODE("is_directory"        , S_ISDIR  );
  SET_FIELD_MODE("is_character_device" , S_ISCHR  );
  SET_FIELD_MODE("is_block_device"     , S_ISBLK  );
  SET_FIELD_MODE("is_fifo"             , S_ISFIFO );
  SET_FIELD_MODE("is_symbolic_link"    , S_ISLNK  );
  SET_FIELD_MODE("is_socket"           , S_ISSOCK );

  SET_FIELD_TIME("atime", st_atim );
  SET_FIELD_TIME("mtime", st_mtim );
  SET_FIELD_TIME("ctime", st_ctim );

#undef SET_FIELD_INT
#undef SET_FIELD_MODE
#undef SET_FIELD_TIME
}

static const char* lluv_to_string(lua_State *L, int idx){
  idx = lua_absindex(L, idx);
  lua_getglobal(L, "tostring");
  lua_pushvalue(L, idx);
  lua_call(L, 1, 1);
  return lua_tostring(L, -1);
}

LLUV_INTERNAL void lluv_value_dump(lua_State* L, int i, const char* prefix) {
  const char* tname = lua_typename(L, lua_type(L, i));
  if(!prefix){
    static const char *tab = "  ";
    prefix = tab;
  }
  switch (lua_type(L, i)) {
    case LUA_TNONE:
      printf("%s%d: %s\n",     prefix, i, tname);
      break;
    case LUA_TNIL:
      printf("%s%d: %s\n",     prefix, i, tname);
      break;
    case LUA_TNUMBER:
      printf("%s%d: %s\t%f\n", prefix, i, tname, lua_tonumber(L, i));
      break;
    case LUA_TBOOLEAN:
      printf("%s%d: %s\n\t%s", prefix, i, tname, lua_toboolean(L, i) ? "true" : "false");
      break;
    case LUA_TSTRING:
      printf("%s%d: %s\t%s\n", prefix, i, tname, lua_tostring(L, i));
      break;
    case LUA_TTABLE:
      printf("%s%d: %s\n",     prefix, i, lluv_to_string(L, i)); lua_pop(L, 1);
      break;
    case LUA_TFUNCTION:
      printf("%s%d: %s\t%p\n", prefix, i, tname, lua_tocfunction(L, i));
      break;
    case LUA_TUSERDATA:
      printf("%s%d: %s\t%s\n", prefix, i, tname, lluv_to_string(L, i)); lua_pop(L, 1);
      break;
    case LUA_TTHREAD:
      printf("%s%d: %s\t%p\n", prefix, i, tname, lua_tothread(L, i));
      break;
    case LUA_TLIGHTUSERDATA:
      printf("%s%d: %s\t%p\n", prefix, i, tname, lua_touserdata(L, i));
      break;
  }
}

LLUV_INTERNAL void lluv_stack_dump(lua_State* L, int top, const char* name) {
  int i, l;
  printf("\n" LLUV_PREFIX " API STACK DUMP: %s\n", name);
  for (i = top, l = lua_gettop(L); i <= l; i++) {
    lluv_value_dump(L, i, "  ");
  }
  printf("\n");
}

LLUV_INTERNAL void lluv_register_constants(lua_State* L, const lluv_uv_const_t* cons){
  const lluv_uv_const_t* ptr;
  for(ptr = &cons[0];ptr->name;++ptr){
    lua_pushstring(L, ptr->name);
    lutil_pushint64(L, ptr->code);
    lua_rawset(L, -3);
  }
}

LLUV_INTERNAL unsigned int lluv_opt_flags_ui(lua_State *L, int idx, unsigned int d, const lluv_uv_const_t* names){
  if(lua_isnoneornil(L, idx)) return d;
  if(lua_isnumber(L, idx)) return (unsigned int)lutil_checkint64(L, idx);
  if(lua_istable(L, idx)){
    unsigned int flags = 0;
    idx = lua_absindex(L, idx);
    lua_pushnil(L);
    while(lua_next(L, idx) != 0){
      const lluv_uv_const_t *name; int found = 0;
      const char *key; int value;
      if(lua_isnumber(L, -2)){ // array
        value = 1;
        key = luaL_checkstring(L, -1);
      }
      else{ // set
        key = luaL_checkstring(L, -2);
        value = lua_toboolean(L, -1);
      }
      lua_pop(L, 1);
      for(name = names; name->name; ++name){
        if(0 == strcmp(name->name, key)){
          if(value) flags |= (unsigned int)name->code;
          else flags &= ~((unsigned int)name->code);
          found = 1;
          break;
        }
      }
      if(!found){
        lua_pushfstring(L, "Unknown flag: `%s`", key);
        return lua_error(L);
      }
    }
    return flags;
  }
  lua_pushstring(L, "Unsupported flag type: ");
  lua_pushstring(L, lua_typename(L, lua_type(L, idx)));
  lua_concat(L, 2);
  return lua_error(L);
}

LLUV_INTERNAL unsigned int lluv_opt_flags_ui_2(lua_State *L, int idx, unsigned int d, const lluv_uv_const_t* names){
  if(lua_type(L, idx) == LUA_TSTRING){
    const lluv_uv_const_t *name;
    const char *key = lua_tostring(L, idx);
    for(name = names; name->name; ++name){
      if(0 == strcmp(name->name, key)){
        return name->code;
      }
    }
    lua_pushfstring(L, "Unknown flag: `%s`", key);
    return lua_error(L);
  }
  return lluv_opt_flags_ui(L, idx, d, names);
}

LLUV_INTERNAL ssize_t lluv_opt_named_const(lua_State *L, int idx, unsigned int d, const lluv_uv_const_t* names){
  if(lua_isnoneornil(L, idx)) return d;
  if(lua_isnumber(L, idx)) return (lua_Integer)lutil_checkint64(L, idx);
  if(lua_isstring(L, idx)){
    const char *key = lua_tostring(L, idx);
    const lluv_uv_const_t *name;
    for(name = names; name->name; ++name){
      if(0 == strcmp(name->name, key)){
        return name->code;
      }
    }
    lua_pushfstring(L, "Unknown constant: `%s`", key);
    return lua_error(L);
  }
  lua_pushstring(L, "Unsupported constant type: ");
  lua_pushstring(L, lua_typename(L, idx));
  lua_concat(L, 2);
  return lua_error(L);
}

LLUV_INTERNAL unsigned int lluv_opt_af_flags(lua_State *L, int idx, unsigned int d){
  static const lluv_uv_const_t FLAGS[] = {
    {AF_UNSPEC,    "unspec"   },
    {AF_INET,      "inet"     },
    {AF_INET6,     "inet6"    },

    {0, NULL}
  };

  return lluv_opt_flags_ui_2(L, idx, d, FLAGS);
}

LLUV_INTERNAL void lluv_push_timeval(lua_State *L, const uv_timeval_t *tv){
  lua_createtable(L, 0, 2);
  lua_pushinteger(L, tv->tv_sec);
  lua_setfield(L, -2, "sec");
  lua_pushinteger(L, tv->tv_usec);
  lua_setfield(L, -2, "usec");
}

LLUV_INTERNAL void lluv_push_timespec(lua_State *L, const uv_timespec_t *ts){
  lua_createtable(L, 0, 2);
  lua_pushinteger(L, ts->tv_sec);
  lua_setfield(L, -2, "sec");
  lua_pushinteger(L, ts->tv_nsec);
  lua_setfield(L, -2, "nsec");
}

LLUV_INTERNAL int lluv_return_req(lua_State *L, lluv_handle_t *handle, lluv_req_t *req, int err){
  if(err < 0){
    lua_rawgeti(L, LLUV_LUA_REGISTRY, req->cb);
    lua_rawgeti(L, LLUV_LUA_REGISTRY, req->ctx);
    lluv_req_free(L, req);
    if(lua_isnil(L, -2)){
      lua_pop(L, 2);
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
    }

    lua_pushvalue(L, 1); // push self
    lua_insert(L, -2);   // move self as first arg
    lluv_error_create(L, LLUV_ERR_UV, err, NULL);
    lua_insert(L, -2);
    lluv_loop_defer_call(L, lluv_loop_by_handle(&handle->handle), 3);
  }

  lua_settop(L, 1);
  return 1;
}

LLUV_INTERNAL int lluv_return_loop_req(lua_State *L, lluv_loop_t *loop, lluv_req_t *req, int err){
  if(err < 0){
    lua_rawgeti(L, LLUV_LUA_REGISTRY, req->cb);
    lluv_req_free(L, req);
    if(lua_isnil(L, -1)){
      lua_pop(L, 1);
      return lluv_fail(L, loop->flags, LLUV_ERR_UV, err, NULL);
    }

    lua_pushvalue(L, 1);
    lluv_error_create(L, LLUV_ERR_UV, err, NULL);
    lluv_loop_defer_call(L, loop, 2);
  }

  lua_settop(L, 1);
  return 1;
}

LLUV_INTERNAL int lluv_return(lua_State *L, lluv_handle_t *handle, int cb, int err){
  if(err < 0){
    lua_rawgeti(L, LLUV_LUA_REGISTRY, cb);

    if(lua_isnil(L, -1)){
      lua_pop(L, 1);
      return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
    }

    lua_pushvalue(L, 1);
    lluv_error_create(L, LLUV_ERR_UV, err, NULL);
    lluv_loop_defer_call(L, lluv_loop_by_handle(&handle->handle), 2);
  }

  lua_settop(L, 1);
  return 1;
}

LLUV_INTERNAL int lluv_new_weak_table(lua_State*L, const char *mode){
  int top = lua_gettop(L);
  lua_newtable(L);
  lua_newtable(L);
  lua_pushstring(L, mode);
  lua_setfield(L, -2, "__mode");
  lua_setmetatable(L,-2);
  assert((top+1) == lua_gettop(L));
  return 1;
}

LLUV_INTERNAL uv_buf_t lluv_buf_init(char* base, size_t len) {
  uv_buf_t buf;
  buf.base = base;
  buf.len = len;
  return buf;
}

uv_os_sock_t lluv_check_os_sock(lua_State *L, int idx){
  if(lua_islightuserdata(L, idx)){
    return (uv_os_sock_t)lua_touserdata(L, idx);
  }
  return (uv_os_sock_t)lutil_checkint64(L, idx);
}

void lluv_push_os_fd(lua_State *L, uv_os_fd_t fd){
#if !defined(_WIN32)
  lutil_pushint64(L, (uint64_t)fd);
#else
  LLUV_ASSERT_SAME_SIZE(uv_os_fd_t, uv_os_sock_t);
  lluv_push_os_socket(L, (uv_os_sock_t)fd);
#endif
}

void lluv_push_os_socket(lua_State *L, uv_os_sock_t fd) {
#if !defined(_WIN32)
  lutil_pushint64(L, (uint64_t)fd);
#else /*_WIN32*/
  /* Assumes that compiler can optimize constant conditions. MSVC do this. */

  /*On Lua 5.3 lua_Integer type can be represented exactly*/
#if LUA_VERSION_NUM >= 503
  if (sizeof(uv_os_sock_t) <= sizeof(lua_Integer)) {
    lua_pushinteger(L, (lua_Integer)fd);
    return;
  }
#endif

#if defined(LUA_NUMBER_DOUBLE) || defined(LUA_NUMBER_FLOAT)
  /*! @todo test DBL_MANT_DIG, FLT_MANT_DIG */

  if (sizeof(lua_Number) == 8) { /*we have 53 bits for integer*/
    if ((sizeof(uv_os_sock_t) <= 6)) {
      lua_pushnumber(L, (lua_Number)fd);
      return;
    }

    if(((UINT_PTR)fd & 0x1FFFFFFFFFFFFF) == (UINT_PTR)fd)
      lua_pushnumber(L, (lua_Number)fd);
    else
      lua_pushlightuserdata(L, (void*)fd);

    return;
  }

  if (sizeof(lua_Number) == 4) { /*we have 24 bits for integer*/
    if (((UINT_PTR)fd & 0xFFFFFF) == (UINT_PTR)fd)
      lua_pushnumber(L, (lua_Number)fd);
    else
      lua_pushlightuserdata(L, (void*)fd);
    return;
  }
#endif

  lutil_pushint64(L, (uint64_t)fd);
  if (lluv_check_os_sock(L, -1) != fd)
    lua_pushlightuserdata(L, (void*)fd);

#endif /*_WIN32*/
}
