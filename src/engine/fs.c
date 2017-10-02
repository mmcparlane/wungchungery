//
// Copyright © Mason McParlane
//

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif


#if defined(__EMSCRIPTEN__)
static int fs_mkdir(lua_State* L) {
	const char* path = luaL_checkstring(L, 1);

	lua_pushfstring(L,"FS.mkdir('%s');", path);
	emscripten_run_script(lua_tostring(L, -1));
	
	return 0;
}

static int fs_rmdir(lua_State* L) {
	const char*  path = luaL_checkstring(L, 1);

	lua_pushfstring(L, "FS.rmdir('%s');", path);
	emscripten_run_script(lua_tostring(L, -1));
	
	return 0;
}

static int fs_ls(lua_State* L) {
	int ifiles = 0, iresult = 0, iiter = 0;
	const char* path = luaL_gsub(L, luaL_checkstring(L, 1), "\\", "/");
	const char* type = "all";
	if (lua_isstring(L, 2)) type = lua_tostring(L, 2);

	lua_pushfstring(
		L,
		"(function(path, type) {"
		"    var r = [];"
		"    if (ENVIRONMENT_IS_NODE) {"
		"        var fs = require('fs');"
		"        try {"
		"            var children = fs.readdirSync(path);"
		"        } catch(e) {"
		"            throw new Error('Error reading directory \"' + path + '\"');"
		"        }"
		"        children.forEach("
		"            function(file) {"
		"                file = (path.endsWith('/') ? path : path + '/') + file;"
		"                var s = fs.statSync(file);"
		"                if (s) {"
		"                    switch(type) {"
		"                    case 'dir':"
		"                        if (s.isDirectory()) r.push(file);"
		"                        break;"
		"                    case 'file':"
		"                        if (s.isFile()) r.push(file);"
		"                        break;"
		"                    default:"
		"                        r.push(file);"
		"                        break;"
		"                    }"
		"                }"
		"        });"
		"    }"
		"    return r.join('\\n').concat('\\n');"
		" })('%s', '%s')", path, type);

	ifiles = lua_gettop(L);
	
	lua_newtable(L);
	iresult = lua_gettop(L);
	
	lua_getglobal(L, "string");
	lua_getfield(L, -1, "gmatch");
        lua_pushstring(L, emscripten_run_script_string(lua_tostring(L, ifiles)));
	lua_pushstring(L, ".+\n");
	lua_call(L, 2, 1);
	iiter = lua_gettop(L);

	// for f in iter do r[#r + 1] = f end
        lua_pushvalue(L, iiter);
	lua_call(L, 0, 1);
	while(lua_isstring(L, -1)) {
		lua_rawseti(L, iresult, lua_rawlen(L, iresult)+1);
		
		lua_pushvalue(L, iiter);
		lua_call(L, 0, 1);
	}

	lua_pushvalue(L, iresult);
	
	return 1;
}

static int fs_pwd(lua_State* L) {
	lua_pushstring(L, emscripten_run_script_string("FS.cwd();"));
	
	return 1;
}


#else
static int fs_mkdir(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_rmdir(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_ls(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_pwd(lua_State* L) {
	// Not implemented
	return 0;
}
#endif


#define FS_LS(L, PATH, TYPE) do { \
		lua_pushcfunction(L, fs_ls); \
		lua_pushstring(L, (PATH)); \
		lua_pushstring(L, (TYPE)); \
		lua_call(L, 2, 1); \
	} while(0)

static int fs_find(lua_State* L) {
	int istring = 0, itable = 0, iresult = 0, idirs = 0, ifiles = 0;
	const char* path = luaL_checkstring(L, 1);
	const char* pattern = luaL_checkstring(L, 2);
	if (! lua_istable(L, 3)) {
		lua_newtable(L);
		iresult = lua_gettop(L);
	}

	lua_getglobal(L, "string");
	istring = lua_gettop(L);
	
	lua_getglobal(L, "table");
	itable = lua_gettop(L);

	FS_LS(L, path, "file");
	ifiles = lua_gettop(L);

	lua_pushnil(L);
	while (lua_next(L, ifiles)) {
		lua_getfield(L, istring, "find");
		lua_pushvalue(L, -2);
		lua_pushstring(L, pattern);
		lua_call(L, 2, 1);
		if (! lua_isnil(L, -1)) {
			lua_pop(L, 1);
		        lua_rawseti(L, iresult, lua_rawlen(L, iresult)+1);
		} else {
			lua_pop(L, 2);
		}
	}

	FS_LS(L, path, "dir");
	idirs = lua_gettop(L);

	if (lua_rawlen(L, -1) > 0) {
		lua_pushnil(L);
		while (lua_next(L, idirs)) {
			lua_pushcfunction(L, fs_find);
			lua_pushvalue(L, -2);
			lua_pushstring(L, pattern);
			lua_pushvalue(L, iresult);
			lua_call(L, 3, 0);
			
			lua_pop(L, 1);
		}
	}

	lua_pushvalue(L, iresult);
	return 1;
}

static const luaL_Reg fs_lib[] = {
	{"mkdir", fs_mkdir},
	{"rmdir", fs_rmdir},
	{"ls", fs_ls},
	{"find", fs_find},
	{"pwd", fs_pwd},
	{NULL, NULL},
};

int luaopen_fs(lua_State* L) {
	luaL_newlib(L, fs_lib);
	return 1;
}

