//
// Copyright © Mason McParlane
//

#include "lua.h"
#include "lauxlib.h"
#include "args.h"

#define ERROR_MISSING_FLAG(L, ARG) \
	lua_pushfstring(L, "Required argument '%s' is missing", ARG->name); \
	return WCH_ARGS_MISSING_FLAG;

#define ERROR_BAD_FORMAT(L, ARGV, I) \
	lua_pushfstring(L, "Invalid argument '%s' specified for '%s'", ARGV[I], ARGV[I-1]); \
	return WCH_ARGS_BAD_FORMAT;

#define ERROR_MISSING_VALUE(L, ARGV, I) \
	lua_pushfstring(L, "No value specified for '%s'", ARGV[I-1]); \
	return WCH_ARGS_MISSING_VALUE;

#define CHECK_INVALID_VALUE(L, ARGV, I) \
	if (lua_isnil(L, -1)) {	\
		ERROR_BAD_FORMAT(L, ARGV, I); \
	}

int wch_parse_args(lua_State* L,
		   int argc,
		   const char* argv[],
		   const wch_Arg expected[]) {

	lua_newtable(L);
	const int IRETURN = lua_gettop(L);

	lua_getglobal(L, "string");
	const int ISTRING = lua_gettop(L);
	
	int i, j;
	const wch_Arg* a;
	for (i = 0; i < argc; ++i) {
		j = 0;
		while ((a = &expected[j++])) {
			if (a->name == NULL) break;
			
			lua_getfield(L, ISTRING, "match");
			{
				lua_pushstring(L, a->flags);
				lua_pushstring(L, argv[i]);
				if (lua_pcall(L, 2, 1, 0))
					return WCH_ARGS_LUA_ERR;
			}

			if(! lua_isnil(L, -1)) {
				switch (a->type) {
				case LUA_TBOOLEAN:
					lua_pushboolean(L, 1);
					break;

				case LUA_TNUMBER:
					if (++i < argc) {
					        lua_getglobal(L, "tonumber");
						lua_pushstring(L, argv[i]);
						if (lua_pcall(L, 1, 1, 0))
							return WCH_ARGS_LUA_ERR;
						
						CHECK_INVALID_VALUE(L, argv, i)
						
					} else {
						ERROR_MISSING_VALUE(L, argv, i)
					}
					break;
				}
				lua_setfield(L, IRETURN, a->name);
			}
			// function match
			lua_pop(L, 1);
		}
	}
	// table string
	lua_pop(L, 1);

	// Check for missing arguments
	for (i = 0; i < argc; ++i) {
		j = 0;
		while ((a = &expected[j++])) {
			if (a->name == NULL) break;
			if (a->mandatory) {
				lua_getfield(L, IRETURN, a->name);
				if (lua_isnil(L, -1)) {
					ERROR_MISSING_FLAG(L, a)
				}
				// field a->name
				lua_pop(L, 1);
			}
		}
	}

	return WCH_ARGS_OK;
}
