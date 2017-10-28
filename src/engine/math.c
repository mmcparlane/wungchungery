//
// Copyright © Mason McParlane
//
#include <stdio.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

static int math_test(lua_State* L) {
	printf("in math2 lib test...\n");
	return 0;
}

static const luaL_Reg math_lib[] = {
	{"m2test", math_test},
	{NULL, NULL},
};

int luaopen_math2(lua_State* L) {
	luaL_newlib(L, math_lib);
	
        return 1;
}
