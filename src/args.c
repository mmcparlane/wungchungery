//
// Copyright © Mason McParlane
//

#include "lua.h"
#include "lauxlib.h"

#define ERROR_MISSING_ARG(L, ARG) \
	return luaL_error(L, "Required argument '%s' is missing", ARG ## .name);

#define ERROR_INVALID_ARG(L, ARGV, I) \
	return luaL_error(L, "Invalid argument '%s' specified for '%s'", ARGV[I], ARGV[I-1]);

#define ERROR_MISSING_VALUE(L, ARGV, I) \
	return luaL_error(L, "No value specified for '%s'", ARGV[I-1]);

#define CHECK_INVALID_ARG(L, ARGV, I) \
	if (lua_isnil(L, -1)) {	\
		ERROR_INVALID_ARG(L, ARGV, I); \
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
	for (i = 0; i < argc; ++i) {
		j = 0;
		while ((const wch_Arg* a = args[j++]).name) {
			lua_getfield(L, ISTRING, "match");
			{
				lua_pushstring(L, a->flags);
				lua_pushstring(L, argv[i]);
				lua_call(L, 2, 1);
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
						lua_call(L, 1, 1);
						
						CHECK_INVALID_ARG(L, argv, i)
						
					} else {
						ERROR_MISSING_VALUE(L, argv, i)
					}
					break;
				}
				lua_setfield(L, IRETURN, a.name);
			}
			// function match
			lua_pop(L);
		}
	}
	// table string
	lua_pop(L);

	// Check for missing arguments
	for (i = 0; i < argc; ++i) {
		j = 0;
		while ((const wch_Arg* a = args[j++]).name) {
			if (a.mandatory) {
				lua_getfield(L, IRETURN, a.name);
				if (lua_isnil(L, -1)) {
					ERROR_MISSING_ARG(L, a)
				}
				// field a.name
				lua_pop(L);
			}
		}
	}

	return 1;
}
