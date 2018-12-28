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
#include "lluv_fs.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include "lluv_loop.h"
#include "lluv_handle.h"
#include "lluv_stream.h"
#include "lluv_pipe.h"
#include "lluv_fbuf.h"
#include <assert.h>
#include <fcntl.h>

#ifndef _WIN32

#include <unistd.h>

#endif

/* callback signatures  callback(loop|file, err|nil, ...)
** one exaption is fs_open callback(file|nil, err|nil, path)
**
**/

typedef struct lluv_fs_request_tag{
  uv_fs_t req;
  lua_State *L;
  int cb;
  int file_ref;
}lluv_fs_request_t;

#define LLUV_FCALLBACK_L(H) (lluv_loop_byptr(H->req.loop)->L)

static lluv_fs_request_t *lluv_fs_request_new(lua_State *L){
  lluv_fs_request_t *req = lluv_alloc_t(L, lluv_fs_request_t);
  req->L        = L;
  req->req.data = req;
  req->cb = req->file_ref = LUA_NOREF;
  return req;
}

static void lluv_fs_request_free(lua_State *L, lluv_fs_request_t *req){
  if(req->cb != LUA_NOREF)
    luaL_unref(L, LLUV_LUA_REGISTRY, req->cb);
  if(req->file_ref != LUA_NOREF)
    luaL_unref(L, LLUV_LUA_REGISTRY, req->file_ref);
  lluv_free_t(L, lluv_fs_request_t, req);
}

#ifndef O_SYNC
  #define O_SYNC 0
#endif

static int lluv_file_create(lua_State *L, lluv_loop_t  *loop, uv_file h, unsigned char flags);

static int lluv_push_fs_result_object(lua_State* L, lluv_fs_request_t* lreq) {
  uv_fs_t *req = &lreq->req;
  lluv_loop_t *loop = lluv_loop_byptr(req->loop);

  switch (req->fs_type) {
    case UV_FS_RENAME:
    case UV_FS_UNLINK:
    case UV_FS_RMDIR:
    case UV_FS_MKDIR:
    case UV_FS_MKDTEMP:
    case UV_FS_UTIME:
    case UV_FS_CHMOD:
    case UV_FS_LINK:
    case UV_FS_SYMLINK:
    case UV_FS_CHOWN:
    case UV_FS_READLINK:
    case UV_FS_SCANDIR:
    case UV_FS_STAT:
    case UV_FS_LSTAT:
    case UV_FS_ACCESS:
#if LLUV_UV_VER_GE(1,8,0)
    case UV_FS_REALPATH:
#endif
#if LLUV_UV_VER_GE(1,14,0)
    case UV_FS_COPYFILE:
#endif
      lua_pushvalue(L, LLUV_LOOP_INDEX);
      return 1;

    case UV_FS_OPEN:
      if(req->result < 0) lua_pushnil(L);
      else lluv_file_create(L, loop, (uv_file)req->result, 0);
      return 1;

    case UV_FS_CLOSE:
    case UV_FS_FTRUNCATE:
    case UV_FS_FSYNC:
    case UV_FS_FDATASYNC:
    case UV_FS_FUTIME:
    case UV_FS_FCHMOD:
    case UV_FS_FCHOWN:
    case UV_FS_FSTAT:
    case UV_FS_WRITE:
    case UV_FS_READ:
      lua_rawgeti(L, LLUV_LUA_REGISTRY, lreq->file_ref);
      return 1;

    case UV_FS_SENDFILE:
      lua_rawgeti(L, LLUV_LUA_REGISTRY, lreq->file_ref);
      return 1;

    default:
      fprintf(stderr, "UNKNOWN FS TYPE %d\n", req->fs_type);
      return 0;
  }
}

#define LLUV_DIRENT_MAP(XX)        \
  XX("unknown", UV_DIRENT_UNKNOWN) \
  XX("file",    UV_DIRENT_FILE)    \
  XX("dir",     UV_DIRENT_DIR)     \
  XX("link",    UV_DIRENT_LINK)    \
  XX("fifo",    UV_DIRENT_FIFO)    \
  XX("socket",  UV_DIRENT_SOCKET)  \
  XX("char",    UV_DIRENT_CHAR)    \
  XX("block",   UV_DIRENT_BLOCK)   \

static int lluv_push_fs_result(lua_State* L, lluv_fs_request_t* lreq) {
  uv_fs_t *req = &lreq->req;
  /*lluv_loop_t *loop = req->loop->data;*/

  switch (req->fs_type) {
    case UV_FS_RENAME:
    case UV_FS_UNLINK:
    case UV_FS_RMDIR:
    case UV_FS_MKDIR:
    case UV_FS_MKDTEMP:
    case UV_FS_UTIME:
    case UV_FS_CHMOD:
    case UV_FS_LINK:
    case UV_FS_SYMLINK:
    case UV_FS_CHOWN:
      lua_pushstring(L, req->path);
      return 1;

    case UV_FS_CLOSE:
    case UV_FS_FTRUNCATE:
    case UV_FS_FSYNC:
    case UV_FS_FDATASYNC:
    case UV_FS_FUTIME:
    case UV_FS_FCHMOD:
    case UV_FS_FCHOWN:
    case UV_FS_OPEN:
      if(req->path) lua_pushstring(L, req->path);
      else lua_pushboolean(L, 1);
      return 1;

    case UV_FS_SENDFILE:
      lutil_pushint64(L, req->result);
      return 1;

    case UV_FS_STAT:
    case UV_FS_LSTAT:
      lluv_push_stat(L, &req->statbuf);
      lua_pushstring(L, req->path);
      return 2;

    case UV_FS_FSTAT:
      lluv_push_stat(L, &req->statbuf);
      return 1;

    case UV_FS_READLINK:
#if LLUV_UV_VER_GE(1,8,0)
    case UV_FS_REALPATH:
#endif
      lua_pushstring(L, (char*)req->ptr);
      return 1;

    case UV_FS_WRITE:
    case UV_FS_READ:
      lua_rawgetp(L, LLUV_LUA_REGISTRY, req);
      lua_pushnil(L); lua_rawsetp(L, LLUV_LUA_REGISTRY, req);
      lutil_pushint64(L, req->result);
      return 2;

    case UV_FS_SCANDIR:{
      uv_dirent_t ent;
      int i = 0, err;
      lua_createtable(L, (int)req->result, 0);
      lua_createtable(L, (int)req->result, 0);
      while((err = uv_fs_scandir_next(req, &ent)) >= 0){
        lua_pushstring (L, ent.name); lua_rawseti(L, -3, ++i);
#define XX(C,S) case S: lua_pushliteral(L, C); lua_rawseti(L, -2, i); break;
          switch(ent.type){
            LLUV_DIRENT_MAP(XX)
            default: lua_pushstring(L, "unknown"); lua_rawseti(L, -2, i);
          }
#undef XX
      }
      lua_pushstring(L, req->path);
      lua_insert(L, -2);
      return 3;
    }

    case UV_FS_ACCESS:{
      lua_pushboolean(L, req->result == 0);
      lua_pushstring(L, req->path);
      return 2;
    }

#if LLUV_UV_VER_GE(1,14,0)
    case UV_FS_COPYFILE:
      lua_pushboolean(L, req->result == 0);
      return 1;
#endif

    default:
      fprintf(stderr, "UNKNOWN FS TYPE %d\n", req->fs_type);
      return 0;
  }
}

static void lluv_on_fs(uv_fs_t *arg){
  lluv_fs_request_t *req = arg->data;
  lua_State *L = LLUV_FCALLBACK_L(req);
  lluv_loop_t *loop = lluv_loop_byptr(req->req.loop);
  int argc;

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  lua_rawgeti(L, LLUV_LUA_REGISTRY, req->cb);

  argc = lluv_push_fs_result_object(L, req);

  if(arg->result < 0){
    lluv_error_create(L, LLUV_ERR_UV, arg->result, arg->path);
    ++argc;
  }
  else{
    lua_pushnil(L);
    argc += 1 + lluv_push_fs_result(L, req);
  }
  uv_fs_req_cleanup(&req->req);
  lluv_fs_request_free(L, req);

  LLUV_LOOP_CALL_CB(L, loop, argc);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

//{ Macro
#define LLUV_CHECK_LOOP_FS()                                              \
  lluv_loop_t *loop  = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);               \
  int argc = loop? 1 : 0;                                                 \

#define LLUV_PRE_FS(){                                                    \
  lluv_fs_request_t *req = lluv_fs_request_new(L);                        \
  int err;  uv_fs_cb cb = NULL;                                           \
                                                                          \
  if(!loop)loop = lluv_default_loop(L);                                   \
                                                                          \
  if(lua_gettop(L) > argc){                                               \
    lua_settop(L, argc + 1);                                              \
    cb = lluv_on_fs;                                                      \
  }                                                                       \

#define LLUV_POST_FS_FILE()                                               \
  if(err < 0){                                                            \
    lluv_fs_request_free(L, req);                                         \
    if(!cb){                                                              \
      return lluv_fail(L, f->flags, LLUV_ERR_UV, err, path);              \
    }                                                                     \
    lua_pushvalue(L, 1);                                                  \
    lluv_error_create(L, LLUV_ERR_UV, err, path);                         \
    lluv_loop_defer_call(L, loop, 2);                                     \
    lua_pushboolean(L, 1);                                                \
    return 1;                                                             \
  }                                                                       \

#define LLUV_POST_FS_LOOP()                                               \
  if(err < 0){                                                            \
    lluv_fs_request_free(L, req);                                         \
    if(!cb){                                                              \
      return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, err, path); \
    }                                                                     \
    lluv_loop_pushself(L, loop);                                          \
    lluv_error_create(L, LLUV_ERR_UV, err, path);                         \
    lluv_loop_defer_call(L, loop, 2);                                     \
    lua_pushboolean(L, 1);                                                \
    return 1;                                                             \
  }                                                                       \

#define LLUV_POST_FS_COMMON()                                             \
                                                                          \
  if(cb){                                                                 \
    req->cb = luaL_ref(L, LLUV_LUA_REGISTRY);                             \
    lua_pushboolean(L, 1);                                                \
    return 1;                                                             \
  }                                                                       \
                                                                          \
  if(req->req.result < 0){                                                \
    lua_pushnil(L);                                                       \
    lluv_error_create(L, LLUV_ERR_UV, req->req.result, req->req.path);    \
    argc = 2;                                                             \
  }                                                                       \
  else{                                                                   \
    if(req->req.fs_type == UV_FS_OPEN){                                   \
      lluv_file_create(L, loop, (uv_file)req->req.result, 0);             \
      argc = 1;                                                           \
    }                                                                     \
    else argc = 0;                                                        \
    argc += lluv_push_fs_result(L, req);                                  \
  }                                                                       \
                                                                          \
  uv_fs_req_cleanup(&req->req);                                           \
  lluv_fs_request_free(L, req);                                           \
                                                                          \
  return argc;                                                            \
}

#define LLUV_POST_FS()                                                    \
  LLUV_POST_FS_LOOP()                                                     \
  LLUV_POST_FS_COMMON()                                                   \

#define LLUV_POST_FILE()                                                  \
  LLUV_POST_FS_FILE()                                                     \
  LLUV_POST_FS_COMMON()                                                   \

#define LLUV_PRE_FILE() LLUV_PRE_FS()

#define lluv_arg_exists(L, idx) ((!lua_isnone(L, idx)) && (lua_type(L, idx) != LUA_TFUNCTION))

//}

//{ FS operations

LLUV_IMPL_SAFE(lluv_fs_unlink){
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_unlink(loop->handle, &req->req, path, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_mkdtemp) {
  LLUV_CHECK_LOOP_FS()

  const char *path = "./XXXXXX";
  if(lluv_arg_exists(L, argc + 1)){
    path = luaL_checkstring(L, ++argc);
  }

  LLUV_PRE_FS();
  err = uv_fs_mkdtemp(loop->handle, &req->req, path, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_mkdir) {
  LLUV_CHECK_LOOP_FS()

  const char *path   = luaL_checkstring(L, ++argc);
  int mode = 0;
  if(lluv_arg_exists(L, argc + 1)){
    mode = (int)luaL_checkinteger(L, ++argc);
  }

  LLUV_PRE_FS();
  err = uv_fs_mkdir(loop->handle, &req->req, path, mode, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_rmdir) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_rmdir(loop->handle, &req->req, path, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_scandir) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);
  int flags = 0;
  if(lluv_arg_exists(L, argc + 1)){
    flags = (int)luaL_checkinteger(L, ++argc);
  }

  LLUV_PRE_FS();
  err = uv_fs_scandir(loop->handle, &req->req, path, flags, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_stat) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_stat(loop->handle, &req->req, path, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_lstat) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_lstat(loop->handle, &req->req, path, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_rename) {
  LLUV_CHECK_LOOP_FS()

  const char *path     = luaL_checkstring(L, ++argc);
  const char *new_path = luaL_checkstring(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_rename(loop->handle, &req->req, path, new_path, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_chmod) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring (L, ++argc);
  int         mode = (int)luaL_checkinteger(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_chmod(loop->handle, &req->req, path, mode, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_utime) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);
  double     atime = luaL_checknumber(L, ++argc);
  double     mtime = luaL_checknumber(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_utime(loop->handle, &req->req, path, atime, mtime, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_symlink) {
  LLUV_CHECK_LOOP_FS()

  const char *path     = luaL_checkstring(L, ++argc);
  const char *new_path = luaL_checkstring(L, ++argc);
  int flags = 0;
  if(lluv_arg_exists(L, 3)){
    flags = (int)luaL_checkinteger(L, ++argc);
  }

  LLUV_PRE_FS();
  err = uv_fs_symlink(loop->handle, &req->req, path, new_path, flags, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_readlink) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_readlink(loop->handle, &req->req, path, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_chown) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);
  uv_uid_t     uid = (uv_uid_t)lutil_checkint64(L, ++argc);
  uv_gid_t     gid = (uv_gid_t)lutil_checkint64(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_chown(loop->handle, &req->req, path, uid, gid, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_access) {
  static const lluv_uv_const_t FLAGS[] = {
    { F_OK, "exists"   },
    { R_OK, "read"     },
    { W_OK, "write"    },
    { X_OK, "execute"  },

    { 0, NULL }
  };

  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);
  int flags = F_OK;
  if(lluv_arg_exists(L, argc + 1)){
    flags = lluv_opt_flags_ui_2(L, ++argc, flags, FLAGS);
  }

  LLUV_PRE_FS();
  err = uv_fs_access(loop->handle, &req->req, path, flags, cb);
  LLUV_POST_FS();
}

#if LLUV_UV_VER_GE(1,8,0)

LLUV_IMPL_SAFE(lluv_fs_realpath) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);

  LLUV_PRE_FS();
  err = uv_fs_realpath(loop->handle, &req->req, path, cb);
  LLUV_POST_FS();
}

#endif

#if LLUV_UV_VER_GE(1,14,0)

LLUV_IMPL_SAFE(lluv_fs_copyfile) {
  static const lluv_uv_const_t FLAGS[] = {
    { UV_FS_COPYFILE_EXCL,          "excl"        },
#if LLUV_UV_VER_GE(1,20,0)
    { UV_FS_COPYFILE_FICLONE,       "clone"       },
    { UV_FS_COPYFILE_FICLONE_FORCE, "clone_force" },
#endif

    { 0, NULL }
  };

  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);
  const char *dest = luaL_checkstring(L, ++argc);

  int flags = 0;
  if(lluv_arg_exists(L, argc + 1)){
    flags = lluv_opt_flags_ui_2(L, ++argc, flags, FLAGS);
  }

  LLUV_PRE_FS();
  err = uv_fs_copyfile(loop->handle, &req->req, path, dest, flags, cb);
  LLUV_POST_FS();
}

#endif

static int luv_check_open_flags(lua_State *L, int idx, const char *def){
  static const char *names[] = {
    "r"  ,
    "rs" ,
    "sr" ,
    "r+" ,
    "rs+",
    "sr+",
    "w"  ,
    "wx" ,
    "xw" ,
    "w+" ,
    "wx+",
    "xw+",
    "a"  ,
    "ax" ,
    "xa" ,
    "a+" ,
    "ax+",
    "xa+",
    NULL,
  };

  static const int flags[] = {
    O_RDONLY                               ,/*  r    */
    O_RDONLY | O_SYNC                      ,/*  rs   */
    O_RDONLY | O_SYNC                      ,/*  sr   */
    O_RDWR                                 ,/*  r+   */
    O_RDWR   | O_SYNC                      ,/*  rs+  */
    O_RDWR   | O_SYNC                      ,/*  sr+  */
    O_TRUNC  | O_CREAT | O_WRONLY          ,/*  w    */
    O_TRUNC  | O_CREAT | O_WRONLY | O_EXCL ,/*  wx   */
    O_TRUNC  | O_CREAT | O_WRONLY | O_EXCL ,/*  xw   */
    O_TRUNC  | O_CREAT | O_RDWR            ,/*  w+   */
    O_TRUNC  | O_CREAT | O_RDWR   | O_EXCL ,/*  wx+  */
    O_TRUNC  | O_CREAT | O_RDWR   | O_EXCL ,/*  xw+  */
    O_APPEND | O_CREAT | O_WRONLY          ,/*  a    */
    O_APPEND | O_CREAT | O_WRONLY | O_EXCL ,/*  ax   */
    O_APPEND | O_CREAT | O_WRONLY | O_EXCL ,/*  xa   */
    O_APPEND | O_CREAT | O_RDWR            ,/*  a+   */
    O_APPEND | O_CREAT | O_RDWR   | O_EXCL ,/*  ax+  */
    O_APPEND | O_CREAT | O_RDWR   | O_EXCL ,/*  xa+  */
  };

  //! @todo static assert before change names/flags
  
  int flag = luaL_checkoption(L, idx, def, names);

  return flags[flag];
}

LLUV_IMPL_SAFE(lluv_fs_open) {
  LLUV_CHECK_LOOP_FS()

  const char *path = luaL_checkstring(L, ++argc);
  int        flags = luv_check_open_flags(L, ++argc, NULL);
  int         mode = 0666;

  if(lluv_arg_exists(L, argc+1)){
    mode = (int)luaL_checkinteger(L, ++argc);
  }

  LLUV_PRE_FS();
  err = uv_fs_open(loop->handle, &req->req, path, flags, mode, cb);
  LLUV_POST_FS();
}

LLUV_IMPL_SAFE(lluv_fs_open_fd) {
  LLUV_CHECK_LOOP_FS()
  int64_t fd = lutil_checkint64(L, ++argc);
  int noclose = lua_toboolean(L, ++argc);

  if(!loop)loop = lluv_default_loop(L);
  lluv_file_create(L, loop, (uv_file)fd, noclose ? LLUV_FLAG_NOCLOSE : 0);
  return 1;
}

//}

//{ File object

#define LLUV_FILE_NAME LLUV_PREFIX" File"
static const char *LLUV_FILE = LLUV_FILE_NAME;

typedef struct lluv_file_tag{
  uv_file      handle;
  lluv_flags_t flags;
  lluv_loop_t  *loop;
}lluv_file_t;

static int lluv_file_create(lua_State *L, lluv_loop_t  *loop, uv_file h, unsigned char flags){
  lluv_file_t *f = lutil_newudatap(L, lluv_file_t, LLUV_FILE);
  f->handle = h;
  f->loop   = loop;
  f->flags  = flags | LLUV_FLAG_OPEN; 
  return 1;
}

static lluv_file_t *lluv_check_file(lua_State *L, int i, lluv_flags_t flags){
  lluv_file_t *f = (lluv_file_t *)lutil_checkudatap (L, i, LLUV_FILE);
  luaL_argcheck (L, f != NULL, i, LLUV_FILE_NAME" expected");

  /* loop could be closed already */
  if(!IS_(f->loop, OPEN)){
    if(IS_(f,OPEN)){
      lluv_fs_request_t *req = lluv_fs_request_new(L);
      UNSET_(f,OPEN);
      uv_fs_close(NULL, &req->req, f->handle, NULL);
      lluv_fs_request_free(L, req);
    }
  }

  luaL_argcheck (L, FLAGS_IS_SET(f->flags, flags), i, LLUV_FILE_NAME" closed");
  return f;
}

static int lluv_file_to_s(lua_State *L){
  lluv_file_t *f = lluv_check_file(L, 1, 0);
  lua_pushfstring(L, LLUV_FILE_NAME" (%p)", f);
  return 1;
}

static int lluv_file_close(lua_State *L){
  lluv_file_t *f = lluv_check_file(L, 1, 0);
  lluv_loop_t *loop = f->loop;

  if(IS_(f, OPEN)){
    const char  *path = NULL;
    int          argc = 1;
    UNSET_(f, OPEN);
    if(!IS_(f, NOCLOSE)){
      LLUV_PRE_FILE();
      err = uv_fs_close(loop->handle, &req->req, f->handle, cb);
      LLUV_POST_FILE();
    }
  }

  return 0;
}

static int lluv_file_loop(lua_State *L){
  lluv_file_t *f = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lua_rawgetp(L, LLUV_LUA_REGISTRY, f->loop->handle);
  return 1;
}

static int lluv_file_stat(lua_State *L){
  const char  *path = NULL;
  lluv_file_t *f    = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop = f->loop;
  int          argc = 1;

  LLUV_PRE_FILE();
  lua_pushvalue(L, 1);
  req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
  err = uv_fs_fstat(loop->handle, &req->req, f->handle, cb);
  LLUV_POST_FILE();
}

static int lluv_file_sync(lua_State *L){
  const char  *path = NULL;
  lluv_file_t *f    = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop = f->loop;
  int          argc = 1;

  LLUV_PRE_FILE();
  lua_pushvalue(L, 1);
  req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
  err = uv_fs_fsync(loop->handle, &req->req, f->handle, cb);
  LLUV_POST_FILE();
}

static int lluv_file_datasync(lua_State *L){
  const char  *path = NULL;
  lluv_file_t *f    = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop = f->loop;
  int          argc = 1;

  LLUV_PRE_FILE();
  lua_pushvalue(L, 1);
  req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
  err = uv_fs_fdatasync(loop->handle, &req->req, f->handle, cb);
  LLUV_POST_FILE();
}

static int lluv_file_truncate(lua_State *L){
  const char  *path = NULL;
  lluv_file_t *f    = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop = f->loop;
  int64_t      len  = lutil_checkint64(L, 2);
  int         argc  = 2;

  LLUV_PRE_FILE();
  lua_pushvalue(L, 1);
  req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
  err = uv_fs_ftruncate(loop->handle, &req->req, f->handle, len, cb);
  LLUV_POST_FILE();
}

static int lluv_file_chown(lua_State* L) {
  const char  *path = NULL;
  lluv_file_t *f    = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop = f->loop;
  uv_uid_t     uid  = (uv_uid_t)lutil_checkint64(L, 2);
  uv_gid_t     gid  = (uv_gid_t)lutil_checkint64(L, 3);
  int         argc  = 3;

  LLUV_PRE_FILE();
  lua_pushvalue(L, 1);
  req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
  err = uv_fs_fchown(loop->handle, &req->req, f->handle, uid, gid, cb);
  LLUV_POST_FILE();
}

static int lluv_file_chmod(lua_State* L) {
  const char  *path = NULL;
  lluv_file_t *f    = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop = f->loop;
  int         mode  = (int)luaL_checkinteger(L, 2);
  int         argc  = 2;

  LLUV_PRE_FILE();
  lua_pushvalue(L, 1);
  req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
  err = uv_fs_fchmod(loop->handle, &req->req, f->handle, mode, cb);
  LLUV_POST_FILE();
}

static int lluv_file_utime(lua_State* L) {
  const char  *path = NULL;
  lluv_file_t *f    = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop = f->loop;
  double     atime  = luaL_checknumber(L, 2);
  double     mtime  = luaL_checknumber(L, 3);
  int         argc  = 3;

  LLUV_PRE_FILE();
  lua_pushvalue(L, 1);
  req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
  err = uv_fs_futime(loop->handle, &req->req, f->handle, atime, mtime, cb);
  LLUV_POST_FILE();
}

static int lluv_file_readb(lua_State* L) {
  const char  *path = NULL;
  lluv_file_t *f    = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop = f->loop;

  char *base; size_t capacity;
  lluv_fixed_buffer_t *buffer;

  int64_t   position = 0; /* position in file default: 0*/ 
  int64_t   offset   = 0; /* offset in buffer default: 0*/
  size_t    length   = 0; /* number or bytes  default: buffer->capacity - offset*/

  int         argc = 2;

  if(lua_islightuserdata(L, argc)){
    base     = lua_touserdata(L, argc);
    capacity = (size_t)lutil_checkint64(L, ++argc);
  }
  else{
    buffer   = lluv_check_fbuf(L, argc);
    base     = buffer->data;
    capacity = buffer->capacity;
  }

  if(lluv_arg_exists(L, argc+1)){      /* position        */
    position = lutil_checkint64(L, ++argc);
  }
  if(lluv_arg_exists(L, argc+2)){      /* offset + length */
    offset = lutil_checkint64(L, ++argc);
    length = (size_t)lutil_checkint64(L, ++argc);
  }
  else if(lluv_arg_exists(L, argc+1)){ /* offset          */
    offset = lutil_checkint64(L, ++argc);
    length = capacity - offset;
  }
  else{
    length = capacity;
  }

  luaL_argcheck (L, capacity > (size_t)offset, 4, LLUV_PREFIX" offset out of index"); 
  luaL_argcheck (L, capacity >= ((size_t)offset + length), 5, LLUV_PREFIX" length out of index");

  LLUV_PRE_FILE();
  {
    uv_buf_t ubuf = lluv_buf_init(&base[offset], length);

    lua_pushvalue(L, 2);
    lua_rawsetp(L, LLUV_LUA_REGISTRY, &req->req);
    lua_pushvalue(L, 1);
    req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
    err = uv_fs_read(loop->handle, &req->req, f->handle, &ubuf, 1, position, cb);
  }
  LLUV_POST_FILE();
}

static int lluv_file_read(lua_State* L) {
  // if buffer_length provided then function allocate buffer with this size
  // read(buffer | buffer_length, [position, [ [offset,] [length,] ] ] [callback])

  if(lua_isnumber(L, 2)){
    int64_t len = lutil_checkint64(L, 2);
    lluv_fbuf_alloc(L, (size_t)len);
    lua_remove(L, 2); // replace length
    lua_insert(L, 2); // with buffer
  }
  return lluv_file_readb(L);
}

static int lluv_file_write(lua_State* L) {
  // if you provide string then function does not copy this string
  // write(buffer | string, [position, [ [offset,] [length,] ] ] [callback])

  const char  *path           = NULL;
  lluv_file_t *f              = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lluv_loop_t *loop           = f->loop;
  lluv_fixed_buffer_t *buffer = NULL;
  const char *str             = NULL;
  size_t    capacity;
  int64_t   position          = 0; /* position in file default: 0*/ 
  int64_t   offset            = 0; /* offset in buffer default: 0*/
  size_t    length            = 0; /* number or bytes  default: buffer->capacity - offset*/
  
  int         argc = 2;

  if(NULL == (str = lua_tolstring(L, 2, &capacity))){
    buffer   = lluv_check_fbuf(L, 2);
    capacity = buffer->capacity;
    str      = buffer->data;
  }

  if(lluv_arg_exists(L, 3)){      /* position        */
    position = lutil_checkint64(L, ++argc);
  }
  if(lluv_arg_exists(L, 5)){      /* offset + length */
    offset = lutil_checkint64(L, ++argc);
    length = (size_t)lutil_checkint64(L, ++argc);
  }
  else if(lluv_arg_exists(L, 4)){ /* offset          */
    offset = lutil_checkint64(L, ++argc);
    length = capacity - offset;
  }
  else{
    length = capacity;
  }

  luaL_argcheck (L, capacity > (size_t)offset, 4, LLUV_PREFIX" offset out of index"); 
  luaL_argcheck (L, capacity >= ((size_t)offset + length), 5, LLUV_PREFIX" length out of index");

  LLUV_PRE_FILE();
  {
    uv_buf_t ubuf = lluv_buf_init((char*)&str[offset], length);
    
    lua_pushvalue(L, 2); /*string or buffer*/
    lua_rawsetp(L, LLUV_LUA_REGISTRY, &req->req);
    lua_pushvalue(L, 1);
    req->file_ref = luaL_ref(L, LLUV_LUA_REGISTRY);
    err = uv_fs_write(loop->handle, &req->req, f->handle, &ubuf, 1, position, cb);
  }
  LLUV_POST_FILE();
}

static int lluv_file_fileno(lua_State *L){
  lluv_file_t *f = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  lutil_pushint64(L, f->handle);
  return 1;
}

static int lluv_file_pipe(lua_State *L){
  lluv_file_t *f = lluv_check_file(L, 1, LLUV_FLAG_OPEN);
  int ipc = lua_toboolean(L, 2);

  /*local ok, err = uv.pipe(loop, ipc)*/
  lua_pushvalue(L, LLUV_LUA_REGISTRY);
  lua_pushvalue(L, LLUV_LUA_HANDLES);
  lua_pushcclosure(L, IS_(f, RAISE_ERROR) ? lluv_pipe_create_unsafe : lluv_pipe_create_safe, 2);
  lluv_loop_pushself(L, f->loop);
  lua_pushboolean(L, ipc);
  lua_call(L, 2, 2);

  /*if not ok then return nil, err*/
  if(lua_isnil(L, -2)) return 2;
  lua_pop(L, 1);

  /*local ok, err = pipe:open(fd)*/
  lua_getfield(L, -1, "open");
  assert(lua_isfunction(L, -1));
  lua_pushvalue(L, -2);
  lutil_pushint64(L, f->handle);
  lua_call(L, 2, 2);

  /*if not ok then pipe:close() return nil, err*/
  if(lua_isnil(L, -2)){
    int top = lua_gettop(L);
    lua_getfield(L, -3, "close");
    assert(lua_isfunction(L, -1));
    lua_pushvalue(L, -4);
    lua_pcall(L, 0, 0, 0);
    lua_settop(L, top);
    return 2;
  }
  lua_pop(L, 2);

  UNSET_(f, OPEN);

  return 1;
}

static const struct luaL_Reg lluv_file_methods[] = {
  {"loop",         lluv_file_loop      },
  {"stat",         lluv_file_stat      },
  {"sync",         lluv_file_sync      },
  {"datasync",     lluv_file_datasync  },
  {"truncate",     lluv_file_truncate  },
  {"close",        lluv_file_close     },
  {"chown",        lluv_file_chown     },
  {"chmod",        lluv_file_chmod     },
  {"utime",        lluv_file_utime     },
  {"fd",           lluv_file_fileno    },
  {"pipe",         lluv_file_pipe      },

  {"read",         lluv_file_read      },
  {"write",        lluv_file_write     },
  {"__gc",         lluv_file_close     },
  {"__tostring",   lluv_file_to_s      },
  
  {NULL,NULL}
};

//}

enum {
  LLUV_FS_FUNCTIONS_DUMMY = 16,
  #if LLUV_UV_VER_GE(1,8,0)
  LLUV_FS_FUNCTIONS_DUMMY_1,
  #endif
  #if LLUV_UV_VER_GE(1,14,0)
  LLUV_FS_FUNCTIONS_DUMMY_2,
  #endif
  LLUV_FS_FUNCTIONS_COUNT
};

#define LLUV_FS_FUNCTIONS(F)                \
  { "fs_unlink",   lluv_fs_unlink_##F   },  \
  { "fs_mkdtemp",  lluv_fs_mkdtemp_##F  },  \
  { "fs_mkdir",    lluv_fs_mkdir_##F    },  \
  { "fs_rmdir",    lluv_fs_rmdir_##F    },  \
  { "fs_scandir",  lluv_fs_scandir_##F  },  \
  { "fs_stat",     lluv_fs_stat_##F     },  \
  { "fs_lstat",    lluv_fs_lstat_##F    },  \
  { "fs_rename",   lluv_fs_rename_##F   },  \
  { "fs_chmod",    lluv_fs_chmod_##F    },  \
  { "fs_utime",    lluv_fs_utime_##F    },  \
  { "fs_symlink",  lluv_fs_symlink_##F  },  \
  { "fs_readlink", lluv_fs_readlink_##F },  \
  { "fs_chown",    lluv_fs_chown_##F    },  \
  { "fs_access",   lluv_fs_access_##F   },  \
                                            \
  { "fs_open",     lluv_fs_open_##F     },  \
  { "fs_open_fd",  lluv_fs_open_fd_##F  },  \

#define LLUV_FS_FUNCTIONS_1_8_0(F)          \
  { "fs_realpath", lluv_fs_realpath_##F },  \

#define LLUV_FS_FUNCTIONS_1_14_0(F)         \
  { "fs_copyfile", lluv_fs_copyfile_##F },  \

static const struct luaL_Reg lluv_fs_functions[][LLUV_FS_FUNCTIONS_COUNT] = {
  {
    LLUV_FS_FUNCTIONS(unsafe)
#if LLUV_UV_VER_GE(1,8,0)
    LLUV_FS_FUNCTIONS_1_8_0(unsafe)
#endif
#if LLUV_UV_VER_GE(1,14,0)
    LLUV_FS_FUNCTIONS_1_14_0(unsafe)
#endif
    {NULL,NULL}
  },
  {
    LLUV_FS_FUNCTIONS(safe)
#if LLUV_UV_VER_GE(1,8,0)
    LLUV_FS_FUNCTIONS_1_8_0(safe)
#endif
#if LLUV_UV_VER_GE(1,14,0)
    LLUV_FS_FUNCTIONS_1_14_0(safe)
#endif
    {NULL,NULL}
  },
};

static const lluv_uv_const_t lluv_fs_constants[] = {
  { F_OK, "F_OK"  },
  { R_OK, "R_OK"  },
  { W_OK, "W_OK"  },
  { X_OK, "X_OK"  },

#if LLUV_UV_VER_GE(1,14,0)
  { UV_FS_COPYFILE_EXCL,          "FS_COPYFILE_EXCL"          },
#endif

#if LLUV_UV_VER_GE(1,20,0)
  { UV_FS_COPYFILE_FICLONE,       "FS_COPYFILE_FICLONE"       },
  { UV_FS_COPYFILE_FICLONE_FORCE, "FS_COPYFILE_FICLONE_FORCE" },
#endif

  { 0, NULL }
};

LLUV_INTERNAL void lluv_fs_initlib(lua_State *L, int nup, int safe){
  assert((safe == 0) || (safe == 1));

  lutil_pushnvalues(L, nup);

  if(!lutil_createmetap(L, LLUV_FILE, lluv_file_methods, nup))
    lua_pop(L, nup);
  lua_pop(L, 1);

  luaL_setfuncs(L, lluv_fs_functions[safe], nup);
  lluv_register_constants(L, lluv_fs_constants);
}
