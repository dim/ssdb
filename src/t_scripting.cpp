#include "t_scripting.h"

/* Lua stdlib */
static
const luaL_Reg luaStdLib[] = {
  {"_G", luaopen_base},
  {"table", luaopen_table},
  {"string", luaopen_string},
  {"math", luaopen_math},
  {"bit", luaopen_bit32},
  {"debug", luaopen_debug},
  {NULL, NULL}
};

/* SSDB methods, available in scripts */
static
const luaL_Reg luaMethods[] = {
    {"get", script_lua_get},
    {"incr", script_lua_incr},
    { NULL, NULL }
};

/* initialize lua state */
lua_State* SSDB::luaInit(){
  // Init state
  lua_State *lua = luaL_newstate();

  // Load Lua stdlib
  const luaL_Reg *lib = luaStdLib;
  for(; lib->func != NULL; lib++){
    luaL_requiref(lua, lib->name, lib->func, 1);
    lua_settop(lua, 0); // empty the stack
  }

  // Disable functions we don't want to expose
  lua_pushnil(lua);
  lua_setglobal(lua, "loadfile");
  lua_pushnil(lua);
  lua_setglobal(lua, "require");

  // Initialize the ssdb accessors
  SSDB** udata = (SSDB**) lua_newuserdata(lua, sizeof(SSDB*));
  *udata = this;

  luaL_newmetatable(lua, SSDB_LUA_MT);
  luaL_newlib(lua, luaMethods);
  lua_setfield(lua, -2, "__index");
  lua_setmetatable(lua, -2);
  lua_setglobal(lua, "ssdb");

  return lua;
}


// eval
int SSDB::eval(const Bytes &code, const std::vector<Bytes> &args, int offset, std::vector<std::string> *list) const{
  std::string fname = script_sha1(code);
  std::string func  = script_wrap(code, fname);

  // Load function, exit on error
  if (luaL_loadbuffer(lua, func.c_str(), func.size(), "@user_script")) {
    list->push_back(script_error("failed compiling script", fname));
    lua_pop(lua,1);
    return -1;
  }

  // Call function, exit on error
  if (lua_pcall(lua,0,0,0)) {
    list->push_back(script_error("failed running script", fname));
    lua_pop(lua,1);
    return -1;
  }

  // Find script, exit on error
  lua_getglobal(lua, fname.c_str());
  if (lua_isnil(lua,1)) {
    list->push_back(script_error("no script", fname));
    lua_pop(lua,1);
    return -1;
  }

  // Load arguments into a table
  std::vector<Bytes>::const_iterator it;
  it = args.begin() + offset;

  lua_newtable(lua);
  int tix = lua_gettop(lua);
  int aix = 1;

  for(; it != args.end(); it++){
    std::string s = (*it).String();
    lua_pushlstring(lua, s.c_str(), s.size());
    lua_rawseti(lua, tix, aix);
    ++aix;
  }

  // Execute function with args
  int err = lua_pcall(lua, 1, 1, 0);
  lua_gc(lua, LUA_GCSTEP, 1);
  if (err) {
    list->push_back(script_error("failed running script", fname));
    lua_pop(lua, 1);
    return -1;
  }

  // Read response
  int len;
  switch(lua_type(lua,-1)) {
  case LUA_TSTRING:
    list->push_back(lua_tostring(lua,-1));
    break;
  case LUA_TBOOLEAN:
    list->push_back(lua_toboolean(lua,-1) ? "1" : "0");
    break;
  case LUA_TNUMBER:
    list->push_back(double_to_str(lua_tonumber(lua,-1)));
    break;
  case LUA_TTABLE:
    len = lua_rawlen(lua,-1);
    for(int i=0;i < len; i++) {
      lua_rawgeti(lua, -1, i+1);

      switch(lua_type(lua,-1)) {
      case LUA_TSTRING:
      case LUA_TNUMBER:
        list->push_back(lua_tostring(lua,-1));
        break;
      case LUA_TBOOLEAN:
        list->push_back(lua_toboolean(lua,-1) ? "1" : "0");
        break;
      }
      lua_pop(lua,1);
    }
    break;
  default:
    lua_pop(lua, 1);
    return 0;
  }
  lua_pop(lua,1);

  return 1;
}

// get
int script_lua_get(lua_State *lua) {
  SSDB** ssdb = static_cast<SSDB**>(luaL_checkudata(lua, 1, SSDB_LUA_MT));
  const char* key = luaL_checkstring(lua, 2);

  std::string val;
  if ((*ssdb)->get(key, &val)) {
    lua_pushstring(lua, val.c_str());
    return 1;
  }
  return 0;
}

// incr
int script_lua_incr(lua_State *lua) {
  SSDB** ssdb = static_cast<SSDB**>(luaL_checkudata(lua, 1, SSDB_LUA_MT));
  const char* key = luaL_checkstring(lua, 2);

  int64_t by = 1;
  if(lua_gettop(lua) > 2) {
    by = (int64_t) luaL_checklong(lua, 3);
  }

  std::string val;
  if ((*ssdb)->incr(key, by, &val)) {
    lua_pushstring(lua, val.c_str());
    return 1;
  }
  return 0;
}