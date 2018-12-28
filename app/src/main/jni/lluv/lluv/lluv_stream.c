/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2018 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv.h"
#include "lluv_handle.h"
#include "lluv_stream.h"
#include "lluv_tcp.h"
#include "lluv_pipe.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include "lluv_req.h"
#include <assert.h>

#define LLUV_STREAM_NAME LLUV_PREFIX" Stream"
static const char *LLUV_STREAM = LLUV_STREAM_NAME;

LLUV_INTERNAL int lluv_stream_index(lua_State *L){
  return lluv__index(L, LLUV_STREAM, lluv_handle_index);
}

LLUV_INTERNAL lluv_handle_t* lluv_stream_create(lua_State *L, uv_handle_type type, lluv_flags_t flags){
  lluv_handle_t *handle  = lluv_handle_create(L, type, flags | LLUV_FLAG_STREAM);

  assert( (type == UV_TCP) || (type == UV_NAMED_PIPE) || (type == UV_TTY) );

  return handle;
}

LLUV_INTERNAL lluv_handle_t* lluv_check_stream(lua_State *L, int idx, lluv_flags_t flags){
  lluv_handle_t *handle = lluv_check_handle(L, idx, flags);
  luaL_argcheck (L, IS_(handle, STREAM), idx, LLUV_STREAM_NAME" expected");

  return handle;
}

LLUV_INTERNAL void lluv_on_stream_req_cb(uv_req_t* arg, int status){
  lluv_req_t    *req    = lluv_req_byptr(arg);
  lluv_handle_t *handle = req->handle;
  lua_State     *L      = LLUV_HCALLBACK_L(handle);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  if(!IS_(handle, OPEN)){
    lluv_req_free(L, req);

    LLUV_CHECK_LOOP_CB_INVARIANT(L);
    return;
  }

  lua_rawgeti(L, LLUV_LUA_REGISTRY, req->cb);
  lluv_handle_pushself(L, handle);
  lua_rawgeti(L, LLUV_LUA_REGISTRY, req->ctx);
  lluv_req_free(L, req);

  if(lua_isnil(L, -3)){
    lua_pop(L, 3);

    LLUV_CHECK_LOOP_CB_INVARIANT(L);
    return;
  }

  lluv_push_status(L, status);
  lua_insert(L, -2);

  LLUV_HANDLE_CALL_CB(L, handle, 3);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

LLUV_INTERNAL void lluv_on_stream_connect_cb(uv_connect_t* arg, int status){
  lluv_on_stream_req_cb((uv_req_t*)arg, status);
}

//{ Shutdown

static void lluv_on_stream_shutdown_cb(uv_shutdown_t* arg, int status){
  lluv_on_stream_req_cb((uv_req_t*)arg, status);
}

static int lluv_stream_shutdown(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  lluv_req_t *req; int err;

  if(lua_gettop(L) == 1)
    lua_settop(L, 2);
  else
    lluv_check_args_with_cb(L, 2);

  req = lluv_req_new(L, UV_SHUTDOWN, handle);

  err = uv_shutdown(LLUV_R(req, shutdown), LLUV_H(handle, uv_stream_t), lluv_on_stream_shutdown_cb);

  return lluv_return_req(L, handle, req, err);
}

//}

//{ Listen

static void lluv_on_stream_connection_cb(uv_stream_t* arg, int status){
  lluv_handle_t *handle = lluv_handle_byptr((uv_handle_t*)arg);
  lua_State *L = LLUV_HCALLBACK_L(handle);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  if(!IS_(handle, OPEN)) return;

  lua_rawgeti(L, LLUV_LUA_REGISTRY, LLUV_CONNECTION_CB(handle));
  assert(!lua_isnil(L, -1));

  lluv_handle_pushself(L, handle);
  lluv_push_status(L, status);

  LLUV_HANDLE_CALL_CB(L, handle, 2);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

static int lluv_stream_listen(lua_State *L){
  lluv_handle_t  *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  int backlog = 511; /* http://blog.dubbelboer.com/2012/04/09/syn-cookies.html */
  int err;

  if(lua_gettop(L) > 2) backlog = luaL_checkint(L, 2);
  lluv_check_args_with_cb(L, 3);
  LLUV_CONNECTION_CB(handle) = luaL_ref(L, LLUV_LUA_REGISTRY);

  err = uv_listen(LLUV_H(handle, uv_stream_t), backlog, lluv_on_stream_connection_cb);

  if(err >= 0){
    /*There no way to stop this callback so we never free this lock*/
    lluv_handle_lock(L, handle, LLUV_LOCK_CONNECTION);
  }

  return lluv_return(L, handle, LLUV_CONNECTION_CB(handle), err);
}

//}

//{ Accept

static const char* lluv_ht_(uv_handle_type type){
  switch(type){
#define XX(l, c) case UV_##l: return #c;
UV_HANDLE_TYPE_MAP(XX)
#undef XX
  }
  assert(0 && "Unknown handle type");
  return "<unknown>";
}

static int lluv_new_(lua_State *L, lluv_loop_t *loop, uv_handle_type type, int unsafe){
  if(type == UV_TCP){
    /*local ok, err = uv.tcp(loop)*/
    lua_pushvalue(L, LLUV_LUA_REGISTRY);
    lua_pushvalue(L, LLUV_LUA_HANDLES);
    lua_pushcclosure(L, unsafe ? lluv_tcp_create_unsafe : lluv_tcp_create_safe, 2);
    lluv_loop_pushself(L, loop);
    lua_call(L, 1, 2);

     /*if not ok then return ok, err end*/
    if(lua_isnil(L, -2)) return 2;
    lua_remove(L, -1);
    return 1;
  }

  if(type == UV_NAMED_PIPE){
    /*local ok, err = uv.pipe(loop)*/
    lua_pushvalue(L, LLUV_LUA_REGISTRY);
    lua_pushvalue(L, LLUV_LUA_HANDLES);
    lua_pushcclosure(L, unsafe ? lluv_pipe_create_unsafe : lluv_pipe_create_safe, 2);
    lluv_loop_pushself(L, loop);
    lua_call(L, 1, 2);

    /*if not ok then return ok, err end*/
    if(lua_isnil(L, -2)) return 2;
    lua_remove(L, -1);
    return 1;
  }

  lua_pushfstring(L, "Unsupported handle type: %s. Try create handle by self.", lluv_ht_(type));
  return lua_error(L);
}

static int lluv_stream_accept(lua_State *L){
  lluv_handle_t  *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t    *loop   = lluv_loop_byptr(handle->handle.loop);
  lluv_handle_t  *dst;
  int err; int is_new_dst = 0;

  if(lua_gettop(L) == 1){
    uv_handle_type ht = handle->handle.type;
    lua_settop(L, 1);
    if(ht == UV_TCP){
      int ret = lluv_new_(L, loop, UV_TCP, IS_(handle, RAISE_ERROR));
      if(ret != 1)return ret;

    }
    else if(ht == UV_NAMED_PIPE){
      int err = uv_pipe_pending_count(LLUV_H(handle, uv_pipe_t));
      if(err < 0){
        return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
      }
      if(err > 0){
        uv_handle_type type = uv_pipe_pending_type(LLUV_H(handle, uv_pipe_t));
        err = lluv_new_(L, loop, type, IS_(handle, RAISE_ERROR));
      }
      else{
        err = lluv_new_(L, loop, UV_NAMED_PIPE, IS_(handle, RAISE_ERROR));
      }
      if(err != 1) return err;
    }
    else{
      lua_pushfstring(L, "Unsupported handle type: %s. Try create handle by self.", lluv_ht_(ht));
      return lua_error(L);
    }
    is_new_dst = 1;
  }
  lua_settop(L, 2);
  dst = lluv_check_stream(L, 2, LLUV_FLAG_OPEN);

  err = uv_accept(LLUV_H(handle, uv_stream_t), LLUV_H(dst, uv_stream_t));
  if(err < 0){
    if(is_new_dst && dst){
      /*dst:close()*/
      lua_getfield(L, 2, "close");
      lua_pushvalue(L, 2);
      lua_pcall(L, 1, 0, 0);
    }
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  return 1;
}

//}

//{ Read

static void lluv_on_stream_read_cb(uv_stream_t* arg, ssize_t nread, const uv_buf_t* buf){
  lluv_handle_t *handle = lluv_handle_byptr((uv_handle_t*)arg);
  lua_State *L = LLUV_HCALLBACK_L(handle);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  if(!IS_(handle, OPEN)){
    lluv_free_buffer((uv_handle_t*)arg, buf);
    return;
  }

  lua_rawgeti(L, LLUV_LUA_REGISTRY, LLUV_READ_CB(handle));
  assert(!lua_isnil(L, -1));

  lluv_handle_pushself(L, handle);  

  if(nread >= 0){
    lua_pushnil(L);
    lua_pushlstring(L, buf->base, nread);
    lluv_free_buffer((uv_handle_t*)arg, buf);
  }
  else{
    lluv_free_buffer((uv_handle_t*)arg, buf);

    /* The callee is responsible for stopping closing the stream 
     *  when an error happens by calling uv_read_stop() or uv_close().
     *  Trying to read from the stream again is undefined.
     */
    uv_read_stop(arg);

    luaL_unref(L, LLUV_LUA_REGISTRY, LLUV_READ_CB(handle));
    LLUV_READ_CB(handle) = LUA_NOREF;

    lluv_error_create(L, LLUV_ERR_UV, (uv_errno_t)nread, NULL);
    lua_pushnil(L);

    lluv_handle_unlock(L, handle, LLUV_LOCK_READ);
  }

  LLUV_HANDLE_CALL_CB(L, handle, 3);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

static int lluv_stream_start_read(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  int err;

  lluv_check_args_with_cb(L, 2);
  LLUV_READ_CB(handle) = luaL_ref(L, LLUV_LUA_REGISTRY);

  err = uv_read_start(LLUV_H(handle, uv_stream_t), lluv_alloc_buffer_cb, lluv_on_stream_read_cb);
  if(err >= 0) lluv_handle_lock(L, handle, LLUV_LOCK_READ);
  return lluv_return(L, handle, LLUV_READ_CB(handle), err);
}

static int lluv_stream_stop_read(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  int err;

  lluv_check_none(L, 2);

  err = uv_read_stop(LLUV_H(handle, uv_stream_t));
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  if(LLUV_READ_CB(handle) != LUA_NOREF){
    lluv_handle_unlock(L, handle, LLUV_LOCK_READ);
    luaL_unref(L, LLUV_LUA_REGISTRY, LLUV_READ_CB(handle));
    LLUV_READ_CB(handle) = LUA_NOREF;
  }

  lua_settop(L, 1);
  return 1;
}

//}

//{ Write

static int lluv_stream_try_write(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  size_t len; const char *str = luaL_checklstring(L, 2, &len);
  int err; uv_buf_t buf = lluv_buf_init((char*)str, len);

  lluv_check_none(L, 3);

  err = uv_try_write(LLUV_H(handle, uv_stream_t), &buf, 1);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  lua_pushinteger(L, err);
  return 1;
}

static void lluv_on_stream_write_cb(uv_write_t* arg, int status){
  lluv_req_t    *req    = lluv_req_byptr((uv_req_t*)arg);
  lluv_handle_t *handle = req->handle;
  lua_State     *L      = LLUV_HCALLBACK_L(handle);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  if(!IS_(handle, OPEN)){
    lluv_req_free(L, req);

    LLUV_CHECK_LOOP_CB_INVARIANT(L);
    return;
  }

  lua_rawgeti(L, LLUV_LUA_REGISTRY, req->cb);
  lluv_handle_pushself(L, handle);
  lua_rawgeti(L, LLUV_LUA_REGISTRY, req->ctx);
  lluv_req_free(L, req);

  if(lua_isnil(L, -3)){
    lua_pop(L, 3);

    LLUV_CHECK_LOOP_CB_INVARIANT(L);
    return;
  }

  lluv_push_status(L, status);
  lua_insert(L, -2);

  LLUV_HANDLE_CALL_CB(L, handle, 3);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

static int lluv_stream_write_(lua_State *L, lluv_handle_t *handle, uv_buf_t *buf, size_t n){
  int err; lluv_req_t *req;

  if(lua_gettop(L) == 4){
    int ctx;
    lluv_check_callable(L, -2);
    ctx = luaL_ref(L, LLUV_LUA_REGISTRY);
    req = lluv_req_new(L, UV_WRITE, handle);
    lluv_req_ref(L, req); /* string/table */
    req->ctx = ctx;
  }
  else{
    if(lua_gettop(L) == 2)
      lua_settop(L, 3);
    else
      lluv_check_args_with_cb(L, 3);

    req = lluv_req_new(L, UV_WRITE, handle);
    lluv_req_ref(L, req); /* string/table */
  }

  err = uv_write(LLUV_R(req, write), LLUV_H(handle, uv_stream_t), buf, n, lluv_on_stream_write_cb);

  return lluv_return_req(L, handle, req, err);
}

static int lluv_stream_write_t(lua_State *L, lluv_handle_t  *handle){
  int i, n = lua_rawlen(L, 2);
  uv_buf_t *buf;
  
  assert(lua_type(L, 2) == LUA_TTABLE);

  luaL_argcheck(L, n > 0, 2, "Empty array not supported");
  
  buf = (uv_buf_t*)lluv_alloca(sizeof(uv_buf_t) * n);
  if(!buf){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, ENOMEM, NULL); 
  }

  for(i = 0; i < n; ++i){
    size_t len; const char *str;
    lua_rawgeti(L, 2, i + 1);
    str = luaL_checklstring(L, -1, &len);
    buf[i] = lluv_buf_init((char*)str, len);
    lua_pop(L, 1);
  }

  return lluv_stream_write_(L, handle, buf, n);
}

static int lluv_stream_write(lua_State *L){
  lluv_handle_t  *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  if(lua_type(L, 2) == LUA_TTABLE){
    return lluv_stream_write_t(L, handle);
  }
  else{
    size_t len; const char *str = luaL_checklstring(L, 2, &len);
    uv_buf_t buf = lluv_buf_init((char*)str, len);
    return lluv_stream_write_(L, handle, &buf, 1);
  }
}

static int lluv_stream_write2(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  lluv_handle_t *src    = lluv_check_stream(L, 2, LLUV_FLAG_OPEN);
  size_t len; const char *str;
  int err; lluv_req_t *req;
  uv_buf_t buf;

  if(lua_isfunction(L, 3)){
    lua_pushliteral(L, ".");
    lua_insert(L, 3);
  }
  str = luaL_checklstring(L, 3, &len);
  buf = lluv_buf_init((char*)str, len);

  if(lua_gettop(L) == 3)
    lua_settop(L, 4);
  else
    lluv_check_args_with_cb(L, 4);

  req = lluv_req_new(L, UV_WRITE, handle);
  lluv_req_ref(L, req); /* string */

  err = uv_write2(LLUV_R(req, write), LLUV_H(handle, uv_stream_t), &buf, 1, LLUV_H(src, uv_stream_t), lluv_on_stream_write_cb);

  return lluv_return_req(L, handle, req, err);
}

//}

static int lluv_stream_is_readable(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  lua_settop(L, 1);
  lua_pushboolean(L, uv_is_readable(LLUV_H(handle, uv_stream_t)));
  return 1;
}

static int lluv_stream_is_writable(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  lua_settop(L, 1);
  lua_pushboolean(L, uv_is_writable(LLUV_H(handle, uv_stream_t)));
  return 1;
}

static int lluv_stream_set_blocking(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  int block = luaL_opt(L, lua_toboolean, 2, 1);
  int err;

  lua_settop(L, 1);

  err = uv_stream_set_blocking(LLUV_H(handle, uv_stream_t), block);
  if(err < 0){
    return lluv_fail(L, handle->flags, LLUV_ERR_UV, err, NULL);
  }

  return 1;
}

static int lluv_stream_get_write_queue_size(lua_State *L){
  lluv_handle_t *handle = lluv_check_stream(L, 1, LLUV_FLAG_OPEN);
  size_t queue_size;

  lua_settop(L, 1);

#if LLUV_UV_VER_GE(1,19,0)
  queue_size = uv_stream_get_write_queue_size(LLUV_H(handle, uv_stream_t));
#else
  queue_size = LLUV_H(handle, uv_stream_t)->write_queue_size;
#endif

  lutil_pushint64(L, queue_size);

  return 1;
}

UV_EXTERN size_t uv_stream_get_write_queue_size(const uv_stream_t* stream);

static const struct luaL_Reg lluv_stream_methods[] = {
  { "shutdown",             lluv_stream_shutdown              },
  { "listen",               lluv_stream_listen                },
  { "accept",               lluv_stream_accept                },
  { "start_read",           lluv_stream_start_read            },
  { "stop_read",            lluv_stream_stop_read             },
  { "try_write",            lluv_stream_try_write             },
  { "write",                lluv_stream_write                 },
  { "write2",               lluv_stream_write2                },
  { "readable",             lluv_stream_is_readable           },
  { "writable",             lluv_stream_is_writable           },
  { "set_blocking",         lluv_stream_set_blocking          },
  { "get_write_queue_size", lluv_stream_get_write_queue_size  },
  
  {NULL,NULL}
};

static const struct luaL_Reg lluv_stream_functions[] = {

  {NULL,NULL}
};

LLUV_INTERNAL void lluv_stream_initlib(lua_State *L, int nup, int safe){
  lutil_pushnvalues(L, nup);
  if(!lutil_createmetap(L, LLUV_STREAM, lluv_stream_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_stream_functions, nup);
}
