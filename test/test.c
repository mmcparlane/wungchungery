//
// Copyright © Mason McParlane
//

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int test_parse_args(lua_State* L) {
	luaL_checktype(L, 1, LUA_TTABLE);

	for (int i = 1; i <= lua_rawlen(L, 1); ++i) {
		lua_getglobal(L, "print");
		lua_rawgeti(L, 1, i);
		lua_call(L, 1, 0);
	}
	return 0;
}

static const luaL_Reg test_lib[] = {
	{NULL, NULL},
};

int test_start(int argc, char* argv[]) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_newlib(L, test_lib);
	lua_setglobal(L, "test");
	
	// x = {}
	// for i = 1, argc do x[#x + 1] = argv[i]
	// test_parse_args(x)
	lua_pushcfunction(L, test_parse_args);
	lua_newtable(L);
	for (int i = 0; i < argc; ++i) {
		lua_pushstring(L, argv[i]);
		lua_rawseti(L, -2, i+1);
	}

	lua_call(L, 1, 0);

	return 0;
}
