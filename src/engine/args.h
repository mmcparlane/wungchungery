//
// Copyright © Mason McParlane
//

#ifndef WCH_ARGS_H
#define WCH_ARGS_H

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
