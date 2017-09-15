//
// Copyright © Mason McParlane
//

#ifndef WCH_ARGS_H
#define WCH_ARGS_H

#define WCH_ARGS_OK 0
#define WCH_ARGS_MISSING_FLAG 1
#define WCH_ARGS_MISSING_VALUE 2
#define WCH_ARGS_BAD_FORMAT 3
#define WCH_ARGS_UNSUPPORTED_FLAG 4

#define WCH_ARGS_OPTIONAL 0
#define WCH_ARGS_REQUIRED 1

#define WCH_ARGS_NOFALLBACK NULL

typedef struct wch_ArgInfo wch_ArgInfo;
struct wch_ArgInfo {
	const char* name;
	const char* flags;
	const char* description;
	const char* fallback;
	int mandatory;
	int type;
};

typedef struct wch_AppInfo wch_AppInfo;
struct wch_AppInfo {
	const char* name;
	const char* description;
};

void wch_usage(lua_State* L,
	      const wch_AppInfo* appinfo,
	      const wch_ArgInfo expected[]);

int wch_parse_args(lua_State* L,
		   int argc,
		   const char* argv[],
		   const wch_ArgInfo expected[]);

#endif
