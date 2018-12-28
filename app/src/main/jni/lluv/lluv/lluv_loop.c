/******************************************************************************
* Author: Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Copyright (C) 2014-2017 Alexey Melnichuk <alexeymelnichuck@gmail.com>
*
* Licensed according to the included 'LICENSE' document
*
* This file is part of lua-lluv library.
******************************************************************************/

#include "lluv_loop.h"
#include "lluv_error.h"
#include "lluv_utils.h"
#include "lluv_handle.h"
#include "lluv_list.h"
#include <assert.h>

#ifndef LLUV_DEFER_DEPTH
#  define LLUV_DEFER_DEPTH 10
#endif

#define LLUV_LOOP_NAME LLUV_PREFIX" Loop"
static const char *LLUV_LOOP = LLUV_LOOP_NAME;

static const char *LLUV_DEFAULT_LOOP_TAG = LLUV_PREFIX" default loop";

LLUV_INTERNAL lluv_loop_t* lluv_push_default_loop(lua_State *L){
  lua_rawgetp(L, LLUV_LUA_REGISTRY, LLUV_DEFAULT_LOOP_TAG);
  if(lua_isnil(L, -1)){
    lua_pop(L, 1);
#ifdef LLUV_USE_UV_DEFAULT_LOOP
    lluv_loop_create(L, uv_default_loop(), LLUV_FLAG_DEFAULT_LOOP);
#else
    {
      uv_loop_t *loop = lluv_alloc_t(L, uv_loop_t);
      int err = uv_loop_init(loop);
      if(err < 0){
        lluv_fail(L, LLUV_FLAG_RAISE_ERROR, LLUV_ERR_UV, err, NULL);
        return 0;
      }
      lluv_loop_create(L, loop, 0);
    }
#endif
    lua_pushvalue(L, -1);
    lua_rawsetp(L, LLUV_LUA_REGISTRY, LLUV_DEFAULT_LOOP_TAG);
  }
  assert(lutil_isudatap(L, -1, LLUV_LOOP));
  return lua_touserdata(L, -1);
}

LLUV_INTERNAL lluv_loop_t* lluv_default_loop(lua_State *L){
  lluv_loop_t *loop = lluv_push_default_loop(L);
  lua_pop(L, 1);
  return loop;
}

LLUV_INTERNAL lluv_loop_t* lluv_ensure_loop_at(lua_State *L, int idx){
  lluv_loop_t *loop = lluv_opt_loop(L, idx, 0);
  if(loop) return loop;
  idx = lua_absindex(L, idx);
  loop = lluv_push_default_loop(L);
  lua_insert(L, idx);
  return loop;
}

LLUV_INTERNAL int lluv_loop_create(lua_State *L, uv_loop_t *h, lluv_flags_t flags){
  lluv_loop_t *loop = lutil_newudatap(L, lluv_loop_t, LLUV_LOOP);
  loop->L            = L;
  loop->handle       = h;
  loop->handle->data = loop;
  loop->flags        = flags | LLUV_FLAG_OPEN;
  loop->level        = 0;
  loop->buffer_size  = LLUV_BUFFER_SIZE;
  lluv_list_init(L, &loop->defer);

  lua_pushvalue(L, -1);
  lua_rawsetp(L, LLUV_LUA_REGISTRY, h);
  return 1;
}

LLUV_INTERNAL lluv_loop_t* lluv_check_loop(lua_State *L, int idx, lluv_flags_t flags){
  lluv_loop_t *loop = (lluv_loop_t *)lutil_checkudatap (L, idx, LLUV_LOOP);
  luaL_argcheck (L, loop != NULL, idx, LLUV_LOOP_NAME" expected");

  luaL_argcheck (L, FLAGS_IS_SET(loop->flags, flags), idx, LLUV_LOOP_NAME" closed");
  return loop;
}

LLUV_INTERNAL lluv_loop_t* lluv_opt_loop(lua_State *L, int idx, lluv_flags_t flags){
  if(!lutil_isudatap(L, idx, LLUV_LOOP)) return NULL;
  return lluv_check_loop(L, idx, flags);
}

LLUV_INTERNAL lluv_loop_t* lluv_opt_loop_ex(lua_State *L, int idx, lluv_flags_t flags){
  if(!lutil_isudatap(L, idx, LLUV_LOOP)) return lluv_default_loop(L);
  return lluv_check_loop(L, idx, flags);
}

LLUV_INTERNAL lluv_loop_t* lluv_loop_byptr(uv_loop_t *h){
  lluv_loop_t *loop = h->data;
  assert(loop->handle == h);
  return loop;
}

LLUV_INTERNAL lluv_loop_t* lluv_loop_by_handle(uv_handle_t* h){
  lluv_handle_t *handle = lluv_handle_byptr(h);
  return lluv_loop_byptr(handle->handle.loop);
}

LLUV_INTERNAL void lluv_loop_pushself(lua_State *L, lluv_loop_t *loop){
  lua_rawgetp(L, LLUV_LUA_REGISTRY, loop->handle);
  assert(loop == lua_touserdata(L, -1));
}

static int lluv_defer_call(lua_State *L){
  int i, n = lua_tointeger(L, lua_upvalueindex(1));
  luaL_checkstack(L, n, "too many arguments");

  assert(lua_isfunction(L, lua_upvalueindex(2)));
  assert(!lua_isnone(L, lua_upvalueindex(n + 1)));

  for(i = 2; i <= n+1; ++i)lua_pushvalue(L, lua_upvalueindex(i));

  lua_call(L, n - 1, 0);
  return 0;
}

LLUV_INTERNAL void lluv_loop_defer_call(lua_State *L, lluv_loop_t *loop, int nargs){
  assert(lua_isfunction(L, -1-nargs));

  luaL_checkstack(L, 1, "too many arguments");
  lua_pushinteger(L, nargs+1);
  lua_insert(L, -2-nargs);

  lua_pushcclosure(L, lluv_defer_call, nargs + 2);
  lluv_list_push_back(L, &loop->defer);
}

LLUV_INTERNAL int lluv_loop_defer_proceed(lua_State *L, lluv_loop_t *loop){
  int top = lua_gettop(L);
  int i;
  for(i = 0; i < LLUV_DEFER_DEPTH; ++i){
    size_t s = lluv_list_size(L, &loop->defer);
    for(; s != 0; --s){
      int err = lluv_list_pop_front(L, &loop->defer);
      assert(err == 1);
      assert((top+1) == lua_gettop(L));
      err = lluv_lua_call(L, 0, 0);
      assert(top == lua_gettop(L));
      if(err) return err; 
    }
  }
  return 0;
}

static int lluv_loop_new_impl(lua_State *L, lluv_flags_t flags){
  uv_loop_t *loop = lluv_alloc_t(L, uv_loop_t);
  int err = uv_loop_init(loop);
  if(err < 0){
    lluv_free_t(L, uv_loop_t, loop);
    return lluv_fail(L, flags, LLUV_ERR_UV, err, NULL);
  }
  lluv_loop_create(L, loop, flags);
  return 1;
}

static int lluv_loop_new(lua_State *L){
  return lluv_loop_new_impl(L, 0);
}

typedef struct lluv_close_walk_ctx_tag{
  lua_State *L; 
  uint32_t  count;
} lluv_close_walk_ctx_t;

static void lluv_loop_on_walk_close(uv_handle_t* handle, void* arg){
  lluv_close_walk_ctx_t *ctx = (lluv_close_walk_ctx_t *)arg;
  lua_State *L = ctx->L;

   /* in any case we should call uv_run for this handle */
  ctx->count += 1;

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  if(uv_is_closing(handle)){
    return;
  }

  lluv_handle_pushself(L, lluv_handle_byptr(handle));
  if(lua_isnil(L, -1)){
    /* This handle create some one else or
     * someone corrupt registry
     * but still we need close it
     */
    lua_pop(L, 1);
    uv_close(handle, NULL);
    return;
  }

  lua_getfield(L, -1, "close");
  if(lua_isnil(L, -1)){
    /* is it even possible?
     * too late to cry
     */
    assert(0 && "broken metatable");

    lua_pop(L, 2);
    uv_close(handle, NULL);
    return;
  }

  lua_insert(L, -2);
  lua_pcall(L, 1, 0, 0);

  lua_settop(L, LLUV_CALLBACK_TOP_SIZE);
}

static int lluv_loop_close_all_handles_impl(lua_State *L){
  /* NOTE. if you have fs callbacks then this function
  ** would call all this function because there no handles.
  */

  lluv_loop_t* loop = lluv_check_loop(L, LLUV_LOOP_INDEX, LLUV_FLAG_OPEN);
  lluv_close_walk_ctx_t arg = {L, 0};
  int err = 0;

  uv_walk(loop->handle, lluv_loop_on_walk_close, &arg);

  if(arg.count){
    lua_State *prev_state = loop->L;

    loop->level += 1;
    loop->L = L;

    LLUV_CHECK_LOOP_CB_INVARIANT(loop->L);

    while((err = uv_run(loop->handle, UV_RUN_ONCE))){
      if(err < 0) break;
      LLUV_CHECK_LOOP_CB_INVARIANT(loop->L);
    }

    loop->L = prev_state;
    loop->level -= 1;

    if(err < 0)
      return lluv_fail(L, loop->flags, LLUV_ERR_UV, err, NULL);
  }

  return 0;
}

static int lluv_loop_close_all_handles(lua_State *L){
  /* lluv_loop_t* loop = */ lluv_check_loop(L, 1, LLUV_FLAG_OPEN);
  lua_settop(L, 1);

  lua_pushvalue(L, LLUV_LUA_REGISTRY); lua_pushvalue(L, LLUV_LUA_HANDLES);
  lua_pushvalue(L, 1);                                      /* loop, reg, handles, loop   */
  lua_pushnil(L); lua_pushnil(L);                           /* loop, reg, handles, loop, err, mark */
  lua_pushcclosure(L, lluv_loop_close_all_handles_impl, 5); /* loop, closure              */
  lua_call(L, 0, LUA_MULTRET);                              /* loop, ...                  */
  return lua_gettop(L) - 1;
}

static int lluv_loop_close_impl(lua_State *L, int ignore_error, int close_handle){
  lluv_loop_t* loop = lluv_check_loop(L, 1, 0);
  int err;

  if(!IS_(loop, OPEN)) return 0;

  if((!close_handle) && lua_isboolean(L, 2)){
    close_handle = lua_toboolean(L, 2);
  }

  if(close_handle){
    int ret = lluv_loop_close_all_handles(L);
    if(!ignore_error){
      if(ret != 0) return ret;
    }
  }

  err = uv_loop_close(loop->handle);
  if(!ignore_error){
    if(err < 0){
      return lluv_fail(L, loop->flags, LLUV_ERR_UV, err, NULL);
    }
  }

  lua_rawgetp(L, LLUV_LUA_REGISTRY, LLUV_DEFAULT_LOOP_TAG);
  if(lua_rawequal(L, 1, -1)){
    lua_pushnil(L);
    lua_rawsetp(L, LLUV_LUA_REGISTRY, LLUV_DEFAULT_LOOP_TAG);
  }
  lua_pop(L, 1);

  UNSET(loop, LLUV_FLAG_OPEN);
  lua_pushnil(L);
  lua_rawsetp(L, LLUV_LUA_REGISTRY, loop->handle);

  if(!IS_(loop, DEFAULT_LOOP)){
    lluv_free_t(L, uv_alloc_t, loop->handle);
  }

  loop->handle = NULL;
  lluv_list_close(L, &loop->defer);
  return 0;
}

static int lluv_loop_close(lua_State *L){
  if(!lluv_opt_loop(L, 1, 0)){
    lua_rawgetp(L, LLUV_LUA_REGISTRY, LLUV_DEFAULT_LOOP_TAG);
    if(lua_isnil(L, -1)) return 0;
    lua_pop(L, 1);
  }

  lluv_ensure_loop_at(L, 1);
  return lluv_loop_close_impl(L, 0, 0);
}

static int lluv_loop__gc(lua_State *L){
  lluv_check_loop(L, 1, 0);
  return lluv_loop_close_impl(L, 1, 1);
}

static int lluv_loop_to_s(lua_State *L){
  lluv_loop_t* loop = lluv_check_loop(L, 1, 0);
  lua_pushfstring(L, LLUV_LOOP_NAME" (%p)", loop);
  return 1;
}

static int lluv_loop_run_impl(lua_State *L){
  lluv_loop_t* loop = lluv_check_loop(L, LLUV_LOOP_INDEX, 0);
  uv_run_mode  mode = (uv_run_mode)luaL_checkinteger(L, 1);
  int err;
  lua_State *prev_state = loop->L;

  lua_pop(L, 1);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  loop->level += 1;
  loop->L = L;
  err = lluv_loop_defer_proceed(L, loop);
  if(!err) err = uv_run(loop->handle, mode);
  if(!err) err = lluv_loop_defer_proceed(L, loop);
  loop->L = prev_state;
  loop->level -= 1;

  if(err < 0){
    return lluv_fail(L, loop->flags, LLUV_ERR_UV, err, NULL);
  }

  if(lua_touserdata(L, LLUV_ERROR_MARK_INDEX) == LLUV_MEMORY_ERROR_MARK){
    lua_error(L);
  }
  else if(!lua_isnil(L, LLUV_ERROR_MARK_INDEX)){
    lua_pushvalue(L, LLUV_ERROR_MARK_INDEX);
    return lua_error(L);
  }

  lua_pushinteger(L, err);
  return 1;
}

static int lluv_loop_run(lua_State *L){
  static const lluv_uv_const_t FLAGS[] = {
    { UV_RUN_DEFAULT, "default" },
    { UV_RUN_ONCE,    "once"    },
    { UV_RUN_NOWAIT,  "nowait"  },

    { 0, NULL }
  };

  lluv_ensure_loop_at(L, 1);

  if(lua_isnumber(L, 2) || lua_isstring(L, 2)){
    uv_run_mode mode = (uv_run_mode)lluv_opt_named_const(L, 2, UV_RUN_DEFAULT, FLAGS);
    lua_pushinteger(L, mode);
    lua_replace(L, 2);
  }
  else{
    if(lua_isnil(L, 2)) lua_remove(L, 2);
    lua_pushinteger(L, UV_RUN_DEFAULT);
    lua_insert(L, 2);
  }

  lua_settop(L, 3);

  if(!lua_isfunction(L,3)){
    lua_pop(L, 1);
    lua_pushnil(L);
  }

                                              /* loop, mode, err */
  lua_insert(L, 2);                           /* loop, err, mode */
  lua_insert(L, 1);                           /* mode, loop, err  */
  lua_pushvalue(L, LLUV_LUA_REGISTRY);        /* mode, loop, err, reg */
  lua_insert(L, 2);                           /* mode, reg, loop, err */
  lua_pushvalue(L, LLUV_LUA_HANDLES);         /* mode, reg, loop, err,  */
  lua_insert(L, 3);                           /* mode, reg, handles, loop, err */
  lua_pushnil(L);                             /* mode, reg, handles, loop, err, mark */
  lua_pushcclosure(L, lluv_loop_run_impl, 5); /* mode, closure */
  lua_insert(L, 1);                           /* closure, mode */
  lua_call(L, 1, LUA_MULTRET);

  return lua_gettop(L);
}

static int lluv_loop_alive(lua_State *L){
  lluv_loop_t* loop = lluv_check_loop(L, 1, LLUV_FLAG_OPEN);
  lua_pushboolean(L, uv_loop_alive(loop->handle));
  return 1;
}

static int lluv_loop_update_time(lua_State *L){
  lluv_loop_t* loop = lluv_ensure_loop_at(L, 1);
  lluv_check_loop(L, 1, LLUV_FLAG_OPEN);
  uv_update_time(loop->handle);
  lua_settop(L, 1);
  return 1;
}

static int lluv_loop_fileno(lua_State *L){
  lluv_loop_t* loop = lluv_check_loop(L, 1, LLUV_FLAG_OPEN);
  lutil_pushint64(L, uv_backend_fd(loop->handle));
  return 1;
}

static int lluv_loop_poll_timeout(lua_State *L){
  lluv_loop_t* loop = lluv_check_loop(L, 1, LLUV_FLAG_OPEN);
  lutil_pushint64(L, uv_backend_timeout(loop->handle));
  return 1;
}

static int lluv_loop_stop(lua_State *L){
  lluv_loop_t* loop = lluv_opt_loop_ex(L, 1, LLUV_FLAG_OPEN);
  uv_stop(loop->handle);
  return 0;
}

static int lluv_loop_now(lua_State *L){
  lluv_loop_t* loop = lluv_opt_loop_ex(L, 1, LLUV_FLAG_OPEN);
  uint64_t now = uv_now(loop->handle);
  lutil_pushint64(L, now);
  return 1;
}

static int lluv_loop_defer(lua_State *L){
  int n; lluv_loop_t* loop;

  if(!lutil_isudatap(L, 1, LLUV_LOOP)){
    loop = lluv_default_loop(L);
    n = 1;
  }
  else{
    loop = lluv_check_loop(L, 1, LLUV_FLAG_OPEN);
    n = 2;
  }

  luaL_argcheck (L, lua_isfunction(L, n), n, "function expected");

  n = lua_gettop(L) - n;

  lluv_loop_defer_call(L, loop, n);

  return 0;
}

static void lluv_loop_on_walk(uv_handle_t* handle, void* arg){
  lua_State *L = (lua_State*)arg;

  lua_settop(L, 2); lua_pushvalue(L, -1);
  lluv_handle_pushself(L, lluv_handle_byptr(handle));
  lua_call(L, 1, 0);
}

static void lluv_loop_on_collect(uv_handle_t* handle, void* arg){
  lua_State *L = (lua_State*)arg;

  assert(lua_gettop(L) == 2);
  assert(lua_istable(L, 2));

  lluv_handle_pushself(L, lluv_handle_byptr(handle));
  lua_rawseti(L, 2, lua_rawlen(L, 2) + 1);

  assert(lua_gettop(L) == 2);
}

static int lluv_loop_handles(lua_State *L){
  lluv_loop_t* loop = lluv_ensure_loop_at(L, 1);

  lluv_check_loop(L, 1, LLUV_FLAG_OPEN);

  if(lua_isfunction(L, 2)){
    uv_walk(loop->handle, lluv_loop_on_walk, L);
    return 0;
  }

  lua_settop(L, 1);
  lua_newtable(L);
  uv_walk(loop->handle, lluv_loop_on_collect, L);
  assert(lua_gettop(L) == 2);
  return 1;
}

static int lluv_push_default_loop_l(lua_State *L){
  lluv_push_default_loop(L);
  return 1;
}

static const struct luaL_Reg lluv_loop_methods[] = {
  { "__gc",         lluv_loop__gc          },
  { "__tostring",   lluv_loop_to_s         },
  { "run",          lluv_loop_run          },
  { "close",        lluv_loop_close        },
  { "alive",        lluv_loop_alive        },
  { "stop",         lluv_loop_stop         },
  { "now",          lluv_loop_now          },
  { "handles",      lluv_loop_handles      },
  { "defer",        lluv_loop_defer        },
  { "fileno",       lluv_loop_fileno       },
  { "poll_timeout", lluv_loop_poll_timeout },
  { "update_time",  lluv_loop_update_time  },
  
  { "close_all_handles", lluv_loop_close_all_handles },

  {NULL,NULL}
};

static const lluv_uv_const_t lluv_loop_constants[] = {
  { UV_RUN_DEFAULT,  "RUN_DEFAULT"  },
  { UV_RUN_ONCE,     "RUN_ONCE"     },
  { UV_RUN_NOWAIT,   "RUN_NOWAIT"   },

  { 0, NULL }
};

static const struct luaL_Reg lluv_loop_functions[] = {
  {"loop",         lluv_loop_new           },

  {"run",          lluv_loop_run           },
  {"stop",         lluv_loop_stop          },
  {"handles",      lluv_loop_handles       },
  {"close",        lluv_loop_close         },
  {"now",          lluv_loop_now           },
  {"default_loop", lluv_push_default_loop_l},
  {"update_time",  lluv_loop_update_time   },

  {"defer",        lluv_loop_defer         },

  {NULL,NULL}
};

LLUV_INTERNAL void lluv_loop_initlib(lua_State *L, int nup){
  lutil_pushnvalues(L, nup);

  if(!lutil_createmetap(L, LLUV_LOOP, lluv_loop_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_loop_functions, nup);
  lluv_register_constants(L, lluv_loop_constants);
}