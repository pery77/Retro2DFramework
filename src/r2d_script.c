#include "r2d/r2d.h"

#if R2D_HAS_LUAJIT
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#endif

#if R2D_HAS_LUAJIT

static bool R2D_ScriptCheck(R2D_Script *script)
{
    return script != 0 && script->state != 0;
}

static void R2D_ScriptLogError(R2D_Script *script, const char *action)
{
    lua_State *lua;
    const char *message;

    if (!R2D_ScriptCheck(script)) {
        R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "%s failed: script is not ready", action);
        return;
    }

    lua = (lua_State *)script->state;
    message = lua_tostring(lua, -1);
    R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "%s failed: %s", action, message != 0 ? message : "unknown Lua error");
    lua_pop(lua, 1);
}

bool R2D_ScriptAvailable(void)
{
    return true;
}

bool R2D_ScriptInit(R2D_Script *script)
{
    lua_State *lua;

    if (script == 0) {
        return false;
    }

    script->state = 0;
    script->loaded = false;

    lua = luaL_newstate();
    if (lua == 0) {
        R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "LuaJIT state could not be created");
        return false;
    }

    luaL_openlibs(lua);
    script->state = lua;
    script->loaded = false;
    return true;
}

void R2D_ScriptClose(R2D_Script *script)
{
    if (script == 0) {
        return;
    }

    if (script->state != 0) {
        lua_close((lua_State *)script->state);
    }

    script->state = 0;
    script->loaded = false;
}

bool R2D_ScriptIsReady(const R2D_Script *script)
{
    return script != 0 && script->state != 0;
}

void *R2D_ScriptState(R2D_Script *script)
{
    return script != 0 ? script->state : 0;
}

bool R2D_ScriptLoadFile(R2D_Script *script, const char *path)
{
    if (!R2D_ScriptCheck(script) || path == 0) {
        R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "load file failed: script is not ready");
        return false;
    }

    if (luaL_dofile((lua_State *)script->state, path) != 0) {
        R2D_ScriptLogError(script, "load file");
        return false;
    }

    script->loaded = true;
    return true;
}

bool R2D_ScriptDoString(R2D_Script *script, const char *source)
{
    if (!R2D_ScriptCheck(script) || source == 0) {
        R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "do string failed: script is not ready");
        return false;
    }

    if (luaL_dostring((lua_State *)script->state, source) != 0) {
        R2D_ScriptLogError(script, "do string");
        return false;
    }

    script->loaded = true;
    return true;
}

bool R2D_ScriptCall(R2D_Script *script, const char *function_name, int argument_count, int result_count)
{
    lua_State *lua;

    if (!R2D_ScriptCheck(script) || function_name == 0 || argument_count < 0 || result_count < 0) {
        R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "call failed: invalid script call");
        return false;
    }

    lua = (lua_State *)script->state;
    if (lua_gettop(lua) < argument_count) {
        R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "call failed: not enough arguments on the stack");
        return false;
    }

    lua_getglobal(lua, function_name);
    if (!lua_isfunction(lua, -1)) {
        lua_pop(lua, 1);
        R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "call failed: '%s' is not a function", function_name);
        return false;
    }

    if (argument_count > 0) {
        lua_insert(lua, -argument_count - 1);
    }

    if (lua_pcall(lua, argument_count, result_count, 0) != 0) {
        R2D_ScriptLogError(script, function_name);
        return false;
    }

    return true;
}

bool R2D_ScriptRegister(R2D_Script *script, const char *name, R2D_ScriptFunction function)
{
    lua_State *lua;

    if (!R2D_ScriptCheck(script) || name == 0 || function == 0) {
        return false;
    }

    lua = (lua_State *)script->state;
    lua_pushcfunction(lua, (lua_CFunction)function);
    lua_setglobal(lua, name);
    return true;
}

void R2D_ScriptPop(R2D_Script *script, int count)
{
    if (R2D_ScriptCheck(script) && count > 0) {
        lua_pop((lua_State *)script->state, count);
    }
}

int R2D_ScriptStackTop(R2D_Script *script)
{
    return R2D_ScriptCheck(script) ? lua_gettop((lua_State *)script->state) : 0;
}

void R2D_ScriptPushNumber(R2D_Script *script, double value)
{
    if (R2D_ScriptCheck(script)) {
        lua_pushnumber((lua_State *)script->state, (lua_Number)value);
    }
}

void R2D_ScriptPushBoolean(R2D_Script *script, bool value)
{
    if (R2D_ScriptCheck(script)) {
        lua_pushboolean((lua_State *)script->state, value ? 1 : 0);
    }
}

void R2D_ScriptPushString(R2D_Script *script, const char *value)
{
    if (R2D_ScriptCheck(script)) {
        lua_pushstring((lua_State *)script->state, value != 0 ? value : "");
    }
}

double R2D_ScriptToNumber(R2D_Script *script, int index, double fallback)
{
    lua_State *lua;

    if (!R2D_ScriptCheck(script)) {
        return fallback;
    }

    lua = (lua_State *)script->state;
    return lua_isnumber(lua, index) ? (double)lua_tonumber(lua, index) : fallback;
}

bool R2D_ScriptToBoolean(R2D_Script *script, int index, bool fallback)
{
    lua_State *lua;

    if (!R2D_ScriptCheck(script)) {
        return fallback;
    }

    lua = (lua_State *)script->state;
    return lua_isboolean(lua, index) ? lua_toboolean(lua, index) != 0 : fallback;
}

const char *R2D_ScriptToString(R2D_Script *script, int index, const char *fallback)
{
    lua_State *lua;
    const char *value;

    if (!R2D_ScriptCheck(script)) {
        return fallback;
    }

    lua = (lua_State *)script->state;
    value = lua_tostring(lua, index);
    return value != 0 ? value : fallback;
}

#else

bool R2D_ScriptAvailable(void)
{
    return false;
}

bool R2D_ScriptInit(R2D_Script *script)
{
    if (script != 0) {
        script->state = 0;
        script->loaded = false;
    }

    R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "LuaJIT support is not enabled");
    return false;
}

void R2D_ScriptClose(R2D_Script *script)
{
    if (script != 0) {
        script->state = 0;
        script->loaded = false;
    }
}

bool R2D_ScriptIsReady(const R2D_Script *script)
{
    (void)script;
    return false;
}

void *R2D_ScriptState(R2D_Script *script)
{
    (void)script;
    return 0;
}

bool R2D_ScriptLoadFile(R2D_Script *script, const char *path)
{
    (void)script;
    (void)path;
    R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "LuaJIT support is not enabled");
    return false;
}

bool R2D_ScriptDoString(R2D_Script *script, const char *source)
{
    (void)script;
    (void)source;
    R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "LuaJIT support is not enabled");
    return false;
}

bool R2D_ScriptCall(R2D_Script *script, const char *function_name, int argument_count, int result_count)
{
    (void)script;
    (void)function_name;
    (void)argument_count;
    (void)result_count;
    R2D_LogError(R2D_LOG_SUBSYSTEM_SCRIPT, "LuaJIT support is not enabled");
    return false;
}

bool R2D_ScriptRegister(R2D_Script *script, const char *name, R2D_ScriptFunction function)
{
    (void)script;
    (void)name;
    (void)function;
    return false;
}

void R2D_ScriptPop(R2D_Script *script, int count)
{
    (void)script;
    (void)count;
}

int R2D_ScriptStackTop(R2D_Script *script)
{
    (void)script;
    return 0;
}

void R2D_ScriptPushNumber(R2D_Script *script, double value)
{
    (void)script;
    (void)value;
}

void R2D_ScriptPushBoolean(R2D_Script *script, bool value)
{
    (void)script;
    (void)value;
}

void R2D_ScriptPushString(R2D_Script *script, const char *value)
{
    (void)script;
    (void)value;
}

double R2D_ScriptToNumber(R2D_Script *script, int index, double fallback)
{
    (void)script;
    (void)index;
    return fallback;
}

bool R2D_ScriptToBoolean(R2D_Script *script, int index, bool fallback)
{
    (void)script;
    (void)index;
    return fallback;
}

const char *R2D_ScriptToString(R2D_Script *script, int index, const char *fallback)
{
    (void)script;
    (void)index;
    return fallback;
}

#endif
