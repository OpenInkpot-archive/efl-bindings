#!/usr/bin/lua
require "lxp"

local modname = "Unknown"
local modversion = "Unknown"
local description = "Unknown"
local copyright = "Unknown"
local typemaps = {}
local funcs = {}
local pos_param = 0
local current_f = nil
local retval_type = nil
local call_str = ""
local marshalling = ""
local demarshalling = ""

local function PositionalParam()
    pos_param = pos_param + 1
    if pos_param > 1 then
        call_str = call_str .. ", "
    end
    return string.format("param_%d", pos_param)
end

local function Prologue()
    io.write("/* This file is auto generated */\n")
    io.write("#include <libelua.h>\n\n")
end

local function Include(name)
    io.write(string.format("#include \"%s\"\n", name))
end

local function Define(name, call)
    current_f = name
    void = true
    if call then
        call_str = call
    else
        call_str = name
    end
    call_str = string.format("%s (", call_str)
    local fname = string.format("l_%s", name)
    funcs[ name ] = fname
    io.write(string.format("static int %s(lua_State *L) {\n", fname))
    pos_param = 0
end

local function EndDefine(name)
    if retval_type then
        io.write(string.format("%s retval;\n", retval_type))
    end
    io.write(marshalling)
    if retval_type then
        io.write("retval = ")
    end
    io.write(call_str)
    io.write(");\n")
    io.write(demarshalling)
    if retval_type then
        io.write("return retval;\n")
    end
    io.write("};\n")
    demarshalling = ""
    marshalling = ""
    retval_type = nil
end

local function Typemap(typename, ext)
    typemaps[typename] = ext
end

local function In(typename, ext)
    if not ext then ext = typemaps[typename] end
    argname = PositionalParam()
    local s
    s = string.format("%s %s = elua_lua2%s(L, %d);\n", ext, argname, typename, pos_param)
    marshalling = marshalling .. s
    call_str = call_str .. string.format("%s", argname)
end

local function Out(typename, ext)
    argname = PositionalParam()
    if not ext then ext = typemaps[typename] end
    local s
    s = string.format("%s %s;\n", ext, argname);
    marshalling = marshalling .. s
    s = string.format("elua_%s2lua(L, %s);\n", typename, argname)
    demarshalling = demarshalling .. s
    call_str = call_str .. string.format(", &%s", argname)
end

local function Return(typename, ext)
    if not ext then ext = typemaps[typename] end
    retval_type = ext
    local s 
    s = string.format("elua_%s2lua(L, retval);\n", typename)
    demarshalling = demarshalling .. s
end

local function Modinfo(attrs)
    modname = attrs["modname"]
    modversion = attrs["version"]
    description = attrs["description"]
    copyright = attrs["copyright"]
    if not copyright then copyright = "Uncopyrighted" end
    if not description then description = "Unknown" end
    if not modversion then modversion = "Unknown" end
    if not modname then modname = "Unknown" end
end

local function Epilogue()
    io.write(string.format("static struct luaL_reg %s_lib[] = {\n", modname))
    for k, v in pairs(funcs) do
        io.write(string.format("{\"%s\", %s },\n", k, v))
    end
    io.write("{ NULL, NULL }\n")
    io.write("};\n\n\n")
    io.write(string.format("int luaopen_%s (lua_State *L) {\n", modname))
    io.write(string.format("luaL_openlib(L, \"%s\", %s_lib, 0);\n", modname, modname))
    io.write(string.format("elua_set_info(L, \"%s\", \"%s\", \"%s\");\n",
        copyright, description, modversion))
    io.write("}'\n")
end

local callbacks = {
    StartElement = function (parser, name, attrs)
        if name == "bind" then Prologue()
        elseif name == "include" then Include(attrs["file"])
        elseif name == "modinfo" then Modinfo(attrs)
        elseif name == "typemap" then Typemap(attrs["type"], attrs["ext"])
        elseif name == "define" then Define(attrs["name"], attrs["call"])
        elseif name == "in" then In(attrs["type"], attrs["ext"])
        elseif name == "out" then Out(attrs["type"], attrs["ext"])
        elseif name == "return" then Return(attrs["type"], attrs["ext"])
        end
    end,
    EndElement = function(parser, name)
       if name == "bind" then Epilogue()
       elseif name == "define" then EndDefine()
       end
    end

}

local p = lxp.new(callbacks)
local f, e = io.open("definition/evas.def", "r")
if f then
    for l in f:lines() do  -- iterate lines
        p:parse(l)          -- parses the line
        p:parse("\n")       -- parses the end of line
    end
    f:close()
else
    error(e)
end
p:parse()               -- finishes the document
p:close() 

