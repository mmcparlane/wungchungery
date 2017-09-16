//
// Copyright © Mason McParlane
//

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif


#if defined(__EMSCRIPTEN__)
static int fs_mount(lua_State* L) {
        const char* real = luaL_checkstring(L, 1);
	const char* virtual = luaL_checkstring(L, 2);

	lua_pushfstring(L,
			"    if (ENVIRONMENT_IS_NODE) {"
			"        FS.mkdir('%s');"
			"        FS.mount(NODEFS, {root: '%s'}, '%s');"
			"    }",
			virtual, real, virtual);
	
	emscripten_run_script(lua_tostring(L, -1));
	
	return 0;
}

static int fs_unmount(lua_State* L) {
	const char* virtual = luaL_checkstring(L, 1);

	lua_pushfstring(L,"FS.unmount('%s');", virtual);
	emscripten_run_script(lua_tostring(L, -1));
	
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


static const luaL_Reg fs_lib[] = {
	{"mount", fs_mount},
	{"unmount", fs_unmount},
	{"mkdir", fs_mkdir},
	{"rmdir", fs_rmdir},
	{"ls", fs_ls},
	{"pwd", fs_pwd},
	{NULL, NULL},
};

int luaopen_fs(lua_State* L) {
	luaL_newlib(L, fs_lib);
	return 1;
}

