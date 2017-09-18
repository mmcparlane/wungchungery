//
// Copyright © Mason McParlane
//

#include <stdio.h>
#include <stdlib.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "args.h"
#include "fs.h"

static const wch_ArgInfo args[] = {
	{
		"help",
		"-h --help /?",
		"Prints this help message.",
		WCH_ARGS_NOFALLBACK,
		WCH_ARGS_OPTIONAL,
		LUA_TBOOLEAN,
	},
	{
		"dir",
		"-d --dir /d",
		"Folder containing set of Lua scripts (tests) to run.",
	        "./scripts/test/",
		WCH_ARGS_REQUIRED,
		LUA_TSTRING,
	},
	{NULL, NULL, NULL, 0, 0},
};

static const luaL_Reg test_lib[] = {
	{NULL, NULL},
};

int test_start(int argc, const char* argv[]) {
	
	wch_AppInfo appinfo = {
		"Wungchungery Test Framework",
		argv[0],
		"Program for running Wungchungery unit tests.",
	};
	
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);

	// Add test lib
	luaL_newlib(L, test_lib);
	lua_setglobal(L, "test");

	// Add filesystem lib
	luaopen_fs(L);
	lua_setglobal(L, "fs");

	if (wch_parse_args(L, argc, argv, args)) {
		// Process help flag
		lua_getfield(L, -1, "help");
		if (lua_toboolean(L, -1)) {
			wch_usage(L, &appinfo, args);
			return 1;
		}
		lua_pop(L, 1);

		// Process dir flag
		//lua_getfield(L, -1, "dir");
		//if (lua_isnil(L, -1)) {
		//	lua_pushstring(L,
		//		       "Required parameter 'dir' is missing.");
		//	return lua_error(L);
		//}

		// TODO
		// There is a bug with the arg processing.
		// When an argument has a default value and the flag is
		// not specified it is not getting populated in the
		// arg-parsed table.
	}
	return 0;
}
