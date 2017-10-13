//
// Copyright © Mason McParlane
//
// NOTE:
//  This file uses various Emscripten FS object functions but currently
//  this does not work with the "--closure 1" optimization setting. Fur-
//  ther research needs to be done in order to get these working. Some
//  possibilities include:
//    1) Use -s EXPORTED_RUNTIME_METHODS or -s EXTRA_EXPORTED_RUNTIME_METHODS
//       to prevent closure from mangling the FS object.
//    2) Expose FS to this library via JavaScript hook. Using cwrap a callback
//       to C code can be exposed which allows JavaScript to send a reference
//       to the FS object.
//    3) Use a Posix C library that has a compatible Emscripten implementation.
//    4) Delete this library and pass all files in on the command-line.

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define DBG() printf("top: %i, type: %s\n", lua_gettop(L), lua_typename(L, lua_type(L, -1)))

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif


#if defined(__EMSCRIPTEN__)

#if defined(WCH_BROWSER)
static int fs_mount(lua_State* L) {
	const char* real = luaL_checkstring(L, 1);
	const char* virt = luaL_checkstring(L, 2);

	// In the browser there is no difference between a real
	// and a virtual filesystem. The MEMFS is used and all
	// files are preloaded via the emcc --preload-file switch.
	lua_pushstring(L, real);
	
	return 1;
}
#else
static int fs_mount(lua_State* L) {
	const char* real = luaL_checkstring(L, 1);
	const char* virt = luaL_checkstring(L, 2);

	lua_pushfstring(L,
			"(function(real, virt) {"
			"        FS.mkdir(virt);"
			"        FS.mount(NODEFS, {root: real}, virt);"
			"        return virt;"
			" })('%s', '%s')", real, virt);

	lua_pushstring(L, emscripten_run_script_string(lua_tostring(L, -1)));
	
	return 1;
}
#endif


static int fs_unmount(lua_State* L) {
	const char* virt = luaL_checkstring(L, 1);

	lua_pushfstring(L,
			"(function(virt) {"
			"    FS.unmount(virt);"
			"})('%s')", virt);

	emscripten_run_script(lua_tostring(L, -1));
	
	return 0;
}

static int fs_mkdir(lua_State* L) {
	const char* path = luaL_checkstring(L, 1);

	lua_pushfstring(L,"(function(path){FS.mkdir(path);})('%s')", path);
	emscripten_run_script(lua_tostring(L, -1));
	
	return 0;
}

static int fs_rmdir(lua_State* L) {
	const char*  path = luaL_checkstring(L, 1);

	lua_pushfstring(L, "(function(path){FS.rmdir(path);})('%s')", path);
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
		"    FS.readdir(path).forEach("
		"        function(file) {"
		"            if (file === '.' || file === '..') return;"
		"            file = (path.endsWith('/') ? path : path + '/') + file;"
		"            var s = FS.stat(file);"
		"            if (s) {"
		"                switch(type) {"
		"                case 'dir':"
		"                    if (FS.isDir(s.mode)) r.push(file);"
		"                    break;"
		"                case 'file':"
		"                    if (FS.isFile(s.mode)) r.push(file);"
		"                    break;"
		"                case 'all':"
		"                    r.push(file);"
		"                    break;"
		"                default:"
		"                    throw new Error('ls: unsupported file type \"' + type + '\" specified');"
		"                    break;"
		"               }"
		"            }"
		"        });"
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
	lua_pushstring(L, emscripten_run_script_string("FS.cwd()"));

	return 1;
}


#else
static int fs_mount(lua_State* L) {
	// Not implemented
	return 0;
}

static int fs_unmount(lua_State* L) {
	// Not implemented
	return 0;
}

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
	{"unmount", fs_unmount},
	{"mount", fs_mount},
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

