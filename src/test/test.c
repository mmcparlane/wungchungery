//
// Copyright © Mason McParlane
//
#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "args.h"


static const wch_Arg args[] = {
	{
		"help",
		"-h --help /?",
		"prints help message",
		0,
		LUA_TBOOLEAN,
	},
	{
		"value",
		"-v --value",
		"test value",
		0,
		LUA_TNUMBER,
	},
	{NULL, NULL, NULL, 0, 0},
};

static const luaL_Reg test_lib[] = {
	{NULL, NULL},
};

int test_start(int argc, const char* argv[]) {
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_newlib(L, test_lib);
	lua_setglobal(L, "test");

	wch_parse_args(L, argc, argv, args);
	luaL_checktype(L, -1, LUA_TTABLE);
	const int IARGT = lua_gettop(L);

	lua_pushnil(L);
	const int ITMP = lua_gettop(L);
	
	lua_pushnil(L);
	while (lua_next(L, IARGT) != 0) {
		lua_copy(L, -2, ITMP);
		lua_getglobal(L, "print");
		lua_rotate(L, -3, 1);
		lua_call(L, 2, 0);
		lua_pushvalue(L, ITMP);
	}
	
	return 0;
}
