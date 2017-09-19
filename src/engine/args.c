//
// Copyright � Mason McParlane
//

#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "args.h"

static int error(lua_State* L) {
	int id = luaL_checkinteger(L, lua_upvalueindex(1));
	const char* msg = luaL_checkstring(L, lua_upvalueindex(2));
	lua_pushstring(L, msg);
	lua_pushinteger(L, id);
	return 2;
}

#define ERROR_MISSING_FLAG(L, ARG) do {	\
		lua_pushinteger(L, WCH_ARGS_MISSING_FLAG); \
		lua_pushfstring(L, "Required parameter '%s' is missing.", (ARG).name); \
		lua_pushcclosure(L, error, 2); \
		return 1; \
	} while(0)

#define ERROR_MISSING_VALUE(L, FLAG) do { \
		lua_pushinteger(L, WCH_ARGS_MISSING_VALUE); \
		lua_pushfstring(L, "No value specified for '%s'.", (FLAG)); \
		lua_pushcclosure(L, error, 2); \
		return 1; \
	} while (0)

#define ERROR_UNSUPPORTED_FLAG(L, FLAG) do { \
		lua_pushinteger(L, WCH_ARGS_UNSUPPORTED_FLAG); \
		lua_pushfstring(L, "Parameter '%s' is not supported.", (FLAG)); \
		lua_pushcclosure(L, error, 2); \
		return 1; \
	} while (0)

#define ERROR_BAD_FORMAT(L, FLAG, VALUE, TEXPECTED) do { \
		lua_pushinteger(L, WCH_ARGS_BAD_FORMAT); \
		lua_pushfstring(L, "Invalid value '%s' specified for '%s'; expected '%s'.", \
				(VALUE), (FLAG), lua_typename((L), (TEXPECTED))); \
		lua_pushcclosure(L, error, 2); \
		return 1; \
	} while (0)

#define GSUB(L, S, P, R) do { \
		lua_getglobal((L), "string"); \
		lua_getfield((L), -1, "gsub"); \
		lua_pushstring((L), (S)); \
		lua_pushstring((L), (P)); \
		lua_pushstring((L), (R)); \
		lua_call((L), 3, 1); \
		lua_remove((L), -2); \
	} while(0)

#define FLAG_1(L, FLAGS) do { \
		lua_getglobal((L), "string"); \
		lua_getfield((L), -1, "gmatch"); \
		lua_pushstring((L), (FLAGS)); \
		lua_pushstring((L), "%S+"); \
		lua_call(L, 2, 1); \
		lua_call(L, 0, 1); \
		lua_remove(L, -2); \
	} while (0)

#define TNAME_NOBOOL(L, T) (((T) == LUA_TBOOLEAN) ? "" : lua_typename(L, (T)))
#define SPACE_NOBOOL(L, T) (((T) == LUA_TBOOLEAN) ? "" : " ")

void wch_usage(lua_State* L,
	      const wch_AppInfo* appinfo,
	      const wch_ArgInfo expected[]) {

	int i, type;
	const wch_ArgInfo* arginfo;

	// Program description.
	lua_getglobal(L, "print");
	lua_pushfstring(L, "ABOUT\n\t%s\n\t%s\n", appinfo->name, appinfo->description);
	lua_call(L, 1, 0);

	// Summary info.
	lua_getglobal(L, "print");
	
	GSUB(L, appinfo->cmdname, ".*[/\\]", "");
	lua_pushfstring(L, "USAGE\n\t%s", lua_tostring(L, -1));
	lua_remove(L, -2);
	for (i = 0; expected[i].name != NULL; ++i) {
		arginfo = &expected[i];

		FLAG_1(L, arginfo->flags);

		if (arginfo->mandatory == WCH_ARGS_REQUIRED) {
			lua_pushfstring(L,
					"%s %s%s%s",
					lua_tostring(L, -2),
					lua_tostring(L, -1),
					SPACE_NOBOOL(L, arginfo->type),
					TNAME_NOBOOL(L, arginfo->type));
		} else {
			lua_pushfstring(L,
					"%s [%s%s%s]",
					lua_tostring(L, -2),
					lua_tostring(L, -1),
					SPACE_NOBOOL(L, arginfo->type),
					TNAME_NOBOOL(L, arginfo->type));
		}
		lua_remove(L, -2); // Flag 1
		lua_remove(L, -2); // Previous 
	}
	lua_call(L, 1, 0);

	// Detailed info.
	lua_getglobal(L, "print");
	
	lua_pushstring(L, "\n");
	for (i = 0; expected[i].name != NULL; ++i) {
		arginfo = &expected[i];
		
		lua_pushfstring(L,
				"%s\t%s: "
				"\n\t\trequired: %s"
				"\n\t\tflags:    %s"
				"\n\t\ttype:     %s"
				"\n\t\tdefault:  %s"
				"\n"
				"\n\t\t%s"
				"\n"
				"\t\n",
				lua_tostring(L, -1),
				arginfo->name,
				arginfo->mandatory ? "true" : "false",
				arginfo->flags,
				lua_typename(L, arginfo->type),
				arginfo->fallback ? arginfo->fallback : "none",
				arginfo->description);
		
		lua_remove(L, -2);
	}
	lua_call(L, 1, 0);
}

#define CONTAINS_FLAG(L, FINDEX, FMATCHES) do {\
		lua_pushcfunction((L), contains_flag); \
		lua_pushvalue((L), (FINDEX)); \
		lua_pushstring((L), (FMATCHES)); \
		lua_call(L, 2, 1); \
	} while(0)

static int contains_flag(lua_State* L) {
	const char* arg = luaL_checkstring(L, 1); // Supplied flag
	luaL_checktype(L, 2, LUA_TSTRING); // Possible flag matches
	lua_getglobal(L, "string");

	// Create flag iterator.
	lua_getfield(L, 3, "gmatch");
	lua_pushvalue(L, 2);
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
		return 1;
	}
	
	lua_pushboolean(L, 0);
	return 1;
}

#define EXTRACT_VALUE(L, TYPE, IFLAG, IVALUE) do { \
		lua_pushcfunction((L), extract_value); \
		lua_pushinteger((L), (TYPE)); \
		lua_pushvalue((L), (IFLAG)); \
		if ((TYPE) == LUA_TBOOLEAN) { \
			lua_call(L, 2, 1); \
		} else { \
			lua_pushvalue(L, (IVALUE)); \
			lua_call(L, 3, 1); \
		} \
		if (lua_isfunction(L, -1)) \
			return 1; \
	} while(0)

static int extract_value(lua_State* L) {
	lua_Integer type = luaL_checkinteger(L, 1);
	const char* flag = luaL_checkstring(L, 2);
	const char* value = NULL;
	
	switch (type) {
	case LUA_TBOOLEAN:
		lua_pushboolean(L, 1);
		// Boolean on top
		break;
					
	case LUA_TNUMBER:
		if (lua_isnoneornil(L, 3)) {
			ERROR_MISSING_VALUE(L, flag);

		} else {
			value = lua_tostring(L, 3);
			
			lua_getglobal(L, "tonumber");
			lua_pushstring(L, value);
			lua_call(L, 1, 1);

			if (lua_isnil(L, -1)) {
				ERROR_BAD_FORMAT(L, flag, value, type);
			}
		}
		// Number on top					
		break;
					
	case LUA_TSTRING:
		if (lua_isnoneornil(L, 3)) {
			ERROR_MISSING_VALUE(L, flag);
		}
		// String on top
		break;
	}
	
	return 1;
}

int wch_parse_args(lua_State* L,
		   int argc,
		   const char* argv[],
		   const wch_ArgInfo expected[]) {

	int i = 0, iresult = 0, ivalue = 0, iflag = 0, unsupported = 1;
	
	lua_newtable(L); iresult = lua_gettop(L);
	
	// Populate table with defaults.
	for (i = 0; expected[i].name != NULL; ++i) {
		if (expected[i].fallback != NULL) {
			lua_pushstring(L, expected[i].flags); iflag = lua_gettop(L);
			lua_pushstring(L, expected[i].fallback); ivalue = lua_gettop(L);

			EXTRACT_VALUE(L, expected[i].type, iflag, ivalue);

			lua_setfield(L, iresult, expected[i].name);
			
			lua_pop(L, 2);
		}
	}

	if (argc < 2) return 1;
	
	lua_pushnil(L); // Sentinel
	for (i = argc-1; i > 0; --i) lua_pushstring(L, argv[i]);

	do {
		unsupported = 1;
		
		iflag = lua_gettop(L);
		ivalue = iflag - 1;
		
		// Populate table with user-supplied data.
		for (i = 0; expected[i].name != NULL; ++i) {

			CONTAINS_FLAG(L, iflag, expected[i].flags);

			if (lua_toboolean(L, -1)) {
				lua_pop(L, 1);

				EXTRACT_VALUE(L, expected[i].type, iflag, ivalue);

				lua_setfield(L, iresult, expected[i].name);

				if (expected[i].type != LUA_TBOOLEAN) lua_pop(L, 1);
				
				unsupported = 0;
				break;
				
			} else {
				lua_pop(L, 1);
				continue;
			}
		}

		if (unsupported) {
			ERROR_UNSUPPORTED_FLAG(L, lua_tostring(L, iflag));
			
		} else {
			lua_pop(L, 1); // Argument (or Value if there was one)
		}
		
	} while (! lua_isnil(L, -1));

	lua_pop(L, 1); // Sentinel

	return 1;
}
