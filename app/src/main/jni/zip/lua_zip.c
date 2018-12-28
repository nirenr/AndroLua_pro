#include <lauxlib.h>
#include <lua.h>
#include <zip.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#if LUA_VERSION_NUM > 501
#define lua_objlen lua_rawlen
#endif

#if LUA_VERSION_NUM > 501
#define lua_equal(L, idx1, idx2) lua_compare((L), (idx1), (idx2), LUA_OPEQ)
#endif

#if LUA_VERSION_NUM > 501
#define luaL_register(L,_,funcs) luaL_setfuncs((L),funcs,0)
#endif

#define ARCHIVE_MT      "zip{archive}"
#define ARCHIVE_FILE_MT "zip{archive.file}"
#define WEAK_MT         "zip{weak}"

#define check_archive_file(L, narg)                                   \
    ((struct zip_file**)luaL_checkudata((L), (narg), ARCHIVE_FILE_MT))

#define absindex(L,i) ((i)>0?(i):lua_gettop(L)+(i)+1)

static int S_archive_gc(lua_State* L);
static int S_archive_get_num_files(lua_State* L);

static void stackdump(lua_State* l)
{
    int i;
    int top = lua_gettop(l);

    for (i = 1; i <= top; i++)
    {  /* repeat for each level */
        int t = lua_type(l, i);
        switch (t) {
        case LUA_TSTRING:  /* strings */
            fprintf(stderr, "\tstring: '%s'\n", lua_tostring(l, i));
            break;
        case LUA_TBOOLEAN:  /* booleans */
            fprintf(stderr, "\tboolean %s\n",lua_toboolean(l, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:  /* numbers */
            fprintf(stderr, "\tnumber: %g\n", lua_tonumber(l, i));
            break;
        default:  /* other values */
            fprintf(stderr, "\t%s\n", lua_typename(l, t));
            break;
        }
    }
}

static struct zip** check_archive(lua_State* L, int narg) {
    luaL_checktype(L, narg, LUA_TUSERDATA);
    if ( !lua_getmetatable(L, narg) ) {
        luaL_argerror(L, narg, "zip{archive} expected (missing metatable)");
        return NULL;
    }
    lua_getfield(L, -1, "__index");
    luaL_getmetatable(L, ARCHIVE_MT);
    if ( !lua_equal(L, -1, -2) ) {
        luaL_argerror(L, narg, "zip{archive} expected (__index field incorrect)");
        return NULL;
    }
    lua_pop(L, 3);
    return (struct zip**)lua_touserdata(L, narg);
}

/* If zip_error is non-zero, then push an appropriate error message
 * onto the top of the Lua stack and return zip_error.  Otherwise,
 * just return 0.
 */
static int S_push_error(lua_State* L, int zip_error, int sys_error) {
    char buff[1024];
    if ( 0 == zip_error ) return 0;

    int len = zip_error_to_str(buff, sizeof(buff), zip_error, sys_error);
    if ( len >= sizeof(buff) ) len = sizeof(buff)-1;
    lua_pushlstring(L, buff, len);

    return zip_error;
}

static int S_archive_open(lua_State* L) {
    const char*  path  = luaL_checkstring(L, 1);
    int          flags = (lua_gettop(L) < 2) ? 0 : luaL_checkint(L, 2);
    struct zip** ar    = (struct zip**)lua_newuserdata(L, sizeof(struct zip*));
    int          err   = 0;

    *ar = zip_open(path, flags, &err);

    if ( ! *ar ) {
        assert(err);
        S_push_error(L, err, errno);
        lua_pushnil(L);
        lua_insert(L, -2);
        return 2;
    }

    lua_newtable(L);

    luaL_getmetatable(L, ARCHIVE_MT);
    assert(!lua_isnil(L, -1)/* ARCHIVE_MT found? */);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, S_archive_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, S_archive_get_num_files);
    lua_setfield(L, -2, "__len");

    /* Each archive has a weak table of objects that are invalidated
     * when the archive is closed or garbage collected.
     */
    lua_newtable(L);
    luaL_getmetatable(L, WEAK_MT);
    assert(!lua_isnil(L, -1)/* WEAK_MT found? */);
    lua_setmetatable(L, -2);
    lua_setfield(L, -2, "refs");

    lua_setmetatable(L, -2);

    return 1;
}

/* Push the refs weak table onto the stack for archive at index ar_idx.
 */
static void S_get_refs(lua_State* L, int ar_idx) {
    int ok = lua_getmetatable(L, ar_idx);
    assert(ok /* ar_idx has metatable */);
    lua_getfield(L, -1, "refs");
    assert(!lua_isnil(L, -1)/* getmetatable(ar_idx).refs must exist! */);
    lua_remove(L, -2);
}

/* Invalidate all "weak" references.  This should be done just before
 * zip_close() is called.  Invalidation occurs by calling __gc
 * metamethod.
 */
static void S_archive_gc_refs(lua_State* L, int ar_idx) {
    ar_idx = absindex(L, ar_idx);
    S_get_refs(L, ar_idx);

    /* NULL this archive to prevent infinite recursion
     */
    *((struct zip**)lua_touserdata(L, ar_idx)) = NULL;

    lua_pushnil(L);
    while ( lua_next(L, -2) != 0 ) {
        /* TODO: Exceptions when calling __gc meta method should be
         * handled, otherwise we get a memory leak.
         */
        if ( luaL_callmeta(L, -2, "__gc") ) {
            lua_pop(L, 2);
        } else {
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1); /* Pop the refs */
}

/* Adds a reference from the archive at ar_idx to the object at
 * obj_idx.  This reference will be a "weak" reference if is_weak is
 * true, otherwise it will be a normal reference that prevents the
 * value from being GC'ed.
 *
 * We need to keep these references around so we can assert that the
 * lifetime of "child" objects is always shorter than the lifetime of
 * the "parent" archive object.  We also need to assert that any zip
 * source has a lifetime that lasts at least until the zip_close()
 * function is called.
 */
static void S_archive_add_ref(lua_State* L, int is_weak, int ar_idx, int obj_idx) {
    obj_idx = absindex(L, obj_idx);
    S_get_refs(L, ar_idx);

    if ( is_weak ) {
        lua_pushvalue(L, obj_idx);
        lua_pushboolean(L, 1);
        lua_rawset(L, -3);
    } else {
        lua_pushvalue(L, obj_idx);
        lua_rawseti(L, -2, lua_objlen(L, -2)+1);
    }

    lua_pop(L, 1); /* Pop the refs */
}

/* Explicitly close the archive, throwing an error if there are any
 * problems.
 */
static int S_archive_close(lua_State* L) {
    struct zip*  ar  = *check_archive(L, 1);
    int          err;

    if ( ! ar ) return 0;

    S_archive_gc_refs(L, 1);

    err = zip_close(ar);
    if ( err != 0 ) {
        S_push_error(L, zip_error_code_zip(zip_get_error(ar)), errno);
        lua_error(L);
    }

    return 0;
}

/* Try to revert all changes and close the archive since the archive
 * was not explicitly closed.
 */
static int S_archive_gc(lua_State* L) {
    struct zip* ar = *check_archive(L, 1);

    if ( ! ar ) return 0;

    S_archive_gc_refs(L, 1);

    zip_unchange_all(ar);
    zip_close(ar);

    return 0;
}

static int S_archive_get_num_files(lua_State* L) {
    struct zip** ar = check_archive(L, 1);

    if ( ! *ar ) return 0;

    lua_pushinteger(L, zip_get_num_files(*ar));

    return 1;
}

static int S_archive_name_locate(lua_State* L) {
    struct zip** ar    = check_archive(L, 1);
    const char*  fname = luaL_checkstring(L, 2);
    int          flags = (lua_gettop(L) < 3) ? 0 : luaL_checkint(L, 3);
    int          idx;

    if ( ! *ar ) return 0;

    idx = zip_name_locate(*ar, fname, flags);

    if ( idx < 0 ) {
        lua_pushnil(L);
        lua_pushstring(L, zip_strerror(*ar));
        return 2;
    }

    lua_pushinteger(L, idx+1);
    return 1;
}

static int S_archive_stat(lua_State* L) {
    struct zip**    ar        = check_archive(L, 1);
    const char*     path      = (lua_isnumber(L, 2)) ? NULL : luaL_checkstring(L, 2);
    int             path_idx  = (lua_isnumber(L, 2)) ? luaL_checkint(L, 2)-1 : -1;
    int             flags     = (lua_gettop(L) < 3)  ? 0    : luaL_checkint(L, 3);
    struct zip_stat stat;
    int             result;

    if ( ! *ar ) return 0;

    if ( NULL == path ) {
        result = zip_stat_index(*ar, path_idx, flags, &stat);
    } else {
        result = zip_stat(*ar, path, flags, &stat);
    }

    if ( result != 0 ) {
        lua_pushnil(L);
        lua_pushstring(L, zip_strerror(*ar));
        return 2;
    }

    lua_createtable(L, 0, 8);

    lua_pushstring(L, stat.name);
    lua_setfield(L, -2, "name");

    lua_pushinteger(L, stat.index+1);
    lua_setfield(L, -2, "index");

    lua_pushnumber(L, stat.crc);
    lua_setfield(L, -2, "crc");

    lua_pushnumber(L, stat.size);
    lua_setfield(L, -2, "size");

    lua_pushnumber(L, stat.mtime);
    lua_setfield(L, -2, "mtime");

    lua_pushnumber(L, stat.comp_size);
    lua_setfield(L, -2, "comp_size");

    lua_pushnumber(L, stat.comp_method);
    lua_setfield(L, -2, "comp_method");

    lua_pushnumber(L, stat.encryption_method);
    lua_setfield(L, -2, "encryption_method");

    return 1;
}

static int S_archive_get_name(lua_State* L) {
    struct zip** ar        = check_archive(L, 1);
    int          path_idx  = luaL_checkint(L, 2)-1;
    int          flags     = (lua_gettop(L) < 3)  ? 0    : luaL_checkint(L, 3);
    const char*  name;

    if ( ! *ar ) return 0;

    name = zip_get_name(*ar, path_idx, flags);

    if ( NULL == name ) {
        lua_pushnil(L);
        lua_pushstring(L, zip_strerror(*ar));
        return 2;
    }

    lua_pushstring(L, name);
    return 1;
}

static int S_archive_get_archive_comment(lua_State* L) {
    struct zip** ar        = check_archive(L, 1);
    int          flags     = (lua_gettop(L) < 2)  ? 0    : luaL_checkint(L, 2);
    const char*  comment;
    int          comment_len;

    if ( ! *ar ) return 0;

    comment = zip_get_archive_comment(*ar, &comment_len, flags);

    if ( NULL == comment ) {
        lua_pushnil(L);
    } else {
        lua_pushlstring(L, comment, comment_len);
    }
    return 1;
}

static int S_archive_set_archive_comment(lua_State* L) {
    struct zip** ar          = check_archive(L, 1);
    size_t       comment_len = 0;
    const char*  comment     = lua_isnil(L, 2) ? NULL : luaL_checklstring(L, 2, &comment_len);

    if ( ! *ar ) return 0;

    if ( 0 != zip_set_archive_comment(*ar, comment, comment_len) ) {
        lua_pushstring(L, zip_strerror(*ar));
        lua_error(L);
    }

    return 0;
}

static int S_archive_get_file_comment(lua_State* L) {
    struct zip** ar        = check_archive(L, 1);
    int          path_idx  = luaL_checkint(L, 2)-1;
    int          flags     = (lua_gettop(L) < 3)  ? 0    : luaL_checkint(L, 3);
    const char*  comment;
    int          comment_len;

    if ( ! *ar ) return 0;

    comment = zip_get_file_comment(*ar, path_idx, &comment_len, flags);

    if ( NULL == comment ) {
        lua_pushnil(L);
    } else {
        lua_pushlstring(L, comment, comment_len);
    }
    return 1;
}

static int S_archive_set_file_comment(lua_State* L) {
    struct zip** ar          = check_archive(L, 1);
    int          path_idx    = luaL_checkint(L, 2)-1;
    size_t       comment_len = 0;
    const char*  comment     = lua_isnil(L, 3) ? NULL : luaL_checklstring(L, 3, &comment_len);

    if ( ! *ar ) return 0;

    if ( 0 != zip_set_file_comment(*ar, path_idx, comment, comment_len) ) {
        lua_pushstring(L, zip_strerror(*ar));
        lua_error(L);
    }

    return 0;
}

static int S_archive_add_dir(lua_State* L) {
    struct zip**        ar   = check_archive(L, 1);
    const char*         path = luaL_checkstring(L, 2);
    int                 idx;

    if ( ! *ar ) return 0;


    idx = zip_add_dir(*ar, path) + 1;

    if ( 0 == idx ) {
        lua_pushstring(L, zip_strerror(*ar));
        lua_error(L);
        return 0;
    }

    lua_pushinteger(L, idx);

    return 1;
}

static struct zip_source* S_create_source_string(lua_State* L, struct zip* ar) {
    size_t             len;
    const char*        str = luaL_checklstring(L, 4, &len);
    struct zip_source* src = zip_source_buffer(ar, str, len, 0);

    if ( NULL != src ) return src;

    S_archive_add_ref(L, 0, 1, 4);

    lua_pushstring(L, zip_strerror(ar));
    lua_error(L);
    return NULL;
}

static struct zip_source* S_create_source_file(lua_State* L, struct zip* ar) {
    const char*        fname = luaL_checkstring(L, 4);
    int                start = lua_gettop(L) < 5 ? 0  : luaL_checkint(L, 5);
    int                len   = lua_gettop(L) < 6 ? -1 : luaL_checkint(L, 6);
    struct zip_source* src   = zip_source_file(ar, fname, start, len);

    if ( NULL != src ) return src;

    lua_pushstring(L, zip_strerror(ar));
    lua_error(L);
    return NULL;
}

static struct zip_source* S_create_source_zip(lua_State* L, struct zip* ar) {
    struct zip**       other_ar = check_archive(L, 4);
    int                file_idx = luaL_checkint(L, 5);
    int                flags    = lua_gettop(L) < 6 ? 0  : luaL_checkint(L, 6);
    int                start    = lua_gettop(L) < 7 ? 0  : luaL_checkint(L, 7);
    int                len      = lua_gettop(L) < 8 ? -1 : luaL_checkint(L, 8);
    struct zip_source* src      = NULL;

    if ( ! *other_ar ) return NULL;

    /* Check for circular reference */
    S_get_refs(L, 1);
    lua_pushvalue(L, 4);
    lua_rawget(L, -2);
    if ( ! lua_isnil(L, -1) ) {
        lua_pushliteral(L, "Circular reference of zip sources is not allowed");
        lua_error(L);
    }
    lua_pop(L, 1);

    /* We store a strong reference to prevent the "other" archive
     * from getting prematurely gc'ed, we also have the other archive
     * store a weak reference so the __gc metamethod will be called
     * if the "other" archive is closed before we are closed.
     */
    S_archive_add_ref(L, 0, 1, 4);
    S_archive_add_ref(L, 1, 4, 1);

    src = zip_source_zip(ar, *other_ar, file_idx-1, flags, start, len);
    if ( NULL != src ) return src;

    lua_pushstring(L, zip_strerror(ar));
    lua_error(L);
    return NULL;
}

typedef struct zip_source* (S_src_t)(lua_State*, struct zip*);

/* Dispatch to the proper function based on the type string.
 */
static struct zip_source* S_create_source(lua_State* L, struct zip* ar) {
    static const char* types[] = {
        "file",
        "string",
        "zip",
    };
    static S_src_t* fns[] = {
        &S_create_source_file,
        &S_create_source_string,
        &S_create_source_zip,
    };
    if ( NULL == ar ) return NULL;
    return fns[luaL_checkoption(L, 3, NULL, types)](L, ar);
}

static int S_archive_replace(lua_State* L) {
    struct zip**        ar   = check_archive(L, 1);
    int                 idx  = luaL_checkinteger(L, 2);
    struct zip_source*  src  = S_create_source(L, *ar);

    if ( ! *ar ) return 0;

    idx = zip_replace(*ar, idx-1, src) + 1;

    if ( 0 == idx ) {
        zip_source_free(src);
        lua_pushstring(L, zip_strerror(*ar));
        lua_error(L);
    }

    S_archive_add_ref(L, 0, 1, 4);

    lua_pushinteger(L, idx);

    return 1;
}

static int S_archive_rename(lua_State* L) {
    struct zip**        ar        = check_archive(L, 1);
    const char*         path      = (lua_isnumber(L, 2)) ? NULL : luaL_checkstring(L, 2);
    int                 path_idx  = (lua_isnumber(L, 2)) ? luaL_checkint(L, 2)-1 : -1;
    const char*         new_path  = luaL_checkstring(L, 3);

    if ( ! *ar ) return 0;

    if ( NULL != path ) {
        path_idx = zip_name_locate(*ar, path, 0);
        if ( path_idx < 0 ) {
            lua_pushfstring(L, "%s '%s'", zip_strerror(*ar), path);
            lua_error(L);
        }
    }
    if ( zip_rename(*ar, path_idx, new_path) != 0 ) {
        lua_pushstring(L, zip_strerror(*ar));
        lua_error(L);
    }
    return 0;
}

static int S_archive_delete(lua_State* L) {
    struct zip**        ar        = check_archive(L, 1);
    const char*         path      = (lua_isnumber(L, 2)) ? NULL : luaL_checkstring(L, 2);
    int                 path_idx  = (lua_isnumber(L, 2)) ? luaL_checkint(L, 2)-1 : -1;

    if ( ! *ar ) return 0;

    if ( NULL != path ) {
        path_idx = zip_name_locate(*ar, path, 0);
        if ( path_idx < 0 ) {
            lua_pushfstring(L, "%s '%s'", zip_strerror(*ar), path);
            lua_error(L);
        }
    }
    if ( zip_delete(*ar, path_idx) != 0 ) {
        lua_pushstring(L, zip_strerror(*ar));
        lua_error(L);
    }
    return 0;
}

static int S_archive_add(lua_State* L) {
    struct zip**        ar   = check_archive(L, 1);
    const char*         path = luaL_checkstring(L, 2);
    struct zip_source*  src  = S_create_source(L, *ar);
    int                 idx;

    if ( ! *ar ) return 0;

    idx = zip_add(*ar, path, src) + 1;

    if ( 0 == idx ) {
        zip_source_free(src);
        lua_pushstring(L, zip_strerror(*ar));
        lua_error(L);
    }

    S_archive_add_ref(L, 0, 1, 4);

    lua_pushinteger(L, idx);

    return 1;
}

static int S_archive_file_open(lua_State* L) {
    struct zip** ar        = check_archive(L, 1);
    const char*  path      = (lua_isnumber(L, 2)) ? NULL : luaL_checkstring(L, 2);
    int          path_idx  = (lua_isnumber(L, 2)) ? luaL_checkint(L, 2)-1 : -1;
    int          flags     = (lua_gettop(L) < 3)  ? 0    : luaL_checkint(L, 3);
    struct zip_file** file = (struct zip_file**)
        lua_newuserdata(L, sizeof(struct zip_file*));

    *file = NULL;

    if ( ! *ar ) return 0;

    if ( NULL == path ) {
        *file = zip_fopen_index(*ar, path_idx, flags);
    } else {
        *file = zip_fopen(*ar, path, flags);
    }

    if ( ! *file ) {
        lua_pushnil(L);
        lua_pushstring(L, zip_strerror(*ar));
        return 2;
    }

    luaL_getmetatable(L, ARCHIVE_FILE_MT);
    assert(!lua_isnil(L, -1)/* ARCHIVE_FILE_MT found? */);

    lua_setmetatable(L, -2);

    S_archive_add_ref(L, 1, 1, -1);

    return 1;
}

static int S_archive_file_close(lua_State* L) {
    struct zip_file** file = check_archive_file(L, 1);
    int err;

    if ( ! *file ) return 0;

    err = zip_fclose(*file);
    *file = NULL;

    if ( err ) {
        S_push_error(L, err, errno);
        lua_error(L);
    }

    return 0;
}

static int S_archive_file_gc(lua_State* L) {
    struct zip_file** file = check_archive_file(L, 1);

    if ( ! *file ) return 0;

    zip_fclose(*file);
    *file = NULL;

    return 0;
}

static int S_archive_file_read(lua_State* L) {
    struct zip_file** file = check_archive_file(L, 1);
    int               len  = luaL_checkint(L, 2);
    char*             buff;

    if ( len <= 0 ) luaL_argerror(L, 2, "Must be > 0");

    if ( ! *file ) return 0;

    buff = (char*)lua_newuserdata(L, len);

    len = zip_fread(*file, buff, len);

    if ( -1 == len ) {
        lua_pushnil(L);
        lua_pushstring(L, zip_file_strerror(*file));
        return 2;
    }

    lua_pushlstring(L, buff, len);
    return 1;
}

static void S_register_archive(lua_State* L) {
    luaL_newmetatable(L, ARCHIVE_MT);

    lua_pushcfunction(L, S_archive_close);
    lua_setfield(L, -2, "close");

    lua_pushcfunction(L, S_archive_get_num_files);
    lua_setfield(L, -2, "get_num_files");

    lua_pushcfunction(L, S_archive_name_locate);
    lua_setfield(L, -2, "name_locate");

    lua_pushcfunction(L, S_archive_file_open);
    lua_setfield(L, -2, "open");

    lua_pushcfunction(L, S_archive_stat);
    lua_setfield(L, -2, "stat");

    lua_pushcfunction(L, S_archive_get_name);
    lua_setfield(L, -2, "get_name");

    lua_pushcfunction(L, S_archive_get_archive_comment);
    lua_setfield(L, -2, "get_archive_comment");

    lua_pushcfunction(L, S_archive_set_archive_comment);
    lua_setfield(L, -2, "set_archive_comment");

    lua_pushcfunction(L, S_archive_get_file_comment);
    lua_setfield(L, -2, "get_file_comment");

    lua_pushcfunction(L, S_archive_set_file_comment);
    lua_setfield(L, -2, "set_file_comment");

    lua_pushcfunction(L, S_archive_add_dir);
    lua_setfield(L, -2, "add_dir");

    lua_pushcfunction(L, S_archive_add);
    lua_setfield(L, -2, "add");

    lua_pushcfunction(L, S_archive_replace);
    lua_setfield(L, -2, "replace");

    lua_pushcfunction(L, S_archive_rename);
    lua_setfield(L, -2, "rename");

    lua_pushcfunction(L, S_archive_delete);
    lua_setfield(L, -2, "delete");

    lua_pop(L, 1);
}

static void S_register_archive_file(lua_State* L) {
    luaL_newmetatable(L, ARCHIVE_FILE_MT);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushcfunction(L, S_archive_file_close);
    lua_setfield(L, -2, "close");

    lua_pushcfunction(L, S_archive_file_gc);
    lua_setfield(L, -2, "__gc");

    lua_pushcfunction(L, S_archive_file_read);
    lua_setfield(L, -2, "read");

    lua_pop(L, 1);
}

static void S_register_weak(lua_State* L) {
    luaL_newmetatable(L, WEAK_MT);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    lua_pushliteral(L, "k");
    lua_setfield(L, -2, "__mode");

    lua_pop(L, 1);
}

static int S_OR(lua_State* L) {
    int result = 0;
    int top = lua_gettop(L);
    while ( top ) {
        result |= luaL_checkint(L, top--);
    }
    lua_pushinteger(L, result);
    return 1;
}

LUALIB_API int luaopen_brimworks_zip(lua_State* L) {
    static luaL_Reg fns[] = {
        { "open",     S_archive_open },
        { "OR",       S_OR },
        { NULL, NULL }
    };

    lua_newtable(L);
    luaL_register(L, NULL, fns);

#define EXPORT_CONSTANT(NAME) \
    lua_pushnumber(L, ZIP_##NAME); \
    lua_setfield(L, -2, #NAME)

    EXPORT_CONSTANT(CREATE);
    EXPORT_CONSTANT(EXCL);
    EXPORT_CONSTANT(CHECKCONS);
    EXPORT_CONSTANT(FL_NOCASE);
    EXPORT_CONSTANT(FL_NODIR);
    EXPORT_CONSTANT(FL_COMPRESSED);
    EXPORT_CONSTANT(FL_UNCHANGED);
    EXPORT_CONSTANT(FL_RECOMPRESS);

    S_register_archive(L);
    S_register_archive_file(L);
    S_register_weak(L);

    return 1;
}
