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
#include "lluv_dns.h"
#include "lluv_loop.h"
#include "lluv_error.h"
#include "lluv_req.h"
#include <memory.h>
#include <assert.h>


#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif

#ifndef AI_V4MAPPED
#define AI_V4MAPPED 0
#endif

#ifndef AI_ALL
#define AI_ALL 0
#endif

#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST 0
#endif

#ifndef AI_PASSIVE
#define AI_PASSIVE 0
#endif

#ifndef AI_NUMERICSERV
#define AI_NUMERICSERV 0
#endif

#ifndef AI_CANONNAME
#define AI_CANONNAME 0
#endif

//! @todo extend/check list
#define LLUV_AI_FLAG_MAP(XX)                             \
  XX( AI_ADDRCONFIG,   "AI_ADDRCONFIG",  "addrconfig"  ) \
  XX( AI_V4MAPPED,     "AI_V4MAPPED",    "v4mapped"    ) \
  XX( AI_ALL,          "AI_ALL",         "all"         ) \
  XX( AI_NUMERICHOST,  "AI_NUMERICHOST", "numerichost" ) \
  XX( AI_PASSIVE,      "AI_PASSIVE",     "passive"     ) \
  XX( AI_NUMERICSERV,  "AI_NUMERICSERV", "numericserv" ) \
  XX( AI_CANONNAME,    "AI_CANONNAME",   "canonname"   ) \

//! @todo extend list
#define LLUV_AI_FAMILY_MAP(XX)                           \
  XX( AF_UNSPEC,       "AF_UNSPEC",      "unspec"      ) \
  XX( AF_INET,         "AF_INET",        "inet"        ) \
  XX( AF_INET6,        "AF_INET6",       "inet6"       ) \
  XX( AF_UNIX,         "AF_UNIX",        "unix"        ) \

//! @todo extend list
#define LLUV_AI_STYPE_MAP(XX)                            \
  XX( SOCK_STREAM,     "SOCK_STREAM",    "stream"      ) \
  XX( SOCK_DGRAM,      "SOCK_DGRAM",     "dgram"       ) \
  XX( SOCK_RAW,        "SOCK_RAW",       "raw"         ) \

//! @todo extend list
#define LLUV_AI_PROTO_MAP(XX)                            \
  XX( IPPROTO_TCP,     "IPPROTO_TCP",    "tcp"         ) \
  XX( IPPROTO_UDP,     "IPPROTO_UDP",    "udp"         ) \
  XX( IPPROTO_ICMP,    "IPPROTO_ICMP",   "icmp"        ) \


#define XX(C,X,N) if(code == C){lua_pushliteral(L, N); return;}

static void lluv_push_ai_family(lua_State *L, int code){
  LLUV_AI_FAMILY_MAP(XX)
  lua_pushnumber(L, code);
}

static void lluv_push_ai_stype(lua_State *L, int code){
  LLUV_AI_STYPE_MAP(XX)
  lua_pushnumber(L, code);
}

static void lluv_push_ai_proto(lua_State *L, int code){
  LLUV_AI_PROTO_MAP(XX)
  lua_pushnumber(L, code);
}

#undef XX

static void lluv_on_getnameinfo(uv_getnameinfo_t* arg, int status, const char* hostname, const char* service){
  lluv_req_t  *req  = lluv_req_byptr((uv_req_t*)arg);
  lluv_loop_t *loop = lluv_loop_byptr(arg->loop);
  lua_State   *L    = loop->L;

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  if(!IS_(loop, OPEN)){
    lluv_req_free(L, req);
    return;
  }

  lua_rawgeti(L, LLUV_LUA_REGISTRY, req->cb);
  assert(!lua_isnil(L, -1));

  lluv_loop_pushself(L, loop);
  lluv_push_status(L, status);
  if(hostname)lua_pushstring(L, hostname); else lua_pushnil(L);
  if(service) lua_pushstring(L, service);  else lua_pushnil(L);

  lluv_req_free(L, req);

  LLUV_LOOP_CALL_CB(L, loop, 4);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

static void lluv_push_addrinfo(lua_State *L, struct addrinfo* res){
  struct addrinfo* a = res;
  int i = 0;

  lua_newtable(L);

  for(a = res; a; a = a->ai_next){
    char buf[INET6_ADDRSTRLEN + 1];
    int port;
    lua_newtable(L);

    switch (a->ai_family){
      case AF_INET:{
        struct sockaddr_in *sa = (struct sockaddr_in*)a->ai_addr;
        uv_ip4_name(sa, buf, sizeof(buf));
        lua_pushstring(L, buf);
        lua_setfield(L, -2, "address");
        if((port = ntohs(sa->sin_port))){
          lua_pushinteger(L, port);
          lua_setfield(L, -2, "port");
        }
        break;
      }

      case AF_INET6:{
        struct sockaddr_in6 *sa = (struct sockaddr_in6*)a->ai_addr;
        uv_ip6_name(sa, buf, sizeof(buf));
        lua_pushstring(L, buf);
        lua_setfield(L, -2, "address");
        if((port = ntohs(sa->sin6_port))){
          lua_pushinteger(L, port);
          lua_setfield(L, -2, "port");
        }
        break;
      }
    }

    if(a->ai_canonname){
      lua_pushstring(L, a->ai_canonname);
      lua_setfield(L, -2, "canonname");
    }

    lluv_push_ai_family(L, a->ai_family);
    lua_setfield(L, -2, "family");

    lluv_push_ai_stype(L, a->ai_socktype);
    lua_setfield(L, -2, "socktype");

    lluv_push_ai_proto(L, a->ai_protocol);
    lua_setfield(L, -2, "protocol");

    lua_rawseti(L, -2, ++i);
  }
}

static void lluv_on_getaddrinfo(uv_getaddrinfo_t* arg, int status, struct addrinfo* res){
  lluv_req_t  *req   = lluv_req_byptr((uv_req_t*)arg);
  lluv_loop_t *loop  = lluv_loop_byptr(arg->loop);
  lua_State   *L     = loop->L;

  LLUV_CHECK_LOOP_CB_INVARIANT(L);

  lua_rawgeti(L, LLUV_LUA_REGISTRY, req->cb);
  lluv_req_free(L, req);
  assert(!lua_isnil(L, -1));

  lluv_loop_pushself(L, loop);

  if(status < 0){
    uv_freeaddrinfo(res);
    lluv_error_create(L, LLUV_ERR_UV, (uv_errno_t)status, NULL);
    LLUV_LOOP_CALL_CB(L, loop, 2);
    LLUV_CHECK_LOOP_CB_INVARIANT(L);
    return;
  }

  lua_pushnil(L);

  lluv_push_addrinfo(L, res);

  uv_freeaddrinfo(res);
  LLUV_LOOP_CALL_CB(L, loop, 3);

  LLUV_CHECK_LOOP_CB_INVARIANT(L);
}

LLUV_IMPL_SAFE(lluv_getaddrinfo){
#define XX(C, L, N) {C, N},

  static const lluv_uv_const_t ai_family[] = {
    LLUV_AI_FAMILY_MAP(XX)

    { 0, NULL }
  };

  static const lluv_uv_const_t ai_stype[] = {
    LLUV_AI_STYPE_MAP(XX)

    { 0, NULL }
  };

  static const lluv_uv_const_t ai_proto[] = {
    LLUV_AI_PROTO_MAP(XX)

    { 0, NULL }
  };

  static const lluv_uv_const_t FLAGS[] = {
    LLUV_AI_FLAG_MAP(XX)

    { 0, NULL }
  };

#undef XX

  lluv_loop_t *loop = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  int argc = loop ? 1 : 0;
  int hi = 0;
  if(!loop)loop = lluv_default_loop(L);
  {
    const char *node;
    const char *service = NULL;
    lluv_req_t *req; int err;
    int no_callback = 0;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    node = luaL_optstring(L, argc + 1, NULL);

    if(!lua_isfunction(L, argc + 2)){
      if(lua_istable(L, argc + 2)) hi = argc + 2;
      else service = luaL_optstring(L, argc + 2, NULL);
    }

    luaL_argcheck(L, node || service, argc + 1, "you must specify node or service");
    
    if(!hi && lua_istable(L, argc + 3)) hi = argc + 3;

    if(hi){
      lua_getfield(L, hi, "family");
      hints.ai_family = lluv_opt_named_const(L, -1, 0, ai_family);
      lua_pop(L, 1);

      lua_getfield(L, hi, "socktype");
      hints.ai_socktype = lluv_opt_named_const(L, -1, 0, ai_stype);
      lua_pop(L, 1);

      lua_getfield(L, hi, "protocol");
      hints.ai_protocol = lluv_opt_named_const(L, -1, 0, ai_proto);
      lua_pop(L, 1);

      lua_getfield(L, hi, "flags");
      hints.ai_flags = lluv_opt_flags_ui(L, -1, 0, FLAGS);
      lua_pop(L, 1);

      no_callback = lua_isnoneornil(L, argc + 4);
    }
    else {
      no_callback = lua_isnoneornil(L, argc + 3);
    }

#if LLUV_UV_VER_GE(1,3,0)
    if(no_callback){
      req = lluv_req_new(L, UV_GETADDRINFO, NULL);
      err = uv_getaddrinfo(loop->handle, LLUV_R(req, getaddrinfo), NULL, node, service, &hints);
      lua_settop(L, 0);
      if(err < 0){
        lluv_req_free(L, req);
        return lluv_fail(L, loop->flags, LLUV_ERR_UV, err, NULL);
      }
      lluv_push_addrinfo(L, LLUV_R(req, getaddrinfo)->addrinfo);
      uv_freeaddrinfo(LLUV_R(req, getaddrinfo)->addrinfo);
      lluv_req_free(L, req);
      return 1;
    }
#endif

    lluv_check_args_with_cb(L, argc + 4);
    req = lluv_req_new(L, UV_GETADDRINFO, NULL);

    err = uv_getaddrinfo(loop->handle, LLUV_R(req, getaddrinfo), lluv_on_getaddrinfo, node, service, &hints);

    lua_settop(L, 0);
    lluv_loop_pushself(L, loop);

    return lluv_return_loop_req(L, loop, req, err);
  }
}

LLUV_IMPL_SAFE(lluv_getnameinfo){
#define ARGN(n) (argc + n)

  static const lluv_uv_const_t FLAGS[] = {
    { NI_NOFQDN,        "nofqdn"       },
    { NI_NUMERICHOST,   "numerichost"  },
    { NI_NAMEREQD,      "namereqd"     },
    { NI_NUMERICSERV,   "numericserv"  },
    { NI_DGRAM,         "dgram"        },

    { 0, NULL }
  };

  lluv_loop_t *loop = lluv_opt_loop(L, 1, LLUV_FLAG_OPEN);
  int argc = loop ? 1 : 0;
  if(!loop)loop = lluv_default_loop(L);
  {
    struct sockaddr_storage sa;
    int err; unsigned int flags = 0;
    lluv_req_t *req;
    int has_callback = lua_isfunction(L, -1);

    // Push port number
    if(!lua_isnumber(L, ARGN(2))){
      lua_pushinteger(L, 0);
      lua_insert(L, ARGN(2));
    }

    err = lluv_check_addr(L, ARGN(1), &sa);
    if(err < 0){
      if(!has_callback){
        return lluv_fail(L, safe_flag | loop->flags, LLUV_ERR_UV, err, lua_tostring(L, argc + 1));
      }
      lluv_loop_pushself(L, loop);
      lluv_error_create(L, LLUV_ERR_UV, err, lua_tostring(L, argc + 1));
      lluv_loop_defer_call(L, loop, 2);
      lluv_loop_pushself(L, loop);
      return 1;
    }
    // ARG 1 - Address
    // ARG 2 - Port

    if(!lua_isfunction(L, ARGN(3))){
      flags = lluv_opt_flags_ui(L, ARGN(3), 0, FLAGS);
    }

#if LLUV_UV_VER_GE(1,3,0)
    if(!has_callback){
      uv_getnameinfo_t *ni;
      req = lluv_req_new(L, UV_GETNAMEINFO, NULL);
      err = uv_getnameinfo(loop->handle, LLUV_R(req, getnameinfo), NULL, (struct sockaddr*)&sa, flags);
      if(err < 0){
        lluv_req_free(L, req);
        return lluv_fail(L, loop->flags, LLUV_ERR_UV, err, NULL);
      }
      ni = LLUV_R(req, getnameinfo);
      lua_settop(L, 0);
      lua_pushstring(L, LLUV_R(req, getnameinfo)->host);
      lua_pushstring(L, LLUV_R(req, getnameinfo)->service);
      lluv_req_free(L, req);
      return 2;
    }
#endif

    lluv_check_args_with_cb(L, ARGN(4));
    req = lluv_req_new(L, UV_GETNAMEINFO, NULL);

    err = uv_getnameinfo(loop->handle, LLUV_R(req, getnameinfo), lluv_on_getnameinfo, (struct sockaddr*)&sa, flags);

    lua_settop(L, 0);
    lluv_loop_pushself(L, loop);
    return lluv_return_loop_req(L, loop, req, err);
  }
#undef ARGN
}

static const lluv_uv_const_t lluv_dns_constants[] = {
#define XX(C, L, N) {C, L},
    LLUV_AI_FAMILY_MAP(XX)
    LLUV_AI_STYPE_MAP(XX)
    LLUV_AI_PROTO_MAP(XX)
    LLUV_AI_FLAG_MAP(XX)
#undef  XX

  { 0, NULL }
};

#define LLUV_FUNCTIONS(F)                \
  {"getaddrinfo", lluv_getaddrinfo_##F}, \
  {"getnameinfo", lluv_getnameinfo_##F}, \

static const struct luaL_Reg lluv_functions[][3] = {
  {
    LLUV_FUNCTIONS(unsafe)

    {NULL,NULL}
  },
  {
    LLUV_FUNCTIONS(safe)

    {NULL,NULL}
  },
};

LLUV_INTERNAL void lluv_dns_initlib(lua_State *L, int nup, int safe){
  luaL_setfuncs(L, lluv_functions[safe], nup);
  lluv_register_constants(L, lluv_dns_constants);
}