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
static int fs_new(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_mkdir(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_ls(lua_State* L) {
	// Not implemented
	return 0;
}
#endif


static const luaL_Reg fs_lib[] = {
	{"new", fs_new},
	{"mkdir", fs_mkdir},
	{"ls", fs_ls},
	{NULL, NULL},
};

int luaopen_fs(lua_State* L) {
	luaL_newlib(L, fs_lib);
	return 1;
}

