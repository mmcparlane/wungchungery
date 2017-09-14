//
// Copyright © Mason McParlane
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		ERROR_BAD_FORMAT(L, ARGV, I) \
	}

#define IARG 1
#define IFLAGS 2
#define ISTRING 3
static int contains_flag(lua_State* L) {
	const char* arg = luaL_checkstring(L, IARG);
	luaL_checktype(L, IFLAGS, LUA_TSTRING);
	lua_getglobal(L, "string");

	// Create flag iterator.
	lua_getfield(L, ISTRING, "gmatch");
	lua_pushvalue(L, IFLAGS);
	lua_pushstring(L, "%S+");
	lua_call(L, 2, 1);

  NEXT_FLAG:
	lua_pushvalue(L, -1);
	lua_call(L, 0, 1);
	
	if (lua_isstring(L, -1)) {
		if (strcmp(arg, lua_tostring(L, -1)) != 0) {
			lua_pop(L, 1);
			goto NEXT_FLAG;
			
		}
		lua_pushboolean(L, 1);	

	} else {
		lua_pushboolean(L, 0);

	}

	return 1;
}

#define IRETURN 1
int wch_parse_args(lua_State* L,
		   int argc,
		   const char* argv[],
		   const wch_Arg expected[]) {

	lua_newtable(L);
	
	int i, j, err;
	const wch_Arg* a;
	for (i = 0; i < argc; ++i) {
		j = 0;
		while ((a = &expected[j++])) {
			if (a->name == NULL) break;
			
			//printf("Comparing '%s' to '%s'\n", lua_tostring(L, -1), argv[i]);
			lua_pushcfunction(L, contains_flag);
			lua_pushstring(L, argv[i]);
			lua_pushstring(L, a->flags);
			lua_call(L, 2, 1);

			int match = lua_isboolean(L, -1) || lua_toboolean(L, -1);
			if (match) {
				switch (a->type) {
				case LUA_TBOOLEAN:
					lua_pushboolean(L, 1);
					break;
					
				case LUA_TNUMBER:
					if (++i < argc) {
						lua_getglobal(L, "tonumber");
						lua_pushstring(L, argv[i]);
						lua_call(L, 1, 1);
						
						CHECK_INVALID_VALUE(L, argv, i)
						
					} else {
						ERROR_MISSING_VALUE(L, argv, i)
					}
					break;
					lua_setfield(L, IRETURN, a->name);
				}
			}

			// match
			lua_pop(L, 1);
		}
	}
	printf("top: %i\n", lua_gettop(L));

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
	
	// ISTRING
	lua_pop(L, 1);

	return WCH_ARGS_OK;
}
