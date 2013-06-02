#include "t_scripting.h"

// eval
int SSDB::eval(const Bytes &code, const std::vector<Bytes> &args, int offset, std::string *val) const{
  std::string fname = script_sha1(code);
  std::string func  = script_wrap(code, fname);

  // Load function, exit on error
  if (luaL_loadbuffer(lua, func.c_str(), func.size(), "@user_script")) {
    *(val) = script_error("failed compiling script", fname);
    lua_pop(lua,1);
    return -1;
  }

  // Call function, exit on error
  if (lua_pcall(lua,0,0,0)) {
    *(val) = script_error("failed running script", fname);
    lua_pop(lua,1);
    return -1;
  }

  // Find script, exit on error
  lua_getglobal(lua, fname.c_str());
  if (lua_isnil(lua,1)) {
    *(val) = script_error("no script", fname);
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
    *(val) = script_error("failed running script", fname);
    lua_pop(lua, 1);
    return -1;
  }

  // Read response
  switch(lua_type(lua,-1)) {
  case LUA_TSTRING:
    (*val).append(lua_tostring(lua,-1),lua_rawlen(lua,-1));
    break;
  case LUA_TBOOLEAN:
    *(val) = lua_toboolean(lua,-1) ? "1" : "0";
    break;
  case LUA_TNUMBER:
    *(val) = double_to_str(lua_tonumber(lua,-1));
    break;
  // case LUA_TTABLE:
  //   // TODO
  //   break;
  default:
    lua_pop(lua, 1);
    return 0;
  }
  lua_pop(lua,1);

  return 1;
}

