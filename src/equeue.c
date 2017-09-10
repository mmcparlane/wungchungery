//
// Copyright © Mason McParlane
//
//   Lua event-queue data structure which allows Lua
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

#include "lua.h"
#include "lauxlib.h"


static const luaL_Reg equeue_lib[] = {
	{"is_empty", equeue_is_empty},
	{"add_handler", equeue_add_handler},
	{"del_handler", equeue_del_handler},
	{"process", equeue_process},
	{"receive", equeue_receive},
	{NULL, NULL},
};

int luaopen_equeue(lua_State* L) {
	luaL_newlib(L, equeue_lib);
	return 1;
}
