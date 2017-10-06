//
// Copyright © Mason McParlane
//

#include "lua.h"
#include "lauxlib.h"

typedef struct wch_Event wch_Event;
typedef struct wch_Update wch_Update;
typedef struct wch_EventInfo wch_EventInfo;
typedef int (*wch_NewEvent)(lua_State* L, wch_Event* event);

enum {
	WCH_EUPDATE,
	WCH_ERENDER,
	WCH_ESTART,
	WCH_ESTOP,
	WCH_EPAUSE,	
	WCH_ETOTAL,
};

struct wch_Update {
	lua_Number frameratio;
};

struct wch_Event {
        lua_Integer id;
	
	union {
		wch_Update update;
	};
};

struct wch_EventInfo {
	lua_Integer id;
	const char* name;
	wch_NewEvent create;
};

static int newupdate(lua_State* L, wch_Event* event) {
	return 0;
}

static int newrender(lua_State* L, wch_Event* event) {
	return 0;
}

static int newstart(lua_State* L, wch_Event* event) {
	return 0;
}

static int newstop(lua_State* L, wch_Event* event) {
	return 0;
}

static int newpause(lua_State* L, wch_Event* event) {
	return 0;
}

static const wch_EventInfo eventinfo[WCH_ETOTAL] = {
	[WCH_EUPDATE] = {WCH_EUPDATE, "EUPDATE", newupdate},
	[WCH_ERENDER] = {WCH_ERENDER, "ERENDER", newrender},
	[WCH_ESTART] = {WCH_ESTART, "ESTART", newstart},
	[WCH_ESTOP] = {WCH_ESTOP, "ESTOP", newstop},
	[WCH_EPAUSE] = {WCH_EPAUSE, "EPAUSE", newpause},
};

#define QUEUE_DEFAULT_SIZE 25

static int equeue_new(lua_State* L) {
	lua_Integer size = luaL_optinteger(L, 1, QUEUE_DEFAULT_SIZE);
	
	lua_newtable(L);
	
	lua_newtable(L);
	lua_setfield(L, -2, "handlers");
	
	lua_pushinteger(L, 0);
	lua_setfield(L, -2, "first");
	
	lua_pushinteger(L, -1);
	lua_setfield(L, -2, "last");

	for (int i = 0; i < WCH_ETOTAL; ++i) {
		lua_pushinteger(L, eventinfo[i].id);
		lua_setfield(L, -2, eventinfo[i].name);
	}
	
	return 1;
}

static int equeue_is_empty(lua_State* L) { 
	return 0;
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
				eventid, lua_typename(L, lua_type(L, -1)));
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
	return 0;
}

static int equeue_process(lua_State* L) {
	return 0;
}

static int equeue_receive(lua_State* L) {
	return 0;
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
