//
// Copyright � Mason McParlane
//
#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "args.h"


static const wch_Arg args[] = {
	{
		"help",
		"-h --help /?",
		"prints help message",
		WCH_ARGS_NOFALLBACK,
		WCH_ARGS_OPTIONAL,
		LUA_TBOOLEAN,
	},
	{
		"value",
		"-v --value",
		"test value",
	        WCH_ARGS_NOFALLBACK,
		WCH_ARGS_OPTIONAL,
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

	int err = wch_parse_args(L, argc, argv, args);
	printf("top: %i type: %s\n", lua_gettop(L), lua_typename(L, lua_type(L, -1)));
	if (err) {
		lua_getglobal(L, "print");
		lua_rotate(L, -2, 1);
		lua_pcall(L, 1, 0, 0);
		return 1;
		
	} else {
		lua_getfield(L, -1, "help");
		if (lua_toboolean(L, -1)) {
			wch_usage(L, argv[0], args);
			exit(0);
		} else {
			lua_pop(L, 1); // Nil
		}
		
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
		
	}
	return 0;
}
