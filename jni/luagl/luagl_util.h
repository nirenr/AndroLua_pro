#ifndef __LUAGL_UTIL_H__
#define __LUAGL_UTIL_H__


#if LUA_VERSION_NUM > 501
#define luagl_getn(L,i)          ((int)lua_rawlen(L, i))
#else
#define luagl_getn(L,i)          ((int)lua_objlen(L, i))
#endif

lua_Number  luagl_tonumber(lua_State *L, int idx);
lua_Integer luagl_tointeger(lua_State *L, int idx);

int luagl_checkboolean (lua_State *L, int narg);
void* luagl_checkuserdata (lua_State *L, int narg);

/* returns an parray with given type and size */
#define LUAGL_NEW_ARRAY(_type, _size) ( (_type *)malloc((_size) * sizeof(_type)) )

/* frees the space for the given parray, must be called together with LUAGL_NEW_ARRAY */
#define LUAGL_DELETE_ARRAY(_parray) { if(_parray) {free(_parray);} }

/* Returns an array in a lua table. */
void luagl_push_arrayb(lua_State *L, unsigned char* parray, int size);  /* boolean */
void luagl_push_arrayuc(lua_State *L, unsigned char* parray, int size);
void luagl_push_arrayc(lua_State *L, char* parray, int size);
void luagl_push_arrayus(lua_State *L, unsigned short* parray, int size);
void luagl_push_arrays(lua_State *L, short* parray, int size);
void luagl_push_arrayui(lua_State *L, unsigned int* parray, int size);
void luagl_push_arrayi(lua_State *L, int* parray, int size);
void luagl_push_arrayf(lua_State *L, float* parray, int size);
//void luagl_push_arrayd(lua_State *L, double* parray, int size);

void luagl_push_array2f(lua_State *L, float* parray, int height, int width);

void luagl_to_arrayuc(lua_State *L, int index, unsigned char *parray);
void luagl_to_arrayc(lua_State *L, int index, char *parray);
void luagl_to_arrayus(lua_State *L, int index, unsigned short *parray);
void luagl_to_arrays(lua_State *L, int index, short *parray);
void luagl_to_arrayui(lua_State *L, int index, unsigned int *parray);
void luagl_to_arrayi(lua_State *L, int index, int *parray);
//void luagl_to_arrayd(lua_State *L, int index, double *parray);
void luagl_to_arrayf(lua_State *L, int index, float *parray);

/* Gets an parray from a lua table, store it in 'parray' and returns the no. of elems of the parray
   index refers to where the table is in stack. */
int luagl_get_arrayuc(lua_State *L, int index, unsigned char **parray);
int luagl_get_arrayb(lua_State *L, int index, unsigned char **parray);  /* boolean */
int luagl_get_arrayi(lua_State *L, int index, int **parray);
int luagl_get_arrayui(lua_State *L, int index, unsigned int **parray);
//int luagl_get_arrayd(lua_State *L, int index, double **parray);
int luagl_get_arrayf(lua_State *L, int index, float **parray);

int luagl_get_array2uc(lua_State *L, int index, unsigned char **parray, int *size);
//int luagl_get_array2d(lua_State *L, int index, double **parray, int *size);
int luagl_get_array2f(lua_State *L, int index, float **parray, int *size);

int luagl_str2mask(const char *str);
const char *luagl_mask2str(int mask);

#endif
