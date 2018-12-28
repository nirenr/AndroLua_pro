/*************************************************
*  LuaGL - an OpenGL binding for Lua
*  2003-2004(c) Fabio Guerra, Cleyde Marlyse
*  http://luagl.sourceforge.net
*-------------------------------------------------
*  Description: This file implements the OpenGL
*               binding for Lua 5
*-------------------------------------------------
* Mantained by Antonio Scuri since 2009
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
#else
#include <GLES/gl.h>
//#include <EGL/egl.h>
#include <GLES/glext.h>
//#include "importgl.h"

#endif

#include <lua.h>
#include <lauxlib.h>

#include "luagl.h"
#include "luagl_util.h"
#include "luagl_const.h"


#define LUAGL_VERSION "1.9"


#ifndef LUA_UNSIGNED
#define lua_Unsigned lua_Integer
#define lua_pushunsigned lua_pushinteger
#define luaL_checkunsigned luaL_checkinteger
#endif


/*AlphaFunc (func, ref) -> none*/
static int luagl_alpha_func(lua_State *L)
{
  glAlphaFunc(luagl_get_gl_enum(L, 1), (GLclampf)luaL_checknumber(L, 2));
  return 0;
}

/*BindTexture (target, texture) -> none*/
static int luagl_bind_texture(lua_State *L)
{
  glBindTexture(luagl_get_gl_enum(L, 1), luaL_checkinteger(L, 2));
  return 0;
}

/*BlendFunc (sfactor, dfactor) -> none*/
static int luagl_blend_func(lua_State *L)
{
  glBlendFunc(luagl_get_gl_enum(L, 1), luagl_get_gl_enum(L, 2));
  return 0;
}

/*Clear (mask) -> none*/
static int luagl_clear(lua_State *L)
{
  glClear(luagl_get_gl_enum(L, 1));
  return 0;
}

/*ClearColor (red, green, blue, alpha) -> none*/
static int luagl_clear_color(lua_State *L)
{
  glClearColor((GLclampf)luaL_checknumber(L, 1), (GLclampf)luaL_checknumber(L, 2),
               (GLclampf)luaL_checknumber(L, 3), (GLclampf)luaL_checknumber(L, 4));
  return 0;
}

/*ClearDepth (depth) -> none*/
static int luagl_clear_depth(lua_State *L)
{
  glClearDepthf((GLclampf)luaL_checknumber(L, 1));
  return 0;
}

/*ClearStencil (s) -> none*/
static int luagl_clear_stencil(lua_State *L)
{
  glClearStencil(luaL_checkinteger(L, 1));
  return 0;
}

/*ClipPlane (plane, equationArray) -> none*/
static int luagl_clip_plane(lua_State *L)
{
  GLfloat *equation;

  luagl_get_arrayf(L, 2, &equation);

  glClipPlanef(luagl_get_gl_enum(L, 1), equation);

  LUAGL_DELETE_ARRAY(equation);
  return 0;
}

/*Color (red, green, blue [, alpha]) -> none
  Color (color) -> none*/
static int luagl_color(lua_State *L)
{
  if (lua_gettop(L)==4)
  {
    glColor4f(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
  }
  else
  {
    luaL_argerror(L, 1, "invalid number of args");
  }

  return 0;
}

/*ColorMask (red, green, blue, alpha) -> none*/
static int luagl_color_mask(lua_State *L)
{
  glColorMask((GLboolean)luagl_checkboolean(L, 1), (GLboolean)luagl_checkboolean(L, 2),
              (GLboolean)luagl_checkboolean(L, 3), (GLboolean)luagl_checkboolean(L, 4));
  return 0;
}

/*ColorPointer (colorArray) -> none*/
static int luagl_color_pointer(lua_State *L)
{
  GLint size;
  static GLfloat *parray = NULL;

  LUAGL_DELETE_ARRAY(parray);

  if (lua_isnil(L,1))
    return 0;

  if (lua_isnumber(L, 2))
  {
    size = luaL_checkinteger(L, 2);
    luagl_get_arrayf(L, 1, &parray);
  }
  else 
  {
    int h = luagl_get_array2f(L, 1, &parray, &size);
    if (h==-1)
      luaL_argerror(L, 1, "must be a table of tables");
  }

  glColorPointer(size, GL_FLOAT, 0, parray);
  return 0;
}

/*CopyTexImage (level, internalFormat, border, x, y, width[, height]) -> none*/
static int luagl_copy_tex_image(lua_State *L)
{
  int num_args = lua_gettop(L);
  if (num_args > 6)
  {
    glCopyTexImage2D(GL_TEXTURE_2D, luaL_checkinteger(L, 1), luagl_get_gl_enum(L, 2),
      luaL_checkinteger(L, 4), luaL_checkinteger(L, 5),
      luaL_checkinteger(L, 6), luaL_checkinteger(L, 7),
      luaL_checkinteger(L, 3));
  }
  return 0;
}

/*CopyTexSubImage (level, x, y, xoffset, width[, yoffset, height]) -> none*/
static int luagl_copy_tex_sub_image(lua_State *L)
{
  int num_args = lua_gettop(L);
  if (num_args >= 7)
  {
    glCopyTexSubImage2D(GL_TEXTURE_2D,
      luaL_checkinteger(L, 1), luaL_checkinteger(L, 4),
      luaL_checkinteger(L, 6), luaL_checkinteger(L, 2),
      luaL_checkinteger(L, 3), luaL_checkinteger(L, 5),
      luaL_checkinteger(L, 7));
  }
  return 0;
}

/*CullFace (mode) -> none*/
static int luagl_cull_face(lua_State *L)
{
  glCullFace(luagl_get_gl_enum(L, 1));
  return 0;
}

/*DeleteTextures (texturesArray) -> none*/
static int luagl_delete_textures(lua_State *L)
{
  int n;
  GLuint *textures;

  n = luagl_get_arrayui(L, 1, &textures);

  glDeleteTextures((GLsizei)n, (GLuint *)textures);

  LUAGL_DELETE_ARRAY(textures);

  return 0;
}

/*DepthFunc (func) -> none*/
static int luagl_depth_func(lua_State *L)
{
  glDepthFunc(luagl_get_gl_enum(L, 1));
  return 0;
}

/*DepthMask (flag) -> none*/
static int luagl_depth_mask(lua_State *L)
{
  glDepthMask((GLboolean)luagl_checkboolean(L, 1));
  return 0;
}


/*Disable (cap) -> none*/
static int luagl_disable(lua_State *L)
{
  glDisable(luagl_get_gl_enum(L, 1));
  return 0;
}

/*DisableClientState (parray) -> none*/
static int luagl_disable_client_state(lua_State *L)
{
  glDisableClientState(luagl_get_gl_enum(L, 1));
  return 0;
}

/*DrawArrays (mode, first, count) -> none*/
static int luagl_draw_arrays(lua_State *L)
{
  glDrawArrays(luagl_get_gl_enum(L, 1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3));
  return 0;
}

/*DrawElements (mode, indicesArray) -> none*/
static int luagl_draw_elements(lua_State *L)
{
  int n;
  GLuint *indices;

  n = luagl_get_arrayui(L, 2, &indices);

  glDrawElements(luagl_get_gl_enum(L, 1), n, GL_UNSIGNED_INT, indices);

  LUAGL_DELETE_ARRAY(indices);
  return 0;
}


/*Enable (cap) -> none*/
static int luagl_enable(lua_State *L)
{
  glEnable(luagl_get_gl_enum(L, 1));
  return 0;
}

/*EnableClientState (parray) -> none*/
static int luagl_enable_client_state(lua_State *L)
{
  glEnableClientState(luagl_get_gl_enum(L, 1));
  return 0;
}

/*Finish () -> none*/
static int luagl_finish(lua_State *L)
{
  glFinish();
  return 0;
}

/*Flush () -> none*/
static int luagl_flush(lua_State *L)
{
  glFlush();
  return 0;
}

/*Fog (pname, param) -> none
  Fog (pname, paramsArray) -> none*/
static int luagl_fog(lua_State *L)
{
  GLfloat *param;

  if(lua_istable(L, 2))
  {
    luagl_get_arrayf(L, 2, &param);

    glFogfv(luagl_get_gl_enum(L, 1), (GLfloat*)param);

    LUAGL_DELETE_ARRAY(param);
    return 0;
  }
  else if(lua_isnumber(L, 2))
    glFogf(luagl_get_gl_enum(L, 1), (GLfloat)luaL_checknumber(L, 2));
	//lua_error();
  return 0;
}

/*FrontFace (mode) -> none*/
static int luagl_front_face(lua_State *L)
{
  glFrontFace(luagl_get_gl_enum(L, 1));
  return 0;
}

/*Frustum (left, right, bottom, top, zNear, zFar) -> none*/
static int luagl_frustum(lua_State *L)
{
  glFrustumf(luaL_checknumber(L, 1), luaL_checknumber(L, 2),
            luaL_checknumber(L, 3), luaL_checknumber(L, 4),
            luaL_checknumber(L, 5), luaL_checknumber(L, 6));
  return 0;
}

/*GenTextures (n) -> texturesArray*/
static int luagl_gen_textures(lua_State *L)
{
  GLsizei size;
  GLuint *textures;

  size = luaL_checkinteger(L, 1);
  textures = LUAGL_NEW_ARRAY(GLuint, size);

  glGenTextures(size, textures);

  luagl_push_arrayui(L, textures, size);

  LUAGL_DELETE_ARRAY(textures);

  return 1;
}

/*Get (pname) -> params*/
static int luagl_get(lua_State *L)
{
  int i, size=1;
  GLenum e;
  GLfloat *params;
  int mask;

  e = luagl_get_gl_enum(L, 1);

  switch(e)
  {
  case GL_STENCIL_VALUE_MASK:

  case GL_STENCIL_WRITEMASK:
  case GL_DEPTH_RANGE:

  case GL_CURRENT_NORMAL:
    size = 3;
    break;

  case GL_COLOR_CLEAR_VALUE:
  case GL_COLOR_WRITEMASK:
  case GL_CURRENT_COLOR:
  case GL_CURRENT_TEXTURE_COORDS:
  case GL_FOG_COLOR:
  case GL_LIGHT_MODEL_AMBIENT:
  case GL_SCISSOR_BOX:
  case GL_TEXTURE_ENV_COLOR:
  case GL_VIEWPORT:
    size = 4;
    break;

  case GL_MODELVIEW_MATRIX:
  case GL_PROJECTION_MATRIX:
  case GL_TEXTURE_MATRIX:
    size = 16;
    break;

  default:
    luaL_argerror(L, 1, "unknown enumeration.");
    break;
  }

  params = LUAGL_NEW_ARRAY(GLfloat, size);

  glGetFloatv(e, params);

  for(i = 0; i < size; i++)
    lua_pushnumber(L, params[i]);

  LUAGL_DELETE_ARRAY(params);

  return size;
}

/*GetConst (pname) -> constant string*/
static int luagl_get_const(lua_State *L)
{
  int i, size=1;
  GLenum e;
  GLenum *params;

  e = luagl_get_gl_enum(L, 1);

  switch(e)
  {
  case GL_DEPTH_RANGE:
  case GL_MAX_VIEWPORT_DIMS:

  case GL_CURRENT_NORMAL:
    size = 3;
    break;

  case GL_COLOR_CLEAR_VALUE:
  case GL_COLOR_WRITEMASK:
  case GL_CURRENT_COLOR:
  case GL_CURRENT_TEXTURE_COORDS:
  case GL_FOG_COLOR:
  case GL_LIGHT_MODEL_AMBIENT:
  case GL_SCISSOR_BOX:
  case GL_TEXTURE_ENV_COLOR:
  case GL_VIEWPORT:
    size = 4;
    break;

  case GL_MODELVIEW_MATRIX:
  case GL_PROJECTION_MATRIX:
  case GL_TEXTURE_MATRIX:
    size = 16;
    break;
  }

  params = LUAGL_NEW_ARRAY(GLenum, size);

  glGetIntegerv(e, (GLint*)params);

  for(i = 0; i < size; i++)
    luagl_pushenum(L, params[i]);

  LUAGL_DELETE_ARRAY(params);

  return size;
}

/*GetArray (pname) -> paramsArray*/
static int luagl_get_array(lua_State *L)
{
  int size = 1;
  GLenum e;
  GLfloat *params;

  e = luagl_get_gl_enum(L, 1);

  switch(e)
  {
  case GL_DEPTH_RANGE:
  case GL_MAX_VIEWPORT_DIMS:
    size = 2;
    break;

  case GL_CURRENT_NORMAL:
    size = 3;
    break;

  case GL_COLOR_CLEAR_VALUE:
  case GL_COLOR_WRITEMASK:
  case GL_CURRENT_COLOR:
  case GL_CURRENT_TEXTURE_COORDS:
  case GL_FOG_COLOR:
  case GL_LIGHT_MODEL_AMBIENT:
  case GL_SCISSOR_BOX:
  case GL_TEXTURE_ENV_COLOR:
  case GL_VIEWPORT:
    size = 4;
    break;

  case GL_MODELVIEW_MATRIX:
  case GL_PROJECTION_MATRIX:
  case GL_TEXTURE_MATRIX:
    size = 16;
    break;
  }

  params = LUAGL_NEW_ARRAY(GLfloat, size);

  glGetFloatv(e, params);

  luagl_push_arrayf(L, params, size);

  LUAGL_DELETE_ARRAY(params);

  return 1;
}

/*GetClipPlane (plane) -> equationArray*/
static int luagl_get_clip_plane(lua_State *L)
{
  GLfloat *equation;

  equation = LUAGL_NEW_ARRAY(GLfloat, 4);

  glGetClipPlanef(luagl_get_gl_enum(L, 1), equation);

  luagl_push_arrayf(L, equation, 4);

  LUAGL_DELETE_ARRAY(equation);

  return 1;
}

/*GetError () -> error flag*/
static int luagl_get_error(lua_State *L)
{
  GLenum error = glGetError();
  if(error == GL_NO_ERROR)
    lua_pushnil(L);
  else
    luagl_pushenum(L, error);
  return 1;
}

/*GetLight (light, pname) -> paramsArray*/
static int luagl_get_light(lua_State *L)
{
  int size = 1;
  GLenum e1, e2;
  GLfloat *params;

  e1 = luagl_get_gl_enum(L, 1);
  e2 = luagl_get_gl_enum(L, 2);

  switch(e2)
  {
  case GL_AMBIENT:
  case GL_DIFFUSE:
  case GL_SPECULAR:
  case GL_POSITION:
    size = 4;
    break;
  case GL_SPOT_DIRECTION :
    size = 3;
    break;
  case GL_SPOT_EXPONENT:
  case GL_SPOT_CUTOFF:
  case GL_CONSTANT_ATTENUATION:
  case GL_LINEAR_ATTENUATION:
  case GL_QUADRATIC_ATTENUATION:
    size = 1;
    break;
  }

  params = LUAGL_NEW_ARRAY(GLfloat, size);

  glGetLightfv(e1, e2, params);

  luagl_push_arrayf(L, params, size);

  LUAGL_DELETE_ARRAY(params);

  return 1;
}


/*GetMaterial (face, pname) -> paramsArray*/
static int luagl_get_material(lua_State *L)
{
  int size = 1;
  GLenum e1, e2;
  GLfloat *params;

  /* get string parameters */
  e1 = luagl_get_gl_enum(L, 1);
  e2 = luagl_get_gl_enum(L, 2);

  switch(e2)
  {
  case GL_AMBIENT:
  case GL_DIFFUSE:
  case GL_SPECULAR:
  case GL_EMISSION:
    size = 4;
    break;

  case GL_SHININESS:
    size = 1;
    break;
  }

  params = LUAGL_NEW_ARRAY(GLfloat, size);

  glGetMaterialfv(e1, e2, params);

  luagl_push_arrayf(L, params, size);

  LUAGL_DELETE_ARRAY(params);

  return 1;
}


/*GetPointer (pname, n) -> valuesArray*/
static int luagl_get_pointer(lua_State *L)
{
  int n;
  GLenum e;

  e = luagl_get_gl_enum(L, 1);
  n = luaL_checkinteger(L, 2);


    GLfloat *params;
    glGetPointerv(e, (void *)&params);
    if(params == 0)
      return 0;

    luagl_push_arrayf(L, params, n);

  return 1;
}

/*GetString (name) -> string*/
static int luagl_get_string(lua_State *L)
{
  lua_pushstring(L, (const char*)glGetString(luagl_get_gl_enum(L, 1)));
  return 1;
}

/*GetTexEnv (pname) -> paramsArray*/
static int luagl_get_tex_env(lua_State *L)
{
  GLenum e1;

  e1 = luagl_get_gl_enum(L, 1);

  if (e1 != GL_TEXTURE_ENV_MODE)
  {
    GLfloat *params;

    params = LUAGL_NEW_ARRAY(GLfloat, 4);

    glGetTexEnvfv(GL_TEXTURE_ENV, e1, params);

    luagl_push_arrayf(L, params, 4);

    LUAGL_DELETE_ARRAY(params);
  }
  else 
  {
    GLint e2;
    glGetTexEnviv(GL_TEXTURE_ENV, e1, &e2);
    luagl_pushenum(L, e2);
  }

  return 1;
}

static int luagl_get_depth(GLenum format)
{
  int depth = 0;
  switch(format)
  {

  case GL_ALPHA:
  case GL_LUMINANCE:
    depth = 1;
    break;

  case GL_LUMINANCE_ALPHA:
    depth = 2;
    break;

  case GL_RGB:
#ifdef GL_BGR_EXT
  case GL_BGR_EXT:
#endif
    depth = 3;
    break;

  case GL_RGBA:
#ifdef GL_BGRA_EXT
  case GL_BGRA_EXT:
#endif
    depth = 4;
    break;
  }

  return depth;
}


/*GetTexParameter (target, pname) -> paramsArray*/
static int luagl_get_tex_parameter(lua_State *L)
{
  GLenum target, pname;

  target = luagl_get_gl_enum(L, 1);
  pname = luagl_get_gl_enum(L, 2);

    GLint e;
    glGetTexParameteriv(target, pname, &e);
    luagl_pushenum(L, e);
  
  return 1;
}

/*Hint (target, mode) -> none*/
static int luagl_hint(lua_State *L)
{
  glHint(luagl_get_gl_enum(L, 1), luagl_get_gl_enum(L, 2));
  return 0;
}

/*InitNames () -> none*/
static int luagl_init_names(lua_State *L)
{
  glInitNames();
  return 0;
}

/*IsEnabled (cap) -> true/false*/
static int luagl_is_enabled(lua_State *L)
{
  lua_pushboolean(L, glIsEnabled(luagl_get_gl_enum(L, 1)));
  return 1;
}

/*IsTexture (texture) -> true/false*/
static int luagl_is_texture(lua_State *L)
{
  lua_pushboolean(L, glIsTexture(luaL_checkinteger(L, 1)));
  return 1;
}

/*Light (light, pname, param) -> none
  Light (light, pname, paramsArray) -> none*/
static int luagl_light(lua_State *L)
{
  if(lua_istable(L, 3))
  {
    GLfloat *params;
    luagl_get_arrayf(L, 3, &params);

    glLightfv(luagl_get_gl_enum(L, 1), luagl_get_gl_enum(L, 2), (GLfloat *)params);

    LUAGL_DELETE_ARRAY(params);
  }
  else 
    glLightf(luagl_get_gl_enum(L, 1), luagl_get_gl_enum(L, 2), (GLfloat)luaL_checknumber(L, 3));
  return 0;
}

/*LightModel (pname, param) -> none
  LightModel (pname, paramsArray) -> none*/
static int luagl_light_model(lua_State *L)
{
  if (lua_istable(L, 2))
  {
    GLfloat *params;
    luagl_get_arrayf(L, 2, &params);

    glLightModelfv(luagl_get_gl_enum(L, 1), (GLfloat *)params);

    LUAGL_DELETE_ARRAY(params);
  }
  else 
    glLightModelf(luagl_get_gl_enum(L, 1), (GLfloat)luaL_checknumber(L, 2));
  return 0;
}


/*LineWidth (width) -> none*/
static int luagl_line_width(lua_State *L)
{
  glLineWidth((GLfloat)luaL_checknumber(L, 1));
  return 0;
}

/*LoadIdentity () -> none*/
static int luagl_load_identity(lua_State *L)
{
  glLoadIdentity();
  return 0;
}

/*LoadMatrix (mArray) -> none*/
static int luagl_load_matrix(lua_State *L)
{
  GLfloat *m;
  int n;

  n = luagl_get_arrayf(L, 1, &m);
  if (n < 16)
  {
    LUAGL_DELETE_ARRAY(m);
    luaL_argerror(L, 1, "invalid number of elements in the matrix table (n<16).");
  }

  glLoadMatrixf(m);

  LUAGL_DELETE_ARRAY(m);

  return 0;
}

/*LogicOp (opcode) -> none*/
static int luagl_logic_op(lua_State *L)
{
  glLogicOp(luagl_get_gl_enum(L, 1));
  return 0;
}


/*Material (face, pname, param) -> none*/
static int luagl_material(lua_State *L)
{
  GLenum e1, e2;
  GLfloat *params;

  e1 = luagl_get_gl_enum(L, 1);
  e2 = luagl_get_gl_enum(L, 2);

  if (lua_istable(L, 3))
  {
    luagl_get_arrayf(L, 3, &params);

    glMaterialfv(e1, e2, (GLfloat *)params);

    LUAGL_DELETE_ARRAY(params);
  }
  else 
    glMaterialf(e1, e2, (GLfloat)luaL_checknumber(L, 3));
  return 0;
}

/*MatrixMode (mode) -> none*/
static int luagl_matrix_mode(lua_State *L)
{
  glMatrixMode(luagl_get_gl_enum(L, 1));
  return 0;
}

/*MultMatrix (mArray) -> none*/
static int luagl_mult_matrix(lua_State *L)
{
  GLfloat *m;
  int n;

  n = luagl_get_arrayf(L, 1, &m);
  if (n < 16)
  {
    LUAGL_DELETE_ARRAY(m);
    luaL_argerror(L, 1, "invalid number of elements in the matrix table (n<16).");
  }

  glMultMatrixf(m);

  LUAGL_DELETE_ARRAY(m);

  return 0;
}

/*Normal (nx, ny, nz) -> none
  Normal (nArray) -> none*/
static int luagl_normal(lua_State *L)
{
  glNormal3f(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));

  return 0;
}

/*NormalPointer (normalArray) -> none*/
static int luagl_normal_pointer(lua_State *L)
{
  GLint size;
  static GLfloat *parray = NULL;

  LUAGL_DELETE_ARRAY(parray);

  if(lua_isnil(L,1))
    return 0;

  if (lua_isnumber(L, 2))
  {
    size = luaL_checkinteger(L, 2);
    luagl_get_arrayf(L, 1, &parray);
  }
  else 
  {
    int h = luagl_get_array2f(L, 1, &parray, &size);
    if (h==-1)
      luaL_argerror(L, 1, "must be a table of tables");
  }

  glNormalPointer(GL_FLOAT, 0, parray);

  return 0;
}

/*Ortho (left, right, bottom, top, zNear, zFar) -> none*/
static int luagl_ortho(lua_State *L)
{
  glOrthof(luaL_checknumber(L, 1), luaL_checknumber(L, 2),
          luaL_checknumber(L, 3), luaL_checknumber(L, 4),
          luaL_checknumber(L, 5), luaL_checknumber(L, 6));
  return 0;
}

/*PointSize (size) -> none*/
static int luagl_point_size(lua_State *L)
{
  glPointSize((GLfloat)luaL_checknumber(L, 1));
  return 0;
}

/*PolygonOffset (factor, units) -> none*/
static int luagl_polygon_offset(lua_State *L)
{
  glPolygonOffset((GLfloat)luaL_checknumber(L, 1), (GLfloat)luaL_checknumber(L, 2));
  return 0;
}

/*PopMatrix () -> none*/
static int luagl_pop_matrix(lua_State *L)
{
  glPopMatrix();
  return 0;
}

/*PushMatrix () -> none*/
static int luagl_push_matrix(lua_State *L)
{
  glPushMatrix();
  return 0;
}

/*ReadPixels (x, y, width, height, format) -> pixelsArray */
static int luagl_read_pixels(lua_State *L)
{
  GLenum format;
  GLfloat *pixels;
  int width, height, size, depth=1;

  format = luagl_get_gl_enum(L, 5);
  depth = luagl_get_depth(format);
  if (depth == 0)
    luaL_argerror(L, 5, "unknown format");

  width = luaL_checkinteger(L, 3);
  height = luaL_checkinteger(L, 4);
  size = width*height*depth;

  pixels = LUAGL_NEW_ARRAY(GLfloat, size);

  glReadPixels(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2),
               width, height, format, GL_FLOAT, pixels);

  luagl_push_arrayf(L, pixels, size);

  LUAGL_DELETE_ARRAY(pixels);

  return 1;
}

/*ReadPixelsRaw (x, y, width, height, format, type, pixels) -> none*/
static int luagl_read_pixels_raw(lua_State *L)
{
  glReadPixels(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2),
               luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), 
               luagl_get_gl_enum(L, 5), luagl_get_gl_enum(L, 6), luagl_checkuserdata(L, 7));
  return 0;
}


/*Rotate (angle, x, y, z) -> none*/
static int luagl_rotate(lua_State *L)
{
  glRotatef(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));
  return 0;
}

/*Scale (x, y, z) -> none*/
static int luagl_scale(lua_State *L)
{
  glScalef(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

/*Scissor (x, y, width, height) -> none*/
static int luagl_scissor(lua_State *L)
{
  glScissor(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3), luaL_checkinteger(L, 4));
  return 0;
}


static int luagl_get_select_buffer(lua_State *L)
{
  GLuint* buffer = (GLuint*)luagl_checkuserdata(L, 1);
  int i = luaL_checkinteger(L,2);
  if (buffer) 
  { 
    int size = (int)buffer[0];
    if ((i<=size) && (i>0)) 
    {
      lua_pushnumber(L,buffer[i]); /*select buffer data begin at index i */
      return 1;
     }
  }
  return 0;
}

static int luagl_free_select_buffer(lua_State *L)
{
  GLuint* buffer = (GLuint*)luagl_checkuserdata(L, 1);
  LUAGL_DELETE_ARRAY(buffer);
  return 0;
}

static int luagl_get_typesize(int type)
{
  switch (type)
  {
  case GL_BYTE:
  case GL_UNSIGNED_BYTE:
    return 1;
  case GL_SHORT:
  case GL_UNSIGNED_SHORT:
    return 2;
  case GL_UNSIGNED_INT:
    return 4;
  case GL_FLOAT:
    return 4;
	}
  return -1;
}

static int luagl_new_data(lua_State *L)
{
  int size = luaL_checkinteger(L, 1);
  void* buffer;
  if (!lua_isnoneornil(L, 2))
  {
    int type = luagl_get_gl_enum(L, 2);
    int typesize = luagl_get_typesize(type);
    if (typesize < 0)
      luaL_argerror(L, 2, "unknown type");
    size *= typesize;
  }
  buffer = LUAGL_NEW_ARRAY(unsigned char, size);
  lua_pushlightuserdata(L, buffer);
  return 1;
}

static int luagl_free_data(lua_State *L)
{
  void* buffer = luagl_checkuserdata(L, 1);
  LUAGL_DELETE_ARRAY(buffer);
  return 0;
}

static int luagl_set_data_value(lua_State *L)
{
  void* buffer = luagl_checkuserdata(L, 1);
  int type = luagl_get_gl_enum(L, 2);
  int index = luaL_checkint(L, 3);

  switch (type)
  {
  case GL_BYTE:
    {
      GLbyte* data = (GLbyte*)buffer;
      data[index] = (GLbyte)luaL_checkinteger(L, 4);
      break;
    }
  case GL_UNSIGNED_BYTE:
    {
      GLubyte* data = (GLubyte*)buffer;
      data[index] = (GLubyte)luaL_checkunsigned(L, 4);
      break;
    }
  case GL_SHORT:
    {
      GLshort* data = (GLshort*)buffer;
      data[index] = (GLshort)luaL_checkinteger(L, 4);
      break;
    }
  case GL_UNSIGNED_SHORT:
    {
      GLushort* data = (GLushort*)buffer;
      data[index] = (GLushort)luaL_checkunsigned(L, 4);
      break;
    }
    case GL_UNSIGNED_INT:
    {
      GLuint* data = (GLuint*)buffer;
      data[index] = (GLuint)luaL_checkunsigned(L, 4);
      break;
    }
  case GL_FLOAT:
    {
      GLfloat* data = (GLfloat*)buffer;
      data[index] = (GLfloat)luaL_checknumber(L, 4);
      break;
    }
  }

  return 0;
}

static int luagl_get_data_value(lua_State *L)
{
  void* buffer = luagl_checkuserdata(L, 1);
  int type = luagl_get_gl_enum(L, 2);
  int index = luaL_checkint(L, 3);

  switch (type)
  {
  case GL_BYTE:
    {
      GLbyte* data = (GLbyte*)buffer;
      lua_pushinteger(L, (lua_Integer)data[index]);
      break;
    }
  case GL_UNSIGNED_BYTE:
    {
      GLubyte* data = (GLubyte*)buffer;
      lua_pushunsigned(L, (lua_Unsigned)data[index]);
      break;
    }
  case GL_SHORT:
    {
      GLshort* data = (GLshort*)buffer;
      lua_pushinteger(L, (lua_Integer)data[index]);
      break;
    }
  case GL_UNSIGNED_SHORT:
    {
      GLushort* data = (GLushort*)buffer;
      lua_pushunsigned(L, (lua_Unsigned)data[index]);
      break;
    }
  case GL_UNSIGNED_INT:
    {
      GLuint* data = (GLuint*)buffer;
      lua_pushunsigned(L, (lua_Unsigned)data[index]);
      break;
    }
  case GL_FLOAT:
    {
      GLfloat* data = (GLfloat*)buffer;
      lua_pushnumber(L, (lua_Number)data[index]);
      break;
    }
    }

  return 1;
}

static int luagl_set_data(lua_State *L)
{
  void* buffer = luagl_checkuserdata(L, 1);
  int type = luagl_get_gl_enum(L, 2);

  switch (type)
  {
  case GL_BYTE:
    {
      GLbyte* data = (GLbyte*)buffer;
      luagl_to_arrayc(L, 3, data);
      break;
    }
  case GL_UNSIGNED_BYTE:
    {
      GLubyte* data = (GLubyte*)buffer;
      luagl_to_arrayuc(L, 3, data);
      break;
    }
  case GL_SHORT:
    {
      GLshort* data = (GLshort*)buffer;
      luagl_to_arrays(L, 3, data);
      break;
    }
  case GL_UNSIGNED_SHORT:
    {
      GLushort* data = (GLushort*)buffer;
      luagl_to_arrayus(L, 3, data);
      break;
    }
 case GL_UNSIGNED_INT:
    {
      GLuint* data = (GLuint*)buffer;
      luagl_to_arrayui(L, 3, data);
      break;
    }
  case GL_FLOAT:
    {
      GLfloat* data = (GLfloat*)buffer;
      luagl_to_arrayf(L, 3, data);
      break;
    }
  }

  return 0;
}

static int luagl_get_data(lua_State *L)
{
  void* buffer = luagl_checkuserdata(L, 1);
  int type = luagl_get_gl_enum(L, 2);
  int size = luaL_checkinteger(L, 3);

  switch (type)
  {
  case GL_BYTE:
    {
      GLbyte* data = (GLbyte*)buffer;
      luagl_push_arrayc(L, data, size);
      break;
    }
  case GL_UNSIGNED_BYTE:
    {
      GLubyte* data = (GLubyte*)buffer;
      luagl_push_arrayuc(L, data, size);
      break;
    }
  case GL_SHORT:
    {
      GLshort* data = (GLshort*)buffer;
      luagl_push_arrays(L, data, size);
      break;
    }
  case GL_UNSIGNED_SHORT:
    {
      GLushort* data = (GLushort*)buffer;
      luagl_push_arrayus(L, data, size);
      break;
    }
  case GL_UNSIGNED_INT:
    {
      GLuint* data = (GLuint*)buffer;
      luagl_push_arrayui(L, data, size);
      break;
    }
  case GL_FLOAT:
    {
      GLfloat* data = (GLfloat*)buffer;
      luagl_push_arrayf(L, data, size);
      break;
    }
  }

  return 1;
}

/*ShadeModel (mode) -> none*/
static int luagl_shade_model(lua_State *L)
{
  glShadeModel(luagl_get_gl_enum(L, 1));
  return 0;
}

/*StencilFunc (func, ref, mask) -> none*/
static int luagl_stencil_func(lua_State *L)
{
  if (lua_type(L,3) == LUA_TSTRING)
    glStencilFunc(luagl_get_gl_enum(L, 1), luaL_checkinteger(L, 2), luagl_str2mask(luaL_checkstring(L, 3)));
  else 
    glStencilFunc(luagl_get_gl_enum(L, 1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3));
  return 0;
}

/*StencilMask (mask) -> none*/
static int luagl_stencil_mask(lua_State *L)
{
  if(lua_type(L,1) == LUA_TSTRING)
    glStencilMask(luagl_str2mask(luaL_checkstring(L, 1)));
  else 
    glStencilMask(luaL_checkinteger(L, 1));
  return 0;
}

/*StencilOp (fail, zfail, zpass) -> none*/
static int luagl_stencil_op(lua_State *L)
{
  glStencilOp(luagl_get_gl_enum(L, 1), luagl_get_gl_enum(L, 2), luagl_get_gl_enum(L, 3));
  return 0;
}



/*TexCoordPointer(vArray) -> none*/
static int luagl_tex_coord_pointer(lua_State *L)
{
  GLint size;
  static GLfloat *parray = NULL;

  LUAGL_DELETE_ARRAY(parray);

  if(lua_isnil(L,1))
    return 0;

  if (lua_isnumber(L, 2))
  {
    size = luaL_checkinteger(L, 2);
    luagl_get_arrayf(L, 1, &parray);
  }
  else 
  {
    int h = luagl_get_array2f(L, 1, &parray, &size);
    if (h==-1)
      luaL_argerror(L, 1, "must be a table of tables");
  }

  glTexCoordPointer(size, GL_FLOAT, 0, parray);

  return 0;
}

/*TexEnv (pname, param) -> none
  TexEnv (pname, paramsArray) -> none*/
static int luagl_tex_env(lua_State *L)
{
  if(lua_istable(L, 2))
  {
    GLfloat *param;
    luagl_get_arrayf(L, 2, &param);

    glTexEnvfv(GL_TEXTURE_ENV, luagl_get_gl_enum(L, 1), (GLfloat *)param);

    LUAGL_DELETE_ARRAY(param);
  }
  else if(lua_isnumber(L, 2))
    glTexEnvf(GL_TEXTURE_ENV, luagl_get_gl_enum(L, 1), (GLfloat)luaL_checknumber(L, 2));
  else 
    glTexEnvi(GL_TEXTURE_ENV, luagl_get_gl_enum(L, 1), luagl_get_gl_enum(L, 2));

  return 0;
}


/*TexImage(level, internalformat, format, pixels) -> none*/
static int luagl_tex_image(lua_State *L)
{
  GLenum format;
  GLfloat *pixels;
  GLsizei width, height;
  int iformat;  /* same as "components" in glu if integer */
  int index, depth, border;

  if(lua_isnumber(L, 1) && lua_istable(L, 2))
  {
    /* undocumented parameter passing, 
       so it can be compatible with a texture created for glu.Build2DMipmaps */
    lua_getfield(L, 2, "components");
    iformat = luaL_checkinteger(L, -1);
    lua_remove(L, -1);

    lua_getfield(L, 2, "format");
    format = luagl_get_gl_enum(L, -1);
    lua_remove(L, -1);

    index = 2;
  }
  else
  {
    format = luagl_get_gl_enum(L, 3);
    if (lua_isnumber(L, 2))
      iformat = luaL_checkinteger(L, 2);
    else
      iformat = luagl_get_gl_enum(L, 2);

    index = 4;
  }

  depth = luagl_get_depth(format);
  if (depth == 0)
    luaL_argerror(L, index-1, "unknown format");

  border = luaL_optint(L, index+1, 0);

  height = luagl_get_array2f(L, index, &pixels, &width);
  if (height != -1)
  {
    glTexImage2D(GL_TEXTURE_2D, luaL_checkinteger(L, 1),
                 iformat, width/depth, height, border, format, GL_FLOAT, pixels);
  }
  
  LUAGL_DELETE_ARRAY(pixels);
  return 0;
}

/* TexImage2D(level, depth, width, height, border, format, type, pixels) -> none*/
static int luagl_tex_image_2d(lua_State *L)
{
  glTexImage2D(GL_TEXTURE_2D, luaL_checkinteger(L, 1),
              luaL_checkinteger(L, 2), (GLsizei)luaL_checkinteger(L, 3), 
              (GLsizei)luaL_checkinteger(L, 4), luaL_checkinteger(L, 5), 
              luagl_get_gl_enum(L, 6), luagl_get_gl_enum(L, 7), luagl_checkuserdata(L, 8));
  return 0;
}


/*TexSubImage (level, format, pixels, xoffset) -> none
  TexSubImage (level, format, pixels, xoffset, yoffset) -> none*/
static int luagl_tex_sub_image(lua_State *L)
{
  GLenum format;
  GLfloat *pixels;
  GLsizei width, height;
  int depth;

  format = luagl_get_gl_enum(L, 2);
  depth = luagl_get_depth(format);
  if (depth == 0)
    luaL_argerror(L, 2, "unknown format");

  height = luagl_get_array2f(L, 3, &pixels, &width);
  if(height != -1)
  {
    glTexSubImage2D(GL_TEXTURE_2D, luaL_checkinteger(L, 1), luaL_checkinteger(L, 4),
                    luaL_checkinteger(L, 5), width/depth, height, format, GL_FLOAT, pixels);
  }
  

  LUAGL_DELETE_ARRAY(pixels);
  return 0;
}

/* TexSubImage2D (level, xoffset, yoffset, width, height, format, type, pixels) -> none*/
static int luagl_tex_sub_image_2d(lua_State *L)
{
  glTexSubImage2D(GL_TEXTURE_2D, luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), 
                  luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), luaL_checkinteger(L, 5), 
                  luagl_get_gl_enum(L, 6), luagl_get_gl_enum(L, 7), luagl_checkuserdata(L, 8));
  return 0;
}


/*TexParameter (target, pname, param) -> none
TexParameter (target, pname, paramsArray) -> none*/
static int luagl_tex_parameter(lua_State *L)
{
  GLenum e1, e2;

  e1 = luagl_get_gl_enum(L, 1);
  e2 = luagl_get_gl_enum(L, 2);

  if(lua_istable(L, 3))
  {
    GLfloat *param;
    luagl_get_arrayf(L, 3, &param);

    glTexParameterfv(e1, e2, (GLfloat *)param);

    LUAGL_DELETE_ARRAY(param);
  }
  else if(lua_isnumber(L, 3))
    glTexParameterf(e1, e2, (GLfloat)luaL_checknumber(L, 3));
  else 
    glTexParameteri(e1, e2, luagl_get_gl_enum(L, 3));

  return 0;
}

/*Translate (x, y, z) -> none*/
static int luagl_translate(lua_State *L)
{
  glTranslatef(luaL_checknumber(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

/*VertexPointer (vertexArray) -> none*/
static int luagl_vertex_pointer(lua_State *L)
{
  GLint size;
  static GLfloat *parray = NULL;

  LUAGL_DELETE_ARRAY(parray);

  if(lua_isnil(L,1))
    return 0;

  if (lua_isnumber(L, 2))
  {
    size = luaL_checkinteger(L, 2);
    luagl_get_arrayf(L, 1, &parray);
  }
  else 
  {
    int h = luagl_get_array2f(L, 1, &parray, &size);
    if (h==-1)
      luaL_argerror(L, 1, "must be a table of tables");
  }

  glVertexPointer(size, GL_FLOAT, 0, parray);

  return 0;
}

/*Viewport (x, y, width, height) -> none*/
static int luagl_viewport(lua_State *L)
{
  glViewport(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), 
             luaL_checkinteger(L, 3), luaL_checkinteger(L, 4));
  return 0;
}

static const luaL_Reg luagl_lib[] = {

  {"glAlphaFunc", luagl_alpha_func},
  {"glBindTexture", luagl_bind_texture},
  {"glBlendFunc", luagl_blend_func},
  {"glClear", luagl_clear},
  {"glClearColor", luagl_clear_color},
  {"glClearDepth", luagl_clear_depth},
  {"glClearStencil", luagl_clear_stencil},
  {"glClipPlane", luagl_clip_plane},
  {"glColor", luagl_color},
  {"glColorMask", luagl_color_mask},
  {"glColorPointer", luagl_color_pointer},
  {"glCopyTexImage", luagl_copy_tex_image},
  {"glCopyTexSubImage", luagl_copy_tex_sub_image},
  {"glCullFace",luagl_cull_face},
  {"glDeleteTextures",luagl_delete_textures},
  {"glDepthFunc",luagl_depth_func},
  {"glDepthMask",luagl_depth_mask},
  {"glDisable",luagl_disable},
  {"glDisableClientState",luagl_disable_client_state},
  {"glDrawArrays",luagl_draw_arrays},
  {"glDrawElements", luagl_draw_elements},
  {"glEnable", luagl_enable},
  {"glEnableClientState", luagl_enable_client_state},
  {"glFinish", luagl_finish},
  {"glFlush", luagl_flush},
  {"glFog", luagl_fog},
  {"glFrontFace", luagl_front_face},
  {"glFrustum", luagl_frustum},
  {"glGenTextures", luagl_gen_textures},
  {"glGet", luagl_get},
  {"glGetArray", luagl_get_array},
  {"glGetConst", luagl_get_const},
  {"glGetClipPlane", luagl_get_clip_plane},
  {"glGetError", luagl_get_error},
  {"glGetLight", luagl_get_light},
  {"glGetMaterial", luagl_get_material},
  {"glGetPointer", luagl_get_pointer},
  {"glGetString", luagl_get_string},
  {"glGetTexEnv", luagl_get_tex_env},
  {"glGetTexParameter", luagl_get_tex_parameter},
  {"glHint", luagl_hint},
  {"glIsEnabled", luagl_is_enabled},
  {"glIsTexture", luagl_is_texture},
  {"glLight", luagl_light},
  {"glLightModel", luagl_light_model},
  {"glLineWidth", luagl_line_width},
  {"glLoadIdentity", luagl_load_identity},
  {"glLoadMatrix", luagl_load_matrix},
  {"glLogicOp", luagl_logic_op},
  {"glMaterial", luagl_material},
  {"glMatrixMode", luagl_matrix_mode},
  {"glMultMatrix", luagl_mult_matrix},
  {"glNormal", luagl_normal},
  {"glNormalPointer", luagl_normal_pointer},
  {"glOrtho", luagl_ortho},
  {"glPointSize", luagl_point_size},
  {"glPolygonOffset", luagl_polygon_offset},
  {"glPopMatrix", luagl_pop_matrix},
  {"glPushMatrix", luagl_push_matrix},
  {"glReadPixels", luagl_read_pixels},
  {"glReadPixelsRaw", luagl_read_pixels_raw},
  {"glRotate", luagl_rotate},
  {"glScale", luagl_scale},
  {"glScissor", luagl_scissor},
  {"glShadeModel", luagl_shade_model},
  {"glStencilFunc", luagl_stencil_func},
  {"glStencilMask", luagl_stencil_mask},
  {"glStencilOp", luagl_stencil_op},
  {"glTexCoordPointer", luagl_tex_coord_pointer},
  {"glTexEnv", luagl_tex_env},
  {"glTexImage", luagl_tex_image},
  {"glTexImage2D", luagl_tex_image_2d},
  {"glTexSubImage", luagl_tex_sub_image},
  {"glTexSubImage2D", luagl_tex_sub_image_2d},
  {"glTexParameter", luagl_tex_parameter},
  {"glTranslate", luagl_translate},
  {"glVertexPointer", luagl_vertex_pointer},
  {"glViewport", luagl_viewport},

  {"glGetSelectBuffer", luagl_get_select_buffer},
  {"glFreeSelectBuffer", luagl_free_select_buffer},
  {"glNewData", luagl_new_data},
  {"glFreeData", luagl_free_data},
  {"glSetDataValue", luagl_set_data_value},
  {"glGetDataValue", luagl_get_data_value},
  {"glSetData", luagl_set_data},
  {"glGetData", luagl_get_data},
  {NULL, NULL}
};

int luaopen_gl(lua_State *L) 
{
  luaL_newlib(L, luagl_lib);

  luagl_open_const(L);

  lua_pushstring(L, "_VERSION");
  lua_pushstring(L, LUAGL_VERSION);
  lua_settable(L,-3);

  return 1;
}
