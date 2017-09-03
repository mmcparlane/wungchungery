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

typedef struct wch_GameState wch_GameState;
struct wch_GameState {
	lua_State* luaState;
};

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void engine_em_update(void* data){
	wch_GameState* game = (wch_GameState*)(data);
	lua_State* L = game->luaState;
	
	char* buff = "print('A goat is in there')\n";
	int error;

	error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void engine_loop(wch_GameState* game) {
	emscripten_set_main_loop_arg(engine_em_update, game, 10, 0);
}

#else
static void engine_loop(wch_GameState* game) {
	// TODO
	//   Implement native game-loop.
}

#endif


int main (int argc, char* argv[]) {
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	engine_loop(L);
	// TODO
	//  Need to have a way to wait for the loop to signal cleanup
	//  routines to run before returning from main(). This probably
	//  is best done as an event-queue-registered handler as noted
	//  above.
	/*
	lua_close(L);
	*/
	return 0;
}
