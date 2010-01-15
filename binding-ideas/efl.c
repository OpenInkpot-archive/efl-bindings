#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

#include <Evas.h>

static int
luaevas_object_cleanup(lua_State *L)
{
    Evas_Object **obj = lua_touserdata(L, 1);
    if (!*obj)
        return 0;
    evas_object_del(*obj);
    *obj = NULL;
    return 0;
}

static int
luaevas_object_show(lua_State *L)
{
    Evas_Object **obj = lua_touserdata(L, 1);
    if (!*obj)
        return luaL_argerror(L, 1, "Evas_Object* is NULL");
    evas_object_show(*obj);
    return 0;
}

static int
luaevas_object_hide(lua_State *L)
{
    Evas_Object **obj = lua_touserdata(L, 1);
    if (!*obj)
        return luaL_argerror(L, 1, "Evas_Object* is NULL");
    evas_object_hide(*obj);
    return 0;
}

static int
luaevas_object_evas(lua_State *L)
{
    Evas_Object **obj = lua_touserdata(L, 1);
    if (!*obj)
        return luaL_argerror(L, 1, "Evas_Object* is NULL");
    Evas *evas = evas_object_evas_get(*obj);
    get_luawrapper_evas(L, evas);
    return 1;
}

static int
luaevas_rectangle_tostring(lua_State *L)
{
    Evas **rect = lua_touserdata(L, 1);
    if (!*rect)
        return luaL_argerror(L, 1, "Evas_Object* is NULL");
    lua_pushfstring(L, "Evas_Rectangle<l:%p e:%p>", lua_topointer(L, 1), *rect);
    return 1;
}

static int
luaevas_rectangle_index(lua_State *L)
{
    Evas_Object **rect = lua_touserdata(L, 1);
    if (!*rect)
        return luaL_argerror(L, 1, "Evas_Object* is NULL");
    const char* fn = luaL_checklstring(L, 2, NULL);
    if (!strcmp(fn, "show"))
        lua_pushcfunction(L, luaevas_object_show);
    else if (!strcmp(fn, "hide"))
        lua_pushcfunction(L, luaevas_object_hide);
    else if (!strcmp(fn, "delete"))
        lua_pushcfunction(L, luaevas_object_cleanup);
    else if (!strcmp(fn, "evas"))
        lua_pushcfunction(L, luaevas_object_evas);
    else
        return luaL_error(L, "Unknown function: %s", fn);
    return 1;
}

static const luaL_reg
luaevas_rectangle_meta_funcs[] = {
    { "__gc", luaevas_object_cleanup },
    { "__tostring", luaevas_rectangle_tostring },
    { "__index", luaevas_rectangle_index },
    { NULL, NULL },
};

static int
luaevas_rectangle_add(lua_State *L)
{
    Evas **evas = lua_touserdata(L, 1);
    if (!*evas)
        return luaL_argerror(L, 1, "Evas* is NULL");

    Evas_Object **rect = lua_newuserdata(L, sizeof(Evas_Object*));
    *rect = evas_object_rectangle_add(*evas);
    if (!*rect)
        return luaL_error(L, "Unable to create Evas_Rectangle");

    lua_getglobal(L, "evas");
    lua_getfield(L, -1, "__objects");
    if (lua_isnil(L, -1)) {
         lua_pop(L, 1);
         lua_newtable(L);
         lua_pushstring(L, "v");
         lua_setfield(L, -1, "__mode");
    }
    size_t t_size = lua_objlen(L, -1);
    lua_pushvalue(L, -2); /* userdata */
    lua_rawseti(L, -1, t_size);
    lua_pop(L, 1); /* evas.__objects */

    evas_object_data_set(*rect, "lua-object-idx", t_size + 1);

    if (luaL_newmetatable(L, "evas_rectangle")) {
        luaL_register(L, NULL, luaevas_rectangle_meta_funcs);
    }
    lua_setmetatable(L, -2);

    return 1;
}

static int
luaevas_cleanup(lua_State *L)
{
    Evas **evas = lua_touserdata(L, 1);
    if (!*evas)
        return 0;
    evas_free(*evas);
    *evas = NULL;
    return 0;
}

static int
luaevas_tostring(lua_State *L)
{
    Evas **evas = lua_touserdata(L, 1);
    if (!*evas)
        return luaL_argerror(L, 1, "Evas* is NULL");
    lua_pushfstring(L, "Evas<l:%p e:%p>", lua_topointer(L, 1), *evas);
    return 1;
}

static int
luaevas_index(lua_State *L)
{
    Evas **evas = lua_touserdata(L, 1);
    if (!*evas)
        return luaL_argerror(L, 1, "Evas* is NULL");
    const char* fn = luaL_checklstring(L, 2, NULL);
    if (!strcmp(fn, "rectangle"))
        lua_pushcfunction(L, luaevas_rectangle_add);
    else if (!strcmp(fn, "delete"))
        lua_pushcfunction(L, luaevas_cleanup);
    else
        return luaL_error(L, "Unknown function: %s", fn);
    return 1;
}

static const luaL_reg
luaevas_meta_funcs[] = {
    { "__gc", luaevas_cleanup },
    { "__tostring", luaevas_tostring },
    { "__index", luaevas_index },
    { NULL, NULL },
};

static int
luaevas_put_evas_object_into_table(lua_State *L, int index)
{
}

/* Pushes on stack lua wrapper for Ecore_Evas */
static void
create_luawrapper_ecore_evas(lua_State *L, Ecore_Evas *ecore_evas)
{
    lua_getglobal(L, "__efl_bind");

    Ecore_Evas **lua_ecore_evas = lua_newuserdata(L, sizeof(Ecore_Evas*));
    *lua_ecore_evas = ecore_evas;

    int ref = luaL_ref(L, -2);
    ecore_evas_data_set(ecore_evas, "lua-object-idx", ref);

    if (luaL_newmetatable(L, "ecore_evas")) {
        luaL_register(L, NULL, luaevas_meta_funcs);
    }
    lua_setmetatable(L, -2);

    lua_remove(L, -2);
}

/* Pushes on stack lua wrapper around Ecore_Evas */
static void
get_luawrapper_ecore_evas(lua_State *L, Ecore_Evas *ecore_evas)
{
    int idx = (int)ecore_evas_data_get(ecore_evas, "lua-object-idx");
    /* FIXME: assert here */
    lua_getglobal(L, "__efl_bind");
    lua_rawgeti(L, -1, idx);
    lua_remove(L, -2);

    if (!lua_isuserdata(L, -1)) {
        lua_pop(L, 1);
        create_luawrapper_ecore_evas(L, Ecore_Evas *ecore_evas);
    }
}

static int
efl_ecore_evas_new(lua_State *L)
{
    if (!ecore_evas_init())
        return luaL_error("Unable to initialize Ecore_Evas");

    /* FIXME */
    Ecore_Evas *ecore_evas = ecore_evas_software_x11_new(0, 0, 0, 0, 600, 800);
    if (!*ecore_evas)
        return luaL_error("Unable to create Ecore_Evas");

    create_luawrapper_ecore_evas(L, ecore_evas);

    return 1;
}

static const luaL_reg
efl_ecore_evas[] = {
    { "new", luaevas_new },
    { NULL, NULL },
};

LUALIB_API int
luaopen_evas(lua_State *L)
{
    lua_newtable(L);
    lua_setglobal(L, "__efl_bind");
    lua_newtable(L);
    lua_pushliteral(L, "v");
    lua_setfield(L, -2, "mode");
    lua_setmetatable(L, -2);

    luaL_register(L, "ecore_evas", efl_ecore_evas);
    return 1;
}
