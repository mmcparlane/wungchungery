//
// Copyright © Mason McParlane
//

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "wch_args.h"


static const wch_Arg args[] = {
	{
		"help",
		"-h --help /?",
		"prints help message",
		0,
		LUA_TBOOLEAN,
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

	return 0;
}
