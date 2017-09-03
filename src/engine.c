//
// Copyright © Mason McParlane
//
// For now this implements an asynchronous REPL loop which
// allows web-based access to the Lua interpreter. Eventually
// SDL support will be added to Lua along with HTML5/CSS widgets
// as the basis for a browser-based game-engine IDE.

#include <stdio.h>
#include <string.h>
//#include <stdbool.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static void engine_em_update(void* data){
	lua_State* L = (lua_State*)(data);
	
	char* buff = "print('A goat is in there')\n";
	int error;

	error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
	}
}

static void engine_loop(lua_State* L) {
	emscripten_set_main_loop_arg(engine_em_update, L, 10, 0);
}

#else
static void engine_loop(lua_State* L) {
	// TODO
}

#endif


int main (int argc, char* argv[]) {
	printf("goat1\n");
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	engine_loop(L);
	printf("goat2\n");
	/*
	lua_close(L);
	*/
	return 0;
}
