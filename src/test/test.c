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


static wch_AppInfo appinfo = {
	"Wungchungery Test Framework",
	"test",
	"Program for running Wungchungery unit tests.",
};

static const wch_ArgInfo arginfo[] = {
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

static lua_State* initialize() {
	lua_State* L = luaL_newstate();

	// Add standard libs
	luaL_openlibs(L);

	// Add test lib
	luaL_newlib(L, test_lib);
	lua_setglobal(L, "test");

	// Add filesystem lib
	lua_pushcfunction(L, luaopen_fs);
	lua_call(L, 0, 1);
	lua_setglobal(L, "fs");

	return L;
}

static int run(lua_State* L) {
	int iargs = 0, itest = 0, ifs = 0;
	luaL_checktype(L, 1, LUA_TTABLE); iargs = lua_gettop(L);
	lua_getglobal(L, "test"); itest = lua_gettop(L);
	lua_getglobal(L, "fs"); ifs = lua_gettop(L);

	// Process help flag
	lua_getfield(L, iargs, "help");
	if (lua_toboolean(L, -1)) {
		wch_usage(L, &appinfo, arginfo);
		return 1;
	}
	lua_pop(L, 1);

	// Process dir flag
	lua_getfield(L, iargs, "dir");
	if (lua_isnil(L, -1)) {
		lua_pushstring(L,
			       "Required parameter 'dir' is missing.");
		return lua_error(L);
	} else {
		//lua_getfield(L, ifs, "mount");
		//lua_pushvalue(L, -2);
		//lua_pushvalue(L, -2);
		//lua_call(L, 1, 0);
	}
	return 0;
}

int test_start(int argc, const char* argv[]) {

	appinfo.cmdname = argv[0];

	lua_State* L = initialize();

	lua_pushcfunction(L, run);

	wch_parse_args(L, argc, argv, arginfo);
	if(lua_istable(L, -1)) {
		lua_call(L, 1, 0);
		
	} else if (lua_isfunction(L, -1)) {
		lua_call(L, 0, 1);
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		return 1;
		
	} else {
		fprintf(stderr, "Parameter parsing returned unsupported "
			"type '%s'.\n", lua_typename(L, lua_type(L, -1)));
		return 1;

	}
	return 0;
}
