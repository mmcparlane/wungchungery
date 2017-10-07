//
// Copyright © Mason McParlane
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "args.h"
#include "fs.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#define ENGINE_UPDATE_INTERVAL 50

static int engine_run(lua_State* L);
static int engine_update(lua_State* L);
static int engine_clock_now(lua_State* L);

static wch_AppInfo appinfo = {
	"Wungchungery Game Engine",
	"engine",
	"Main program for running wungchungery simulations.",
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
		"scripts",
		"-s --scripts /s",
		"Folder containing all engine scripts.",
	        "./scripts/engine/",
		WCH_ARGS_REQUIRED,
		LUA_TSTRING,
	},
	{NULL, NULL, NULL, 0, 0},
};


#if defined(__EMSCRIPTEN__)
static int engine_clock_now(lua_State* L) {
	lua_Number now = EM_ASM_DOUBLE_V(
		if (ENVIRONMENT_IS_NODE) {
			var t = process.hrtime();
			return (t[0]*1e9 + t[1])/1e6;
		} else {
			return performance.now();
		});
	lua_pushnumber(L, now);
	return 1;
}

static void engine_em_update(void* data){
	lua_State* L = (lua_State*)(data);

	// Fetch update function and call it.
	lua_getglobal(L, "engine");
	lua_pushstring(L, "update");
	lua_gettable(L, -2);
	lua_call(L, 0, 0);
}

static int engine_run(lua_State* L) {
	emscripten_set_main_loop_arg(engine_em_update, L, 0, 0);
	return 0;
}


#else
static int engine_clock_now(lua_State* L) {
	// Not implemented.
	exit(EXIT_FAILURE);
	return 0;
}

static int engine_run(lua_State* L) {
	// Not implemented.
	exit(EXIT_FAILURE);
	return 0;
}
#endif


static int engine_update(lua_State* L) {
	double lag = lua_tonumber(L, lua_upvalueindex(1));
	double before = lua_tonumber(L, lua_upvalueindex(2));

	lua_pushcfunction(L, engine_clock_now);
	lua_call(L, 0, 1);
	
	double now = luaL_checknumber(L, 1);
	double gap = now - before;
	lag += gap;

	printf("lag: '%f', before: '%f', now: '%f', gap: '%f'\n",
	       lag, before, now, gap);

	// input();

	while (lag >= ENGINE_UPDATE_INTERVAL) {
		// update();		
		lag -= ENGINE_UPDATE_INTERVAL;
	}

	// render(lag / ENGINE_UPDATE_INTERVAL);

	// Update closure variables for next
	// iteration.
	lua_pushnumber(L, lag);
	lua_copy(L, -1, lua_upvalueindex(1));
	lua_pushnumber(L, now);
	lua_copy(L, -1, lua_upvalueindex(2));

	return 0;
}

static const luaL_Reg engine_lib[] = {
	{"clock_now", engine_clock_now},
	{"run", engine_run},
	{NULL, NULL},
};

static lua_State* initialize() {
	lua_State* L = luaL_newstate();

	// Add standard libs
	luaL_openlibs(L);

	// Add engine lib
	luaL_newlib(L, engine_lib);

	// ... register functions
	lua_pushstring(L, "update");
	lua_pushnumber(L, 0.0);
	lua_pushcfunction(L, engine_clock_now);
	lua_call(L, 0, 1);
	lua_pushcclosure(L, engine_update, 2);
	lua_settable(L, -3);	
	lua_setglobal(L, "engine");

	// Add filesystem lib
	lua_pushcfunction(L, luaopen_fs);
	lua_call(L, 0, 1);
	lua_setglobal(L, "fs");

	return L;
}

static int run(lua_State* L) {
	int err = 0, iargs = 0, iengine = 0, ifs = 0, iscripts = 0;
	luaL_checktype(L, 1, LUA_TTABLE);
	iargs = lua_gettop(L);
	
	lua_getglobal(L, "engine");
	iengine = lua_gettop(L);
	
	lua_getglobal(L, "fs");
	ifs = lua_gettop(L);

	// Process help flag
	lua_getfield(L, iargs, "help");
	if (lua_toboolean(L, -1)) {
		wch_usage(L, &appinfo, arginfo);
		return 1;
	}
	lua_pop(L, 1);

	// Process scripts flag
	lua_getfield(L, iargs, "scripts");
	if (lua_isnil(L, -1)) {
		lua_pushstring(L, "Required parameter 'scripts' is missing.");
		return lua_error(L);
	} else {
		lua_getfield(L, ifs, "mount");
		lua_pushvalue(L, -2); // dir
		lua_pushstring(L, "/scripts");
		lua_call(L, 2, 1);

		lua_getfield(L, ifs, "find");
		lua_pushvalue(L, -2); // mounted dir
		lua_pushstring(L, ".+.lua");
		lua_call(L, 2, 1);
		iscripts = lua_gettop(L);

		lua_pushnil(L);
		while (lua_next(L, iscripts)) {
			printf("Loading script '%s'\n",
			       lua_tostring(L, -1));

			err = luaL_loadfile(L, lua_tostring(L, -1));
			if (err) {
				fprintf(stderr, "%s\n", lua_tostring(L, -1));
				
			} else {
				err = lua_pcall(L, 0, 0, 0);
			        if (err) fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
			}

			lua_pop(L, 1);
		}
	
		// Run the simulation.
		lua_pushcfunction(L, engine_run);
		lua_call(L, 0, 0);
	}

	return 0;

}

int engine_start(int argc, const char* argv[]) {

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
