#include <lua.h>
#include <lauxlib.h>

#include <time.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
#include <sys/stat.h>


static void dir_meta(lua_State *L);
static int pushdir(lua_State * L, DIR* p);

//strcmp memcmp
static int ld_open(lua_State *L) {
	DIR* self_dir = opendir(luaL_checkstring(L,1));
	return pushdir(L,self_dir);
}

static int ld_close(lua_State *L) {
	DIR *d;
	d=*(DIR**)luaL_checkudata(L,1,"dir");
	int i=closedir(d);
	lua_pushinteger(L,i);
	return 1;
}


int pushdir(lua_State * L, DIR* p)
{
	DIR **userData;
	userData = (DIR **) lua_newuserdata(L, sizeof(p));
	*userData = p;
	dir_meta(L);
	return 1;
}

void
dir_meta(lua_State *L) {
	if (luaL_newmetatable(L, "dir")) {
		luaL_Reg l[] = {
			{ "close", ld_close },
			{ NULL, NULL },
		};
		luaL_newlib(L,l);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
}

int
luaopen_dir(lua_State *L) {
	luaL_checkversion(L);
	
	luaL_Reg l[] = {
		{ "open", ld_open },
		{ NULL,  NULL },
	};

	luaL_newlib(L,l);
	
	return 1;
}

