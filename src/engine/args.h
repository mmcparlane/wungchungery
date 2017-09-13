//
// Copyright � Mason McParlane
//

#ifndef WCH_ARGS_H
#define WCH_ARGS_H

#define WCH_ARGS_OK 0
#define WCH_ARGS_MISSING_FLAG 1
#define WCH_ARGS_MISSING_VALUE 2
#define WCH_ARGS_BAD_FORMAT 3
#define WCH_ARGS_LUA_ERR 4

typedef struct wch_Arg wch_Arg;
struct wch_Arg {
	const char* name;
	const char* flags;
	const char* description;
	int mandatory;
	int type;
};

int wch_parse_args(lua_State* L,
		   int argc,
		   const char* argv[],
		   const wch_Arg expected[]);

#endif
