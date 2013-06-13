#ifndef SSDB_SCRIPTING_H_
#define SSDB_SCRIPTING_H_

#include "ssdb.h"
#include "util/sha1.h"

#define SSDB_LUA_MT "SSDB__MT"

static int script_lua_get(lua_State *lua);
static int script_lua_incr(lua_State *lua);

static inline
std::string script_sha1(const Bytes &code){
  unsigned char hash[20];
  char hex[41];
  sha1::calc(code.data(), code.size(), hash);
  sha1::toHexString(hash, hex);

  std::string buf;
  buf.append("f_", 2);
  buf.append(hex, 41);
  return buf;
}

static inline
std::string script_wrap(const Bytes &code, std::string name){
  std::string buf;
  buf.append("function ", 9);
  buf.append(name.data(), name.size()-1);
  buf.append("(args) ", 7);
  buf.append(code.data(), code.size());
  buf.append(" end", 4);
  return buf;
}

static inline
std::string script_error(std::string msg, std::string name){
  std::string buf;
  buf.append(msg.data(), msg.size());
  buf.append(" ", 1);
  buf.append(name.data(), name.size()-1);
  return buf;
}

#endif
