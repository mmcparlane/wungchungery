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
		lua_newtable(L);
		lua_setfield(L, -2, "handlers");
	
		lua_pushinteger(L, 0);
		lua_setfield(L, -2, "first");
	
		lua_pushinteger(L, -1);
		lua_setfield(L, -2, "last");
	}
	
	return 1;
}

static int equeue_is_empty(lua_State* L) { 
}

static int equeue_add_handler(lua_State* L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_Integer eventid = luaL_checkinteger(L, 2);
	luaL_checktype(L, 3, LUA_TFUNCTION);

	if (lua_getfield(L, 1, "handlers") == LUA_TTABLE) {
		switch(lua_geti(L, -1, eventid)) {
		case LUA_TNIL:
			// x = {}
			// x[#x+1] = hfunc
			// self.handlers[eventid] = x
			lua_pop(L, 1);
			lua_newtable(L);
			{
				lua_pushvalue(L, 3);
				lua_rawseti(L, -2, (lua_rawlen(L, -2) + 1));
			}
			lua_rawseti(L, -2, eventid);
			break;
			
		case LUA_TTABLE:
			// x = self.handlers[eventid]
			// x[#x+1] = hfunc
			lua_pushvalue(L, 3);
			lua_rawseti(L, -2, (lua_rawlen(L, -2) + 1));			
			break;
			
		default:
			return luaL_error(
				L,
				"Event ID '%i' maps to invalid type '%s'. "
				"Table expected (so multiple event handlers "
				"can handle a single event).",
				id, lua_typename(L, lua_type(L, -1)));
		}

		return 0;
		
	} else {
		return luaL_error(
			L,
			"'handlers' field does not exist. Was equeue.new used "
			"to create the equeue type?");
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
