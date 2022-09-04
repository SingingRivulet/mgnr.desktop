#include <math.h>
#include "lua.hpp"

static int l_sin(lua_State* L) {
    double d = luaL_checknumber(L, 1);

    lua_pushnumber(L, sin(d));

    return 1; /* number of results */
}
static struct luaL_Reg lrLibs[] = {

    {"sin", l_sin},
    {NULL, NULL} /* sentinel */

};
extern "C" {
int luaopen_modules_helloworld_libhelloworld(lua_State* L) {
    lua_newtable(L);
    luaL_setfuncs(L, lrLibs, 0);
    return 1;
}
}