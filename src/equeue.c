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

static int equeue_new(lua_State* L) {
	lua_newtable(L);
	{
		lua_pushstring(L, "handlers");
		lua_newtable(L);
		lua_settable(L, -3);
	
		lua_pushstring(L, "first");
		lua_pushinteger(L, 0);
		lua_settable(L, -3);
	
		lua_pushstring(L, "last");
		lua_pushinteger(L, -1);
		lua_settable(L, -3);
	}
	
	return 1;
}

static int equeue_is_empty(lua_State* L) { 
}

static int equeue_add_handler(lua_State* L) {
	// Each event ID can have multiple handlers
	// and so each event ID needs to map to a
	// table.

	// If no handlers exist for a particular ID,
	// create a table and add the event to the
	// table.
	if (! lua_istable(L, -3)) {
		luaL_error(L,
			   "bad argument #1 to 'equeue_add_handler'"
			   "(table expected, got %s)",
			   lua_typename(L, lua_type(L, -3)));
	}
	lua_Integer id = luaL_checkinteger(L, -2);
	if (! lua_isfunction(L, -1)) {
		luaL_error(L,
			   "bad argument #3 to 'equeue_add_handler'"
			   "(function expected, got %s)",
			   lua_typename(L, lua_type(L, -1)));
	} else {
		// Fetch sub-table self.handlers.id
		// self is already on the stack so
		// fetch [-3].handlers
		// fetch top.id
		
		
	}
}

static int equeue_del_handler(lua_State* L) {
}

static int equeue_process(lua_State* L) {
}

static int equeue_receive(lua_State* L) {
}



static const luaL_Reg equeue_lib[] = {
	{"new", equeue_new},
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
