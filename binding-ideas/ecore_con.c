#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

#include <Ecore_Con.h>

#define WRAP_IN_USERDATA(L, type, var, luavar, bind, funcs)  \
    type *luavar = lua_newuserdata(L, sizeof(type));         \
    *luavar = var;                                           \
    bind;                                                    \
    if (luaL_newmetatable(L, #type)) {                       \
        luaL_register(L, NULL, funcs);                       \
    }                                                        \
    lua_setmetatable(L, -2);

static int
lua_ecore_con_server(lua_State *L)
{
   int type = luaL_checkint(L, 1);
   luaL_argcheck(L, type == ECORE_CON_LOCAL_USER, 1, "Illegal server type");
   const char *name = luaL_checkstring(L, 2);
   int port = luaL_checkint(L, 3);

   if (!ecore_con_init())
       return luaL_error(L, "Unable to initialize Ecore_Con");

   Ecore_Con_Server *server = ecore_con_server_add(type, name, port, NULL);
   if (!server) {
       ecore_con_shutdown();
       return luaL_error(L, "Unable to create Ecore_Con_Server");
   }

   WRAP_IN_USERDATA(L, Ecore_Con_Server *, server, lua_server,
                    ecore_con_server_data_set(server, "lua-object-idx", efl_bind(L, -1, "Ecore_Con_Server")),
                    ecore_con_funcs);

   Ecore_Con_Server **lua_server = lua_newuserdata(L, sizeof(Ecore_Con_Server*));
}

static int
lua_ecore_con_client_add_handler(lua_State *L)
{
    ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, add_callback, L);
}

static int luaL_reg
lua_ecore_con_contents[] = {
    { "server", lua_ecore_con_server },
    { "client_add_handler", lua_ecore_con_client_add_handler },
    { "client_del_handler", lua_ecore_con_client_del_handler },
    { "client_data_handler", lua_ecore_con_client_data_handler },
    { NULL, NULL },
};

LUALIB_API int
luaopen_ecore_con(lua_State *L)
{
    luaL_register(L, "ecore_con", lua_ecore_con_contents);
    lua_pushinteger(L, ECORE_CON_LOCAL_USER);
    lua_setfield(L, -2, "LOCAL_USER");
    lua_pushinteger(L, ECORE_CON_EVENT_CLIENT_ADD);
    lua_setfield(L, -2, "EVENT_CLIENT_ADD");
    lua_pushinteger(L, ECORE_CON_EVENT_CLIENT_DATA);
    lua_setfield(L, -2, "EVENT_CLIENT_DATA");
    lua_pushinteger(L, ECORE_CON_EVENT_CLIENT_DEL);
    lua_setfield(L, -2, "EVENT_CLIENT_DEL");
    return 1;
}
