//
// Copyright © Mason McParlane
//
// For now this implements an asynchronous REPL loop which
// allows web-based access to the Lua interpreter. Eventually
// SDL support will be added to Lua along with HTML5/CSS widgets
// as the basis for a browser-based game-engine IDE.

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#define ENGINE_UPDATE_INTERVAL 50

static int engine_run(lua_State* L);
static int engine_update(lua_State* L);
static int engine_clock_now(lua_State* L);

// TODO
//   Add a Lua event-queue data structure which allows Lua
//   code (game-logic-triggered), C code (signals, etc.), and
//   JavaScript(HTML/CSS UI interaction) to trigger events on
//   the runtime system.
//
//   Some of these events will be common to both native
//   and Emscripten implementations--for instance: stop,
//   pause, resume, etc. Others will be specific to a
//   particular implementation.
//
//   Some events get consumed by the main loop while others
//   get consumed by in other places. Hence, the main loop
//   will be responsible for processing the event queue before
//   propagating events to registered handlers.
//
//   Event processing goes something like:
//      - Outside event occurs and gets
//        converted/placed on Lua stack.
//      - Game loop reads event from stack
//        and handles the event if necessary.
//      - Depending on the event, any reg-
//        istered event-handlers get called.
//      - The game loop then proceeds to
//        call the AI, Update, etc.


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
static int engine_run(lua_State* L) {
	// TODO
	//   Implement native game-loop.
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
	
	/*
	int error;

	error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	*/

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

int main (int argc, char* argv[]) {
	// Add standard lib and engine lib while
	// leaving engine lib on stack so more
	// entries can be added as necessary.
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_newlib(L, engine_lib);

	// Create new update closure and add it
	// to the engine lib.
	lua_pushstring(L, "update");
	lua_pushnumber(L, 0.0);
	lua_pushcfunction(L, engine_clock_now);
	lua_call(L, 0, 1);
	lua_pushcclosure(L, engine_update, 2);
	lua_settable(L, -3);

	// Make the engine lib globally accessible.
	lua_setglobal(L, "engine");

	// Run the simulation.
	lua_pushcfunction(L, engine_run);
	lua_call(L, 0, 0);
}
