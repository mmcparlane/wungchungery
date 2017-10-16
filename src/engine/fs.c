//
// Copyright © Mason McParlane
//

#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>

// Helper functions defined in fs.js
extern const char* wch_fs_mount(const char* real, const char* virt);
extern void wch_fs_unmount(const char* path);
extern void wch_fs_mkdir(const char* path);
extern char* wch_fs_ls(const char* path, const char* type);
extern char* wch_fs_pwd(void);

#if defined(WCH_BROWSER)
static int fs_mount(lua_State* L) {
	const char* real = luaL_checkstring(L, 1);
	const char* virt = luaL_checkstring(L, 2);

	// In the browser there is no difference between a real
	// and a virtual filesystem. The MEMFS is used and all
	// files are preloaded via the emcc --preload-file switch.
	lua_pushstring(L, real);
	
	return 1;
}

#else
static int fs_mount(lua_State* L) {
	const char* real = luaL_checkstring(L, 1);
	const char* virt = luaL_checkstring(L, 2);
	lua_pushstring(L, wch_fs_mount(real, virt));
	return 1;
}
#endif


static int fs_unmount(lua_State* L) {
	const char* virt = luaL_checkstring(L, 1);
        wch_fs_unmount(virt);
	return 0;
}

static int fs_mkdir(lua_State* L) {
	const char* path = luaL_checkstring(L, 1);
        wch_fs_mkdir(path);
	return 0;
}

static int fs_rmdir(lua_State* L) {
	const char*  path = luaL_checkstring(L, 1);

	lua_pushfstring(L, "(function(path){FS.rmdir(path);})('%s')", path);
	emscripten_run_script(lua_tostring(L, -1));
	
	return 0;
}

static int fs_ls(lua_State* L) {
	int ifiles = 0, iresult = 0, iiter = 0;
	const char* path = luaL_gsub(L, luaL_checkstring(L, 1), "\\", "/");
	const char* type = "all";
	char* files = NULL;
	if (lua_isstring(L, 2)) type = lua_tostring(L, 2);

	files = wch_fs_ls(path, type);
	
	lua_newtable(L);
	iresult = lua_gettop(L);
	
	lua_getglobal(L, "string");
	lua_getfield(L, -1, "gmatch");
        lua_pushstring(L, files);
	lua_pushstring(L, "[%S ]+");
	lua_call(L, 2, 1);

	iiter = lua_gettop(L);

	// for f in iter do r[#r + 1] = f end
        lua_pushvalue(L, iiter);
	lua_call(L, 0, 1);
	while(lua_isstring(L, -1)) {
		lua_rawseti(L, iresult, lua_rawlen(L, iresult)+1);

		lua_pushvalue(L, iiter);
		lua_call(L, 0, 1);
	}

	lua_pushvalue(L, iresult);

	free(files);
	
	return 1;
}

static int fs_pwd(lua_State* L) {
	lua_pushstring(L, wch_fs_pwd());
	return 1;
}


#else
static int fs_mount(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_unmount(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_mkdir(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_rmdir(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_ls(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_pwd(lua_State* L) {
	// Not implemented
	return 0;
}
#endif


#define FS_LS(L, PATH, TYPE) do { \
		lua_pushcfunction(L, fs_ls); \
		lua_pushstring(L, (PATH)); \
		lua_pushstring(L, (TYPE)); \
		lua_call(L, 2, 1); \
	} while(0)

static int fs_find(lua_State* L) {
	int i = 0, istring = 0, iresult = 0;
	const char* path = luaL_checkstring(L, 1);
	const char* pattern = luaL_checkstring(L, 2);

	lua_pushvalue(L, lua_upvalueindex(1));
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		
	        FS_LS(L, path, "file");
		FS_LS(L, path, "dir");
		lua_pushcclosure(L, fs_find, 2);
		lua_insert(L, -3);
		lua_call(L, 2, 1);
		
	} else {
		lua_pop(L, 1);

		lua_newtable(L);
		iresult = lua_gettop(L);
		
		lua_getglobal(L, "string");
		istring = lua_gettop(L);

		lua_pushnil(L);
		lua_pushstring(L, path);
		do {
			lua_pushnil(L);
			while (lua_next(L, lua_upvalueindex(1))) {
				lua_getfield(L, istring, "find");
				lua_pushvalue(L, -2);
				lua_pushstring(L, pattern);
				lua_call(L, 2, 1);
				if (! lua_isnil(L, -1)) {
					lua_pop(L, 1);
					lua_rawseti(L, iresult, lua_rawlen(L, iresult) + 1);
				
				} else {
					lua_pop(L, 2);
				}
			}

			if (lua_rawlen(L, lua_upvalueindex(2)) > 0) {
				for (i = lua_rawlen(L, lua_upvalueindex(2)); i > 0; --i) {
					lua_rawgeti(L, lua_upvalueindex(2), i);
				}
			}

			path = luaL_checkstring(L, -1);

			FS_LS(L, path, "file");
			lua_copy(L, -1, lua_upvalueindex(1));
			
			FS_LS(L, path, "dir");
			lua_copy(L, -1, lua_upvalueindex(2));
			lua_pop(L, 3); // dir, file, path
			
		} while (! lua_isnil(L, -1));
		
		lua_pushvalue(L, iresult);
	}

	return 1;
}

static const luaL_Reg fs_lib[] = {
	{"unmount", fs_unmount},
	{"mount", fs_mount},
	{"mkdir", fs_mkdir},
	{"rmdir", fs_rmdir},
	{"ls", fs_ls},
	{"find", fs_find},
	{"pwd", fs_pwd},
	{NULL, NULL},
};

int luaopen_fs(lua_State* L) {
	luaL_newlib(L, fs_lib);
	return 1;
}

