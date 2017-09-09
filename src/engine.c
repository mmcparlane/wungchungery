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

static lua_State* engine_init();
static void engine_loop(lua_State* L);
static int engine_update(lua_State* L);
static double engine_clock_now();

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


static int engine_update(lua_State* L) {
	double before = lua_tonumber(L, lua_upvalueindex(1));
	double now = engine_clock_now();
	
	printf("tick delta: %f')\n", (now-before));
	
	/*
	int error;

	error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
	*/

	lua_pushnumber(L, now);
	lua_copy(L, -1, lua_upvalueindex(1));

	return 0;
}

static lua_State* engine_init() {
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	lua_pushnumber(L, engine_clock_now());
	lua_pushcclosure(L, engine_update, 1);

	return L;
}


#if defined(__EMSCRIPTEN__)
static double engine_clock_now() {
	return EM_ASM_DOUBLE_V(
		if (ENVIRONMENT_IS_NODE) {
			var t = process.hrtime();
			return (t[0]*1e9 + t[1])/1e6;
		} else {
			return performance.now();
		});
}

static void engine_em_update(void* data){
	lua_State* L = (lua_State*)(data);
	lua_pushcfunction(L, engine_update);
	lua_call(L, 0, 0);
}

static void engine_loop(lua_State* L) {
	emscripten_set_main_loop_arg(engine_em_update, L, 0, 0);
}

#else
static void engine_loop(lua_State* L) {
	// TODO
	//   Implement native game-loop.
}

#endif


int main (int argc, char* argv[]) {
	engine_loop(engine_init());
}
