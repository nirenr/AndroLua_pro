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
#include "lluv_loop.h"
#include "lluv_misc.h"
#include "lluv_error.h"
#include <assert.h>

static void lluv_push_rusage(lua_State *L, const uv_rusage_t* s){
#define SET_FIELD_INT(F,V)  lutil_pushint64(L, s->V);         lua_setfield(L, -2, F)
#define SET_FIELD_TIME(F,V) lluv_push_timeval(L, &s->V);      lua_setfield(L, -2, F)

  lua_newtable(L);

  SET_FIELD_INT  ( "maxrss"  , ru_maxrss    );
  SET_FIELD_INT  ( "ixrss"   , ru_ixrss     );
  SET_FIELD_INT  ( "idrss"   , ru_idrss     );
  SET_FIELD_INT  ( "isrss"   , ru_isrss     );
  SET_FIELD_INT  ( "minflt"  , ru_minflt    );
  SET_FIELD_INT  ( "majflt"  , ru_majflt    );
  SET_FIELD_INT  ( "nswap"   , ru_nswap     );
  SET_FIELD_INT  ( "inblock" , ru_inblock   );
  SET_FIELD_INT  ( "oublock" , ru_oublock   );
  SET_FIELD_INT  ( "msgsnd"  , ru_msgsnd    );
  SET_FIELD_INT  ( "msgrcv"  , ru_msgrcv    );
  SET_FIELD_INT  ( "nsignals", ru_nsignals  );
  SET_FIELD_INT  ( "nvcsw"   , ru_nvcsw     );
  SET_FIELD_INT  ( "nivcsw"  , ru_nivcsw    );

  SET_FIELD_TIME ( "utime"   , ru_utime     );
  SET_FIELD_TIME ( "stime"   , ru_stime     );

#undef SET_FIELD_INT
#undef SET_FIELD_TIME
}

static void lluv_push_cpu_info(lua_State *L, const uv_cpu_info_t* s){
#define SET_FIELD_STR(F,V)  lua_pushstring(L, s->V);          lua_setfield(L, -2, F)
#define SET_FIELD_INT(F,V)  lutil_pushint64(L, s->V);         lua_setfield(L, -2, F)

  lua_newtable(L);
    SET_FIELD_STR  ( "model"   , model    );
    SET_FIELD_INT  ( "speed"   , speed     );
    lua_newtable(L);
      SET_FIELD_INT  ( "user"    , cpu_times.user     );
      SET_FIELD_INT  ( "nice"    , cpu_times.nice     );
      SET_FIELD_INT  ( "sys"     , cpu_times.sys      );
      SET_FIELD_INT  ( "idle"    , cpu_times.idle     );
      SET_FIELD_INT  ( "irq"     , cpu_times.irq      );
    lua_setfield(L, -2, "times");

#undef SET_FIELD_STR
#undef SET_FIELD_INT
}

static int lluv_push_if_addr(lua_State *L, const struct sockaddr* addr){
  char buf[INET6_ADDRSTRLEN + 1];

  switch (addr->sa_family){
    case AF_INET:{
      struct sockaddr_in *sa = (struct sockaddr_in*)addr;
      uv_ip4_name(sa, buf, sizeof(buf));
      lua_pushstring(L, buf);
      return 1;
                 }

    case AF_INET6:{
      struct sockaddr_in6 *sa = (struct sockaddr_in6*)addr;
      uv_ip6_name(sa, buf, sizeof(buf));
      lua_pushstring(L, buf);
      return 1;
                  }
  }

  lua_pushnil(L);
  return 1;
}

static int lluv_push_phys_addr(lua_State *L, const unsigned char* a){
  char buf[32] = {'\0'};
  sprintf(buf, "%.2X:%.2X:%.2X:%.2X:%.2X:%.2X", a[0],a[1],a[2],a[3],a[4],a[5]);
  lua_pushstring(L, buf);
  return 1;
}

static int lluv_push_if_family(lua_State *L, const struct sockaddr* addr){
  switch (addr->sa_family){
    case AF_INET:{
      lua_pushstring(L, "inet");
      return 1;
    }

    case AF_INET6:{
      lua_pushstring(L, "inet6");
      return 1;
    }
  }

  lua_pushnil(L);
  return 1;
}

static void lluv_push_interface(lua_State *L, uv_interface_address_t* s){
#define SET_FIELD_STR(F,V)  lua_pushstring(L, s->V);          lua_setfield(L, -2, F)
#define SET_FIELD_INT(F,V)  lutil_pushint64(L, s->V);         lua_setfield(L, -2, F)

  lua_newtable(L);
    SET_FIELD_STR  ( "name"      , name       );
    lluv_push_phys_addr(L, s->phys_addr);                           lua_setfield(L, -2, "phys_addr");
    lluv_push_if_addr(L,   (struct sockaddr*)&s->address.address4); lua_setfield(L, -2, "address");
    lluv_push_if_addr(L,   (struct sockaddr*)&s->netmask.netmask4); lua_setfield(L, -2, "netmask");
    lluv_push_if_family(L, (struct sockaddr*)&s->address.address4); lua_setfield(L, -2, "family");
    lua_pushboolean(L, s->is_internal); lua_setfield(L, -2, "internal");

#undef SET_FIELD_STR
#undef SET_FIELD_INT
}

static int lluv_version(lua_State *L){
  int unpack = lua_toboolean(L, 1);
  if(!unpack){
    lua_pushstring(L, uv_version_string());
    return 1;
  }
  else{
    unsigned int ver = uv_version();
    unsigned int
      min = 0xFF & (ver >> 16),
      maj = 0xFF & (ver >> 8),
      pat = 0xFF & ver;
    lua_pushinteger(L, min);
    lua_pushinteger(L, maj);
    lua_pushinteger(L, pat);
    return 3;
  }
}

static int lluv_get_process_title(lua_State *L){
  char buf[255];
  int err = uv_get_process_title(buf, sizeof(buf) - 1);
  if(err < 0){
    lua_pushstring(L, "");
  }
  else{
    buf[ sizeof(buf) - 1 ] = '\0';
    lua_pushstring(L, buf);
  }
  return 1;
}

static int lluv_set_process_title(lua_State *L){
  const char *title = luaL_checkstring(L, 1);
  lua_pushinteger(L, uv_set_process_title(title));
  return 1;
}

static int lluv_resident_set_memory(lua_State *L){
  size_t rss;
  int err = uv_resident_set_memory(&rss);
  lutil_pushint64(L, rss);
  return 1;
}

static int lluv_uptime(lua_State *L){
  double uptime;
  int err = uv_uptime(&uptime);
  lua_pushnumber(L, uptime);
  return 1;
}

static int lluv_getrusage(lua_State *L){
  uv_rusage_t ru;
  int err = uv_getrusage(&ru);
  lluv_push_rusage(L, &ru);
  return 1;
}

static int lluv_cpu_info(lua_State *L){
  uv_cpu_info_t* cpu_infos; int count, i;
  int err = uv_cpu_info(&cpu_infos, &count);
  lua_newtable(L);
  for(i = 0; i < count; ++i){
    lluv_push_cpu_info(L, &cpu_infos[i]);
    lua_rawseti(L, -2, i+1);
  }
  uv_free_cpu_info(cpu_infos, count);
  return 1;
}

static int lluv_interface_addresses(lua_State *L){
  uv_interface_address_t* addresses; int count, i;
  int err = uv_interface_addresses(&addresses, &count);
  lua_newtable(L);
  for(i = 0; i < count; ++i){
    lluv_push_interface(L, &addresses[i]);
    lua_rawseti(L, -2, i+1);
  }
  uv_free_interface_addresses(addresses, count);
  return 1;
}

#define MAX_PATH_LEN 4096

#define IS_PATH_SEP(s) (((s)=='\\')||((s)=='/'))

static int lluv_exepath(lua_State *L){
  char *buf = lluv_alloc(L, MAX_PATH_LEN); size_t len = MAX_PATH_LEN;
  int err = uv_exepath(buf, &len);
  if(err < 0){
    lua_pushstring(L, "");
  }
  else{
    lua_pushlstring(L, buf, len);
  }
  lluv_free(L, buf);
  return 1;
}

static int lluv_cwd(lua_State *L){
  char *buf = lluv_alloc(L, MAX_PATH_LEN); size_t len = MAX_PATH_LEN;
  int err = uv_cwd(buf, &len);
  if(err < 0){
    lua_pushstring(L, "");
  }
  else{
    if(len && IS_PATH_SEP(buf[len])) --len;
    lua_pushlstring(L, buf, len);
  }
  lluv_free(L, buf);
  return 1;
}

static int lluv_chdir(lua_State *L){
  const char *d = luaL_checkstring(L, 1);
  int err = uv_chdir(d);
  return 1;
}

static int lluv_get_total_memory(lua_State *L){
  lutil_pushint64(L, uv_get_total_memory());
  return 1;
}

static int lluv_get_free_memory(lua_State *L){
  lutil_pushint64(L, uv_get_free_memory());
  return 1;
}

static int lluv_hrtime(lua_State *L){
  lutil_pushint64(L, uv_hrtime());
  return 1;
}

#if LLUV_UV_VER_GE(1,6,0)

LLUV_IMPL_SAFE(lluv_os_homedir){
  char temp[255];
  size_t size = sizeof(temp);
  char *buf = &temp[0];

  int err = uv_os_homedir(buf, &size);

  if (err == UV_ENOBUFS) {
    buf = lluv_alloc(L, size);
    if(buf){
      err = uv_os_homedir(buf, &size);
    }
  }

  if(err < 0){
    if(buf && buf != &temp[0]){
      lluv_free(L, buf);
    }
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, err, NULL);
  }

  lua_pushlstring(L, buf, size);

  if(buf && buf != &temp[0]){
    lluv_free(L, buf);
  }

  return 1;
}

#endif

#if LLUV_UV_VER_GE(1,9,0)

LLUV_IMPL_SAFE(lluv_os_get_passwd){
  uv_passwd_t pwd;
  int err = uv_os_get_passwd(&pwd);

  if(err < 0){
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, err, NULL);
  }

  lua_newtable(L);
  lutil_pushint64(L, pwd.uid); lua_setfield(L, -2, "uid");
  lutil_pushint64(L, pwd.gid); lua_setfield(L, -2, "gid");
  if(pwd.username){ lua_pushstring(L, pwd.username); lua_setfield(L, -2, "username"); }
  if(pwd.shell   ){ lua_pushstring(L, pwd.shell   ); lua_setfield(L, -2, "shell"   ); }
  if(pwd.homedir ){ lua_pushstring(L, pwd.homedir ); lua_setfield(L, -2, "homedir" ); }
  uv_os_free_passwd(&pwd);

  return 1;
}

#endif

#if LLUV_UV_VER_GE(1,12,0)

LLUV_IMPL_SAFE(lluv_os_gethostname){
  char temp[255];
  size_t size = sizeof(temp);
  char *buf = &temp[0];

  int err = uv_os_gethostname(buf, &size);

  if (err == UV_ENOBUFS) {
    buf = lluv_alloc(L, size);
    if(buf){
      err = uv_os_gethostname(buf, &size);
    }
  }

  if(err < 0){
    if(buf && buf != &temp[0]){
      lluv_free(L, buf);
    }
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, err, NULL);
  }

  lua_pushlstring(L, buf, size);

  if(buf && buf != &temp[0]){
    lluv_free(L, buf);
  }

  return 1;
}

// UV_EXTERN int uv_os_getenv(const char* name, char* buffer, size_t* size);
// UV_EXTERN int uv_os_setenv(const char* name, const char* value);
// UV_EXTERN int uv_os_unsetenv(const char* name);

LLUV_IMPL_SAFE(lluv_os_getenv){
  char temp[255];
  size_t size = sizeof(temp);
  char *buf = &temp[0];
  const char *name = luaL_checkstring(L, 1);

  int err = uv_os_getenv(name, buf, &size);

  if (err == UV_ENOBUFS) {
    buf = lluv_alloc(L, size);
    if(buf){
      err = uv_os_getenv(name, buf, &size);
    }
  }

  if(err < 0){
    if(buf && buf != &temp[0]){
      lluv_free(L, buf);
    }
    if(err == UV_ENOENT){
      lua_pushnil(L);
      return 1;
    }
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, err, NULL);
  }

  lua_pushlstring(L, buf, size);

  if(buf && buf != &temp[0]){
    lluv_free(L, buf);
  }

  return 1;
}

LLUV_IMPL_SAFE(lluv_os_setenv){
  const char *name  = luaL_checkstring(L, 1);
  const char *value = luaL_checkstring(L, 2);

  int err = uv_os_setenv(name, value);

  if(err < 0){
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, err, NULL);
  }

  lua_pushboolean(L, 1);

  return 1;
}

LLUV_IMPL_SAFE(lluv_os_unsetenv){
  const char *name  = luaL_checkstring(L, 1);

  int err = uv_os_unsetenv(name);

  if(err < 0){
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, err, NULL);
  }

  lua_pushboolean(L, 1);

  return 1;
}

#endif

#if LLUV_UV_VER_GE(1,16,0)

LLUV_IMPL_SAFE(lluv_os_getppid){
  uv_pid_t ppid = uv_os_getppid();
  lutil_pushint64(L, ppid);
  return 1;
}

LLUV_IMPL_SAFE(lluv_if_indextoname){
  int idx = luaL_checkinteger(L, 1);

  char temp[255];
  size_t size = sizeof(temp);
  char *buf = &temp[0];

  int err = uv_if_indextoname(idx, buf, &size);

  if (err == UV_ENOBUFS && size != 0) {
    buf = lluv_alloc(L, size);
    if(buf){
      err = uv_if_indextoname(idx, buf, &size);
    }
  }

  if(err < 0){
    if(buf && buf != &temp[0]){
      lluv_free(L, buf);
    }
    if(err == UV_ENOENT){
      lua_pushnil(L);
      return 1;
    }
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, err, NULL);
  }

  lua_pushlstring(L, buf, size);

  if(buf && buf != &temp[0]){
    lluv_free(L, buf);
  }

  return 1;
}

LLUV_IMPL_SAFE(lluv_if_indextoiid){
  int idx = luaL_checkinteger(L, 1);

  char temp[255];
  size_t size = sizeof(temp);
  char *buf = &temp[0];

  int err = uv_if_indextoiid(idx, buf, &size);

  if (err == UV_ENOBUFS && size != 0) {
    buf = lluv_alloc(L, size);
    if(buf){
      err = uv_if_indextoiid(idx, buf, &size);
    }
  }

  if(err < 0){
    if(buf && buf != &temp[0]){
      lluv_free(L, buf);
    }
    if(err == UV_ENOENT){
      lua_pushnil(L);
      return 1;
    }
    return lluv_fail(L, safe_flag, LLUV_ERR_UV, err, NULL);
  }

  lua_pushlstring(L, buf, size);

  if(buf && buf != &temp[0]){
    lluv_free(L, buf);
  }

  return 1;
}
#endif

#if LLUV_UV_VER_GE(1,18,0)

LLUV_IMPL_SAFE(lluv_os_getpid){
  uv_pid_t ppid = uv_os_getpid();
  lutil_pushint64(L, ppid);
  return 1;
}

#endif

static const lluv_uv_const_t lluv_misc_constants[] = {
  { 0, NULL }
};

enum {
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY = 15,
  #if LLUV_UV_VER_GE(1,9,0)
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_9_0_1,
  #endif
  #if LLUV_UV_VER_GE(1,6,0)
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_6_0_1,
  #endif
  #if LLUV_UV_VER_GE(1,12,0)
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_12_0_1,
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_12_0_2,
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_12_0_3,
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_12_0_4,
  #endif
  #if LLUV_UV_VER_GE(1,16,0)
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_16_0_1,
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_16_0_2,
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_16_0_3,
  #endif
  #if LLUV_UV_VER_GE(1,18,0)
  LLUV_MISC_FUNCTIONS_COUNT_DUMMY_1_18_0_1,
  #endif
  LLUV_MISC_FUNCTIONS_COUNT
};

#define LLUV_MISC_FUNCTIONS(F)                         \
  { "version",             lluv_version             }, \
  { "get_process_title",   lluv_get_process_title   }, \
  { "set_process_title",   lluv_set_process_title   }, \
  { "resident_set_memory", lluv_resident_set_memory }, \
  { "uptime",              lluv_uptime              }, \
  { "getrusage",           lluv_getrusage           }, \
  { "cpu_info",            lluv_cpu_info            }, \
  { "interface_addresses", lluv_interface_addresses }, \
  { "exepath",             lluv_exepath             }, \
  { "cwd",                 lluv_cwd                 }, \
  { "chdir",               lluv_chdir               }, \
  { "get_total_memory",    lluv_get_total_memory    }, \
  { "get_free_memory",     lluv_get_free_memory     }, \
  { "hrtime",              lluv_hrtime              }, \

#define LLUV_MISC_FUNCTIONS_1_6_0(F)                   \
  { "os_homedir",          lluv_os_homedir_##F      }, \

#define LLUV_MISC_FUNCTIONS_1_9_0(F)                   \
  { "os_get_passwd",       lluv_os_get_passwd_##F   }, \

#define LLUV_MISC_FUNCTIONS_1_12_0(F)                  \
  { "os_gethostname",      lluv_os_gethostname_##F  }, \
  { "os_getenv",           lluv_os_getenv_##F       }, \
  { "os_setenv",           lluv_os_setenv_##F       }, \
  { "os_unsetenv",         lluv_os_unsetenv_##F     }, \

#define LLUV_MISC_FUNCTIONS_1_16_0(F)                  \
  { "os_getppid",          lluv_os_getppid_##F      }, \
  { "if_indextoname",      lluv_if_indextoname_##F  }, \
  { "if_indextoiid",       lluv_if_indextoiid_##F   }, \

#define LLUV_MISC_FUNCTIONS_1_18_0(F)                  \
  { "os_getpid",           lluv_os_getpid_##F       }, \

static const struct luaL_Reg lluv_misc_functions[][LLUV_MISC_FUNCTIONS_COUNT] = {
  {
    LLUV_MISC_FUNCTIONS(unsafe)
#if LLUV_UV_VER_GE(1,6,0)
    LLUV_MISC_FUNCTIONS_1_6_0(unsafe)
#endif
#if LLUV_UV_VER_GE(1,9,0)
    LLUV_MISC_FUNCTIONS_1_9_0(unsafe)
#endif
#if LLUV_UV_VER_GE(1,12,0)
    LLUV_MISC_FUNCTIONS_1_12_0(unsafe)
#endif
#if LLUV_UV_VER_GE(1,16,0)
    LLUV_MISC_FUNCTIONS_1_16_0(unsafe)
#endif
#if LLUV_UV_VER_GE(1,18,0)
    LLUV_MISC_FUNCTIONS_1_18_0(unsafe)
#endif
    {NULL,NULL}
  },
  {
    LLUV_MISC_FUNCTIONS(safe)
#if LLUV_UV_VER_GE(1,6,0)
    LLUV_MISC_FUNCTIONS_1_6_0(safe)
#endif
#if LLUV_UV_VER_GE(1,9,0)
    LLUV_MISC_FUNCTIONS_1_9_0(safe)
#endif
#if LLUV_UV_VER_GE(1,12,0)
    LLUV_MISC_FUNCTIONS_1_12_0(safe)
#endif
#if LLUV_UV_VER_GE(1,16,0)
    LLUV_MISC_FUNCTIONS_1_16_0(safe)
#endif
    {NULL,NULL}
  },
};

LLUV_INTERNAL void lluv_misc_initlib(lua_State *L, int nup, int safe){
  assert((safe == 0) || (safe == 1));

  luaL_setfuncs(L, lluv_misc_functions[safe], nup);

  lluv_register_constants(L, lluv_misc_constants);
}
