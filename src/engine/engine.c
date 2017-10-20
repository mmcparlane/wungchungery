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
#include "gfx.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

enum {
	ENGINE_STARTED,
	ENGINE_RUNNING,
	ENGINE_STOPPED,
	ENGINE_PAUSED,
	ENGINE_RESUMED,
};

static int engine_run(lua_State* L);
static int engine_update(lua_State* L);
static int engine_start(lua_State* L);
static int engine_stop(lua_State* L);
static int engine_pause(lua_State* L);
static int engine_resume(lua_State* L);
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
	{
		"interval",
		"-i --interval /i",
		"Milliseconds between simulation updates.",
	        "50.0",
		WCH_ARGS_REQUIRED,
		LUA_TNUMBER,
	},
	{NULL, NULL, NULL, 0, 0},
};


#if defined(__EMSCRIPTEN__)

#if defined(WCH_BROWSER)
static int engine_clock_now(lua_State* L) {
	lua_Number now = EM_ASM_DOUBLE_V(return performance.now());
	lua_pushnumber(L, now);
	return 1;
}
#else
static int engine_clock_now(lua_State* L) {
	lua_Number now = EM_ASM_DOUBLE_V(
		var t = process.hrtime();
		return (t[0]*1e9 + t[1])/1e6;
	);
	lua_pushnumber(L, now);
	return 1;
}
#endif

static void engine_em_update(void* data){
	lua_State* L = (lua_State*)(data);

	// Fetch update function and call it.
	lua_getglobal(L, "engine");
	lua_pushstring(L, "update");
	lua_gettable(L, -2);
	lua_call(L, 0, 0);
}

// JavaScript will trigger these.
EMSCRIPTEN_KEEPALIVE void engine_em_runlua(lua_State* L, const char* chunk) {
	int err = luaL_loadstring(L, chunk);
	if (err) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		
	} else {
		err = lua_pcall(L, 0, 0, 0);
		if (err) fprintf(stderr, "Error: %s\n", lua_tostring(L, -1));
	}
}

EMSCRIPTEN_KEEPALIVE void engine_em_start(lua_State* L) {
	lua_pushcfunction(L, engine_start);
	lua_call(L, 0, 0);
}

EMSCRIPTEN_KEEPALIVE void engine_em_stop(lua_State* L) {
	lua_pushcfunction(L, engine_stop);
	lua_call(L, 0, 0);
}

EMSCRIPTEN_KEEPALIVE void engine_em_pause(lua_State* L) {
	lua_pushcfunction(L, engine_pause);
	lua_call(L, 0, 0);
}

EMSCRIPTEN_KEEPALIVE void engine_em_resume(lua_State* L) {
	lua_pushcfunction(L, engine_resume);
	lua_call(L, 0, 0);
}

static int engine_run(lua_State* L) {
	EM_ASM_({if (Module['onSimulationInitialized'])
				Module['onSimulationInitialized']($0);
			
		}, (unsigned long int)L);
	
	return 0;
}

// Main loop will trigger these.
static int engine_onstarted(lua_State* L) {
	emscripten_set_main_loop_arg(engine_em_update, L, 0, 0);
	return 0;
}

static int engine_onstopped(lua_State* L) {
	emscripten_cancel_main_loop();
	return 0;
}

static int engine_onpaused(lua_State* L) {
	emscripten_pause_main_loop();
	return 0;
}

static int engine_onresumed(lua_State* L) {
	emscripten_resume_main_loop();
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
static int engine_onstarted(lua_State* L) {
	// Not implemented.
	exit(EXIT_FAILURE);
	return 0;
}

static int engine_onstopped(lua_State* L) {
	// Not implemented.
	exit(EXIT_FAILURE);
	return 0;
}

static int engine_onpaused(lua_State* L) {
	// Not implemented.
	exit(EXIT_FAILURE);
	return 0;
}

static int engine_onresumed(lua_State* L) {
	// Not implemented.
	exit(EXIT_FAILURE);
	return 0;
}
#endif


static int engine_start(lua_State* L) {
	lua_getglobal(L, "onstart");
	if (lua_isfunction(L, -1)) {
		lua_call(L, 0, 0);
			
	} else {
		lua_pop(L, 1);
	}
	lua_pushcfunction(L, engine_onstarted);
	lua_call(L, 0, 0);
	
	lua_getglobal(L, "engine");
	lua_pushinteger(L, ENGINE_STARTED);
	lua_setfield(L, -2, "state");
	return 0;
}

static int engine_stop(lua_State* L) {
	lua_getglobal(L, "onstop");
	if (lua_isfunction(L, -1)) {
		lua_call(L, 0, 0);
			
	} else {
		lua_pop(L, 1);
	}
	lua_getglobal(L, "engine");
        lua_pushinteger(L, ENGINE_STOPPED);
	lua_setfield(L, -2, "state");	
	return 0;
}

static int engine_pause(lua_State* L) {
	lua_getglobal(L, "onpause");
	if (lua_isfunction(L, -1)) {
		lua_call(L, 0, 0);
			
	} else {
		lua_pop(L, 1);
	}
	lua_getglobal(L, "engine");
	lua_pushinteger(L, ENGINE_PAUSED);
	lua_setfield(L, -2, "state");
	return 0;
}

static int engine_resume(lua_State* L) {
	lua_getglobal(L, "onresume");
	if (lua_isfunction(L, -1)) {
		lua_call(L, 0, 0);
			
	} else {
		lua_pop(L, 1);
	}
	lua_getglobal(L, "engine");
	lua_pushinteger(L, ENGINE_RESUMED);
	lua_setfield(L, -2, "state");	
	return 0;
}

static int engine_update(lua_State* L) {
	lua_Number lag, now, before, gap, interval;

	lua_getglobal(L, "engine");
	lua_getfield(L, -1, "state");
	switch(lua_tointeger(L, -1)) {
	case ENGINE_STARTED:
		//  Update 'before' time
		lua_pushcfunction(L, engine_clock_now);
		lua_call(L, 0, 1);
	        lua_copy(L, -1, lua_upvalueindex(3));
		
		lua_pushinteger(L, ENGINE_RUNNING);
		lua_setfield(L, -4, "state");
		break;
		
	case ENGINE_STOPPED:
		lua_pushcfunction(L, engine_onstopped);
		lua_call(L, 0, 0);
		break;
		
	case ENGINE_PAUSED:
		lua_pushcfunction(L, engine_onpaused);
		lua_call(L, 0, 0);		
		break;
		
	case ENGINE_RESUMED:
		lua_pushcfunction(L, engine_onresumed);
		lua_call(L, 0, 0);
		
		//  Update 'before' time
		lua_pushcfunction(L, engine_clock_now);
		lua_call(L, 0, 1);
	        lua_copy(L, -1, lua_upvalueindex(3));
		break;		
	}

	interval = lua_tonumber(L, lua_upvalueindex(1));
	lag = lua_tonumber(L, lua_upvalueindex(2));
	before = lua_tonumber(L, lua_upvalueindex(3));

	lua_pushcfunction(L, engine_clock_now);
	lua_call(L, 0, 1);
	
	now = luaL_checknumber(L, -1);
	gap = now - before;
	
	lag += gap;

	lua_getglobal(L, "oninput");
	if (lua_isfunction(L, -1)) {
		lua_call(L, 0, 0);

	} else {
		lua_pop(L, 1);
	}

	while (lag >= interval) {
		
		lua_getglobal(L, "onupdate");
		if (lua_isfunction(L, -1)) {
			lua_call(L, 0, 0);
			
		} else {
			lua_pop(L, 1);
		}
		
		lag -= interval;
	}

	lua_getglobal(L, "onrender");
	if (lua_isfunction(L, -1)) {
		lua_pushnumber(L, (lag / interval));
		lua_call(L, 1, 0);
		
	} else {
		lua_pop(L, 1);
	}

	// Update closure variables for next iteration.
	lua_pushnumber(L, lag);
	lua_copy(L, -1, lua_upvalueindex(2));
	lua_pushnumber(L, now);
	lua_copy(L, -1, lua_upvalueindex(3));

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

	// ... Add engine states
	lua_pushinteger(L, ENGINE_STARTED);
	lua_setfield(L, -2, "STARTED");

	lua_pushinteger(L, ENGINE_RUNNING);
	lua_setfield(L, -2, "RUNNING");
	
	lua_pushinteger(L, ENGINE_STOPPED);
	lua_setfield(L, -2, "STOPPED");

	lua_pushinteger(L, ENGINE_PAUSED);
	lua_setfield(L, -2, "PAUSED");

	lua_pushinteger(L, ENGINE_RESUMED);
	lua_setfield(L, -2, "RESUMED");

	// ... Set default engine state
	lua_pushinteger(L, ENGINE_STOPPED);
	lua_setfield(L, -2, "state");	
	
	lua_setglobal(L, "engine");
	
	// Add filesystem lib
	lua_pushcfunction(L, luaopen_fs);
	lua_call(L, 0, 1);
	lua_setglobal(L, "fs");

	// Add gfx lib
	lua_pushcfunction(L, luaopen_gfx);
	lua_call(L, 0, 1);
	lua_setglobal(L, "gfx");

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

		// Initialize modules now that scripts have been loaded.
		lua_getglobal(L, "gfx");
		lua_getfield(L, -1, "initialize");
		lua_call(L, 0, 0);

		// Create update closure with 3 upvalues:
		// interval, lag, before.
		lua_pushstring(L, "update");
		
		lua_getfield(L, iargs, "interval");
	        if (lua_isnil(L, -1)) {
			lua_pushstring(L, "Required parameter 'interval' is missing.");
			return lua_error(L);
		}
		
		lua_pushnumber(L, 0.0);
		
		lua_pushcfunction(L, engine_clock_now);
		lua_call(L, 0, 1);
		
		lua_pushcclosure(L, engine_update, 3);
		lua_settable(L, iengine);
		
		// Run the simulation.
		lua_pushcfunction(L, engine_run);
		lua_call(L, 0, 0);

	}

	return 0;

}

int engine_main(int argc, const char* argv[]) {

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
