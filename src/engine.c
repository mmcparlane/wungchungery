
#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int main (void) {
  char* buff = "print('A goat is in there')\n";
  int error;
  lua_State *L = luaL_newstate(); /* opens Lua */
  luaL_openlibs(L); /* opens the standard libraries */
  error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
  if (error) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    lua_pop(L, 1); /* pop error message from the stack */
  }
  lua_close(L);
  return 0;
}
