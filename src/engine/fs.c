//
// Copyright © Mason McParlane
//

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define DBG() printf("top: %i, type: %s\n", lua_gettop(L), lua_typename(L, lua_type(L, -1)))

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
		"            throw new Error('ls: error reading directory \"' + path + '\"');"
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
		"                    case 'all':"
		"                        r.push(file);"
		"                        break;"
		"                    default:"
		"                        throw new Error('ls: unsupported file type \"' + type + '\" specified');"
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

	lua_pushstring(L, "[%S ]+");
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
	int i = 0, istring = 0, iresult = 0;
	const char* path = luaL_checkstring(L, 1);
	const char* pattern = luaL_checkstring(L, 2);

	lua_pushvalue(L, lua_upvalueindex(1));
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		
	        FS_LS(L, path, "file");
		FS_LS(L, path, "dir");
		lua_pushcclosure(L, fs_find, 2);
		lua_insert(L, -3);
		lua_call(L, 2, 1);
		
	} else {
		lua_pop(L, 1);

		lua_newtable(L);
		iresult = lua_gettop(L);
		
		lua_getglobal(L, "string");
		istring = lua_gettop(L);

		lua_pushnil(L);
		lua_pushstring(L, path);
		do {
			lua_pushnil(L);
			while (lua_next(L, lua_upvalueindex(1))) {
				lua_getfield(L, istring, "find");
				lua_pushvalue(L, -2);
				lua_pushstring(L, pattern);
				lua_call(L, 2, 1);
				if (! lua_isnil(L, -1)) {
					lua_pop(L, 1);
					lua_rawseti(L, iresult, lua_rawlen(L, iresult) + 1);
				
				} else {
					lua_pop(L, 2);
				}
			}

			if (lua_rawlen(L, lua_upvalueindex(2)) > 0) {
				for (i = lua_rawlen(L, lua_upvalueindex(2)); i > 0; --i) {
					lua_rawgeti(L, lua_upvalueindex(2), i);
				}
			}

			path = luaL_checkstring(L, -1);

			FS_LS(L, path, "file");
			lua_copy(L, -1, lua_upvalueindex(1));
			
			FS_LS(L, path, "dir");
			lua_copy(L, -1, lua_upvalueindex(2));
			lua_pop(L, 3); // dir, file, path
			
		} while (! lua_isnil(L, -1));
		
		lua_pushvalue(L, iresult);
	}

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

