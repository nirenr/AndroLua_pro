/*************************************************
*  LuaGL - an OpenGL binding for Lua
*  2007(c) Fabio Guerra
*  luagl.sourceforge.net
*-------------------------------------------------
*  Description: This file implements the GLU
*               binding for Lua 5
*-------------------------------------------------
* Changed by Antonio Scuri for LuaForge
*  http://luagl.luaforge.net
*-------------------------------------------------
*  See Copyright Notice in LuaGL.h
*************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#endif
#if defined (__APPLE__) || defined (OSX)
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#include <lua.h>
#include <lauxlib.h>

#include "luaglu.h"
#include "luagl_util.h"


static const luaglConst luaglu_const[] = {
   { "VERSION_1_1"                       , GLU_VERSION_1_1 },
   { "VERSION_1_2"                       , GLU_VERSION_1_2 },
   { "INVALID_ENUM"                      , GLU_INVALID_ENUM },
   { "INVALID_VALUE"                     , GLU_INVALID_VALUE },
   { "OUT_OF_MEMORY"                     , GLU_OUT_OF_MEMORY },
   { "INCOMPATIBLE_GL_VERSION"           , GLU_INCOMPATIBLE_GL_VERSION },
   { "VERSION"                           , GLU_VERSION },
   { "EXTENSIONS"                        , GLU_EXTENSIONS },
   { "TRUE"                              , GLU_TRUE },
   { "FALSE"                             , GLU_FALSE },
   { "SMOOTH"                            , GLU_SMOOTH },
   { "FLAT"                              , GLU_FLAT },
   { "NONE"                              , GLU_NONE },
   { "POINT"                             , GLU_POINT },
   { "LINE"                              , GLU_LINE },
   { "FILL"                              , GLU_FILL },
   { "SILHOUETTE"                        , GLU_SILHOUETTE },
   { "OUTSIDE"                           , GLU_OUTSIDE },
   { "INSIDE"                            , GLU_INSIDE },
   /*{ "TESS_MAX_COORD"                    , GLU_TESS_MAX_COORD },*/
   { "TESS_WINDING_RULE"                 , GLU_TESS_WINDING_RULE },
   { "TESS_BOUNDARY_ONLY"                , GLU_TESS_BOUNDARY_ONLY },
   { "TESS_TOLERANCE"                    , GLU_TESS_TOLERANCE },
   { "TESS_WINDING_ODD"                  , GLU_TESS_WINDING_ODD },
   { "TESS_WINDING_NONZERO"              , GLU_TESS_WINDING_NONZERO },
   { "TESS_WINDING_POSITIVE"             , GLU_TESS_WINDING_POSITIVE },
   { "TESS_WINDING_NEGATIVE"             , GLU_TESS_WINDING_NEGATIVE },
   { "TESS_WINDING_ABS_GEQ_TWO"          , GLU_TESS_WINDING_ABS_GEQ_TWO },
   { "TESS_BEGIN"                        , GLU_TESS_BEGIN },
   { "TESS_VERTEX"                       , GLU_TESS_VERTEX },
   { "TESS_END"                          , GLU_TESS_END },
   { "TESS_ERROR"                        , GLU_TESS_ERROR },
   { "TESS_EDGE_FLAG"                    , GLU_TESS_EDGE_FLAG },
   { "TESS_COMBINE"                      , GLU_TESS_COMBINE },
   { "TESS_BEGIN_DATA"                   , GLU_TESS_BEGIN_DATA },
   { "TESS_VERTEX_DATA"                  , GLU_TESS_VERTEX_DATA },
   { "TESS_END_DATA"                     , GLU_TESS_END_DATA },
   { "TESS_ERROR_DATA"                   , GLU_TESS_ERROR_DATA },
   { "TESS_EDGE_FLAG_DATA"               , GLU_TESS_EDGE_FLAG_DATA },
   { "TESS_COMBINE_DATA"                 , GLU_TESS_COMBINE_DATA },
   { "TESS_ERROR1"                       , GLU_TESS_ERROR1 },
   { "TESS_ERROR2"                       , GLU_TESS_ERROR2 },
   { "TESS_ERROR3"                       , GLU_TESS_ERROR3 },
   { "TESS_ERROR4"                       , GLU_TESS_ERROR4 },
   { "TESS_ERROR5"                       , GLU_TESS_ERROR5 },
   { "TESS_ERROR6"                       , GLU_TESS_ERROR6 },
   { "TESS_ERROR7"                       , GLU_TESS_ERROR7 },
   { "TESS_ERROR8"                       , GLU_TESS_ERROR8 },
   { "TESS_MISSING_BEGIN_POLYGON"        , GLU_TESS_MISSING_BEGIN_POLYGON },
   { "TESS_MISSING_BEGIN_CONTOUR"        , GLU_TESS_MISSING_BEGIN_CONTOUR },
   { "TESS_MISSING_END_POLYGON"          , GLU_TESS_MISSING_END_POLYGON },
   { "TESS_MISSING_END_CONTOUR"          , GLU_TESS_MISSING_END_CONTOUR },
   { "TESS_COORD_TOO_LARGE"              , GLU_TESS_COORD_TOO_LARGE },
   { "TESS_NEED_COMBINE_CALLBACK"        , GLU_TESS_NEED_COMBINE_CALLBACK },
   { "AUTO_LOAD_MATRIX"                  , GLU_AUTO_LOAD_MATRIX },
   { "CULLING"                           , GLU_CULLING },
   { "SAMPLING_TOLERANCE"                , GLU_SAMPLING_TOLERANCE },
   { "DISPLAY_MODE"                      , GLU_DISPLAY_MODE },
   { "PARAMETRIC_TOLERANCE"              , GLU_PARAMETRIC_TOLERANCE },
   { "SAMPLING_METHOD"                   , GLU_SAMPLING_METHOD },
   { "U_STEP"                            , GLU_U_STEP },
   { "V_STEP"                            , GLU_V_STEP },
   { "PATH_LENGTH"                       , GLU_PATH_LENGTH },
   { "PARAMETRIC_ERROR"                  , GLU_PARAMETRIC_ERROR },
   { "DOMAIN_DISTANCE"                   , GLU_DOMAIN_DISTANCE },
   { "MAP1_TRIM_2"                       , GLU_MAP1_TRIM_2 },
   { "MAP1_TRIM_3"                       , GLU_MAP1_TRIM_3 },
   { "OUTLINE_POLYGON"                   , GLU_OUTLINE_POLYGON },
   { "OUTLINE_PATCH"                     , GLU_OUTLINE_PATCH },
   { "NURBS_ERROR1"                      , GLU_NURBS_ERROR1 },
   { "NURBS_ERROR2"                      , GLU_NURBS_ERROR2 },
   { "NURBS_ERROR3"                      , GLU_NURBS_ERROR3 },
   { "NURBS_ERROR4"                      , GLU_NURBS_ERROR4 },
   { "NURBS_ERROR5"                      , GLU_NURBS_ERROR5 },
   { "NURBS_ERROR6"                      , GLU_NURBS_ERROR6 },
   { "NURBS_ERROR7"                      , GLU_NURBS_ERROR7 },
   { "NURBS_ERROR8"                      , GLU_NURBS_ERROR8 },
   { "NURBS_ERROR9"                      , GLU_NURBS_ERROR9 },
   { "NURBS_ERROR10"                     , GLU_NURBS_ERROR10 },
   { "NURBS_ERROR11"                     , GLU_NURBS_ERROR11 },
   { "NURBS_ERROR12"                     , GLU_NURBS_ERROR12 },
   { "NURBS_ERROR13"                     , GLU_NURBS_ERROR13 },
   { "NURBS_ERROR14"                     , GLU_NURBS_ERROR14 },
   { "NURBS_ERROR15"                     , GLU_NURBS_ERROR15 },
   { "NURBS_ERROR16"                     , GLU_NURBS_ERROR16 },
   { "NURBS_ERROR17"                     , GLU_NURBS_ERROR17 },
   { "NURBS_ERROR18"                     , GLU_NURBS_ERROR18 },
   { "NURBS_ERROR19"                     , GLU_NURBS_ERROR19 },
   { "NURBS_ERROR20"                     , GLU_NURBS_ERROR20 },
   { "NURBS_ERROR21"                     , GLU_NURBS_ERROR21 },
   { "NURBS_ERROR22"                     , GLU_NURBS_ERROR22 },
   { "NURBS_ERROR23"                     , GLU_NURBS_ERROR23 },
   { "NURBS_ERROR24"                     , GLU_NURBS_ERROR24 },
   { "NURBS_ERROR25"                     , GLU_NURBS_ERROR25 },
   { "NURBS_ERROR26"                     , GLU_NURBS_ERROR26 },
   { "NURBS_ERROR27"                     , GLU_NURBS_ERROR27 },
   { "NURBS_ERROR28"                     , GLU_NURBS_ERROR28 },
   { "NURBS_ERROR29"                     , GLU_NURBS_ERROR29 },
   { "NURBS_ERROR30"                     , GLU_NURBS_ERROR30 },
   { "NURBS_ERROR31"                     , GLU_NURBS_ERROR31 },
   { "NURBS_ERROR32"                     , GLU_NURBS_ERROR32 },
   { "NURBS_ERROR33"                     , GLU_NURBS_ERROR33 },
   { "NURBS_ERROR34"                     , GLU_NURBS_ERROR34 },
   { "NURBS_ERROR35"                     , GLU_NURBS_ERROR35 },
   { "NURBS_ERROR36"                     , GLU_NURBS_ERROR36 },
   { "NURBS_ERROR37"                     , GLU_NURBS_ERROR37 },
   { "CW"                                , GLU_CW },
   { "CCW"                               , GLU_CCW },
   { "INTERIOR"                          , GLU_INTERIOR },
   { "EXTERIOR"                          , GLU_EXTERIOR },
   { "UNKNOWN"                           , GLU_UNKNOWN },
   { "BEGIN"                             , GLU_BEGIN },
   { "VERTEX"                            , GLU_VERTEX },
   { "END"                               , GLU_END },
   { "ERROR"                             , GLU_ERROR },
   { "EDGE_FLAG"                         , GLU_EDGE_FLAG },
   {0, 0}
};

static GLenum luaglu_get_gl_enum(lua_State *L, int index)
{
  return luagl_get_enum(L, index, luaglu_const);
}

/* GetString (name) -> string */
static int luaglu_get_string(lua_State *L)
{
  GLenum e;
  const GLubyte *str;

  /* test argument type */
  if(!lua_isstring(L, 1))
    luaL_error(L, "incorrect argument to function 'glu.GetString'");

  /* get string parameter */
  e = luaglu_get_gl_enum(L, 1);

  /* test argument */
  if(e == LUAGL_ENUM_ERROR)
    luaL_error(L, "incorrect string argument to function 'glu.GetString'");

  /* call opengl function */
  str = gluGetString(e);

  lua_pushstring(L, (const char*)str);

  return 1;
}

/* Ortho2D(left, right, bottom, top) -> none */
static int luaglu_ortho_2D(lua_State *L)
{
  if (!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4)))
  {
    luaL_error(L, "incorrect argument to function 'glu.Ortho2D'");
    return 0;
  }

  gluOrtho2D( (GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2), 
    (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4));

  return 0;
}

/* Perspective(fovy, aspect, near, far) -> none */
static int luaglu_perspective(lua_State *L)
{
  if (!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4)))
  {
    luaL_error(L, "incorrect string argument to function 'glu.Perspective'");
    return 0;
  }
  gluPerspective((GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2), (GLdouble)lua_tonumber(L, 3), (GLdouble)lua_tonumber(L, 4));
  return 0;
}

/* LookAt(Ex, Ey, Ez, Lx, Ly, Lz, Ux, Uy, Uz) -> none */
static int luaglu_look_at(lua_State *L)
{
  if (!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)
    && lua_isnumber(L, 4) && lua_isnumber(L, 5) && lua_isnumber(L, 6)
    && lua_isnumber(L, 7) && lua_isnumber(L, 8) && lua_isnumber(L, 9)))
  {
    luaL_error(L, "incorrect argument to function 'glu.LookAt'");
    return 0;
  }
  gluLookAt(
    (GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2), (GLdouble)lua_tonumber(L, 3),
    (GLdouble)lua_tonumber(L, 4), (GLdouble)lua_tonumber(L, 5), (GLdouble)lua_tonumber(L, 6),
    (GLdouble)lua_tonumber(L, 7), (GLdouble)lua_tonumber(L, 8), (GLdouble)lua_tonumber(L, 9));

  return 0;
}

/* PickMatrix( x, y, deltax, deltay, viewportArray) -> none */
static int luaglu_pick_matrix(lua_State *L)
{
  GLint *vp;
  int num_args;

  if (!( lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)
    && lua_isnumber(L, 4)))
  {
    luaL_error(L, "incorrect argument to function 'glu.PickMatrix'");
    return 0;
  }

  if(!lua_istable(L, 5) || (num_args = luagl_get_arrayi(L, 5, &vp)) < 4)
  {
    luaL_error(L, "incorrect argument to function 'glu.PickMatrix'");
    return 0;
  }

  gluPickMatrix(
    (GLdouble)lua_tonumber(L, 1), (GLdouble)lua_tonumber(L, 2), (GLdouble)lua_tonumber(L, 3),
    (GLdouble)lua_tonumber(L, 4), vp);

  LUAGL_DELETE_ARRAY(vp);

  return 0;
}

/* ErrorString(errorCode) -> string */
static int luaglu_error_string(lua_State *L)
{
  if (!lua_isnumber(L, 1))
  {
    luaL_error(L, "incorrect argument to function 'glu.ErrorString'");
    return 0;
  }

  lua_pushstring(L, (char *)gluErrorString((GLenum)lua_tonumber(L, 1)));

  return 1;
}

/*********************************** GLU Mipmapping ***********************************/

/*int gluScaleImage( GLenum format, GLsizei widthin,
GLsizei heightin, GLenum typein, const void *datain,
GLsizei widthout, GLsizei heightout, GLenum typeout,
void *dataout );*/

/*int gluBuild1DMipmaps( GLenum target,
GLint internalFormat, GLsizei width, GLenum format,
GLenum type, const void *data );*/

/*int gluBuild2DMipmaps( GLenum target,
GLint internalFormat, GLsizei width, GLsizei height,
GLenum format, GLenum type, const void *data );*/

/*Build2DMipmaps(textureData) -> error */
static int luaglu_build_2d_mipmaps(lua_State *L)
{
  GLenum target, format, type;
  GLubyte *pixels;
  GLint internalFormat;
  GLsizei width, height, w, h;
  int result;

  if(!lua_istable(L, 1))
    LUAGL_SHOWERROR("incorrect argument to function 'glu.Build2DMipmaps'");

  lua_pushstring(L, "target");  lua_gettable(L, 1);  target = luaglu_get_gl_enum(L, -1);  lua_pop(L, 1);
  lua_pushstring(L, "format");  lua_gettable(L, 1);  format = luaglu_get_gl_enum(L, -1);  lua_pop(L, 1);
  lua_pushstring(L, "type");    lua_gettable(L, 1);  type   = luaglu_get_gl_enum(L, -1);  lua_pop(L, 1);
  lua_pushstring(L, "width");   lua_gettable(L, 1);  width  = (GLsizei)lua_tonumber(L, -1);  lua_pop(L, 1);
  lua_pushstring(L, "height");  lua_gettable(L, 1);  height = (GLsizei)lua_tonumber(L, -1);  lua_pop(L, 1);
  lua_pushstring(L, "components");  lua_gettable(L, 1);  internalFormat = (GLint)lua_tonumber(L, -1);  lua_pop(L, 1);

  h = luagl_get_array2ubyte(L, 1, &pixels, &w);

  w /= internalFormat;

  if (width > w)
    width = w;

  if (height > h)
    height = h;

  result = gluBuild2DMipmaps(target, internalFormat, width,
    height, format, type, pixels);

  LUAGL_DELETE_ARRAY(pixels);

  lua_pushnumber(L, result);

  return 1;
}

/*int gluBuild3DMipmaps( GLenum target,
GLint internalFormat, GLsizei width, GLsizei height,
GLsizei depth, GLenum format, GLenum type,
const void *data );*/

static const luaL_reg luaglu_lib[] = {
  {"GetString", luaglu_get_string},
  {"Ortho2D", luaglu_ortho_2D},
  {"Perspective", luaglu_perspective},
  {"LookAt", luaglu_look_at},
  {"PickMatrix", luaglu_pick_matrix},
  {"Build2DMipmaps", luaglu_build_2d_mipmaps},
  {"ErrorString", luaglu_error_string},
  {NULL, NULL}
};

int luaopen_luaglu(lua_State *L)
{
  luaL_openlib(L, "glu", luaglu_lib, 0);

  luagl_initconst(L, luaglu_const);

  return 1;
}
