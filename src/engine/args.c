//
// Copyright © Mason McParlane
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "args.h"

#define ERROR_MISSING_FLAG(L, ARG) do {	\
		lua_pushfstring(L, "Required argument '%s' is missing", (ARG).name); \
		return WCH_ARGS_MISSING_FLAG; \
	} while(0)

#define ERROR_BAD_FORMAT(L, FLAG, VALUE, TEXPECTED) do { \
		lua_pushfstring(L, "Invalid value '%s' specified for '%s'; expected '%s'", \
				(VALUE), (FLAG), lua_typename((L), (TEXPECTED))); \
		return WCH_ARGS_BAD_FORMAT; \
	} while (0)

#define ERROR_MISSING_VALUE(L, FLAG) do { \
		lua_pushfstring(L, "No value specified for '%s'", (FLAG)); \
		return WCH_ARGS_MISSING_VALUE; \
	} while (0)


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
		printf("Comparing '%s' to '%s'\n", lua_tostring(L, -1), arg);
		if (strcmp(arg, lua_tostring(L, -1)) != 0) {
			lua_pop(L, 1);
			goto NEXT_FLAG;
			
		}
		lua_pushboolean(L, 1);
		return 1;
	}
	
	lua_pushboolean(L, 0);
	return 1;
}

#define IRETURN 1
int wch_parse_args(lua_State* L,
		   int argc,
		   const char* argv[],
		   const wch_Arg expected[]) {
	int i;
	lua_newtable(L);
	
	if (argc < 2) return WCH_ARGS_OK;
	
	lua_pushnil(L); // Sentinel
	for (i = argc-1; i > 0; --i)
		lua_pushstring(L, argv[i]);

	do {
		for (i = 0; expected[i].name != NULL; ++i) {
			lua_pushcfunction(L, contains_flag);
			lua_pushvalue(L, -2);
			lua_pushstring(L, expected[i].flags);
			lua_call(L, 2, 1);
			
			if (lua_toboolean(L, -1)) {
				lua_pop(L, 1); // Contains_flag result
				
				switch (expected[i].type) {
				case LUA_TBOOLEAN:
					printf("boolean\n");
					lua_pushboolean(L, 1);
					break;
					
				case LUA_TNUMBER:
					printf("number\n");
					//lua_pop(L, 1); // Arg
					//printf("top: %i type: %s\n", lua_gettop(L), lua_typename(L, lua_type(L, -1)));
					lua_getglobal(L, "tonumber");
					lua_pushvalue(L, -3); // Value
					
					if (lua_isnil(L, -1)) {
						lua_pop(L, 2); // Tonumber, Nil

						ERROR_MISSING_VALUE(L, lua_tostring(L, -1)); // Flag

					} else {
						//printf("top: %i type: %s val: %s\n", lua_gettop(L), lua_typename(L, lua_type(L, -1)), lua_tostring(L, -1));
						lua_call(L, 1, 1);
						printf("top: %i type: %s\n", lua_gettop(L), lua_typename(L, lua_type(L, -1)));

						if (lua_isnil(L, -1)) {
							lua_pop(L, 1); // Nil
							
							ERROR_BAD_FORMAT(L,
									 lua_tostring(L, -1), // Flag
									 lua_tostring(L, -2), // Value
									 expected[i].type);
						}

						lua_remove(L, -2); // Value (prepare for next flag)

						printf("top: %i type: %s\n", lua_gettop(L), lua_typename(L, lua_type(L, -1)));
						// Number on top
					}
					break; // Switch
				}
				
				lua_setfield(L, IRETURN, expected[i].name);
				break; // For
				
			} else {
				lua_pop(L, 1); // Contains_flag result
				continue; // For
			}
		}
		
		lua_pop(L, 1); // Argument (or Value if there was one)
		
	} while (! lua_isnil(L, -1));

	lua_pop(L, 1); // Sentinel

	printf("top: %i type: %s\n", lua_gettop(L), lua_typename(L, lua_type(L, -1)));

	return WCH_ARGS_OK;
}
