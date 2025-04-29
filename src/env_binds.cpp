
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

// clang-format off
#include "util.h"
#include "log.h"
#include "opts.h"
#include "wrap.h"
// clang-format on

#define trc std::cout << __LINE__ << std::endl

enum env_act
{
  get,
  del,
  set,
  ins,
  add
};

std::string env(webview_wrapper &w, const std::string &req, const env_act act = get) //
{
  // logTrace("REQ: ", req);
  std::string var, old_val;
  var = json_parse(req, "", 0);
  old_val = w.json_escape(my_getenv(var, true));

  if (act == get)
    return old_val;

  std::string new_val = "";
  if (act == del)
  {
    my_delenv(var);
  }
  else
  {
    new_val = json_parse(req, "", 1);

    switch (act)
    {
    case set:
      my_setenv(var, new_val);
      break;
    case ins:
      my_insenv(var, new_val);
      break;
    case add:
    default:
      my_addenv(var, new_val);
      break;
    }
  }

  // logTrace("VAR: ", var, ", OLD_VAL: ", old_val, ", NEW_VAL: ", w.json_escape(my_getenv(var, true)));
  return old_val;
}

void env(webview_wrapper &w, const std::string &seq, const std::string &req, const env_act act = get) //
{
  // logTrace("SEQ: ", seq);
  std::thread([&, seq, req, act] { w.resolve(seq, 0, env(w, req, act)); }).detach();
}

const std::string PATHSEP =
#ifdef _WIN32
    ";"
#else
    ":"
#endif
    ;

void clean_path(std::string &s)
{
  s = trim(s);
  s = trim(s, PATHSEP);
  s = trim(s);
  s = trim(s, PATHSEP);
}

const std::string path_var = "PATH";
std::string env_path(webview_wrapper &w, const std::string &req, const env_act act = get)
{

  std::string old_path_val = w.json_escape(my_getenv(path_var, true));
  if (act == get)
    return old_path_val;
  clean_path(old_path_val);

  if (act == del)
  {
    my_delenv(path_var);
  }
  else
  {
    // logTrace("REQ: ", req);
    std::string new_path_val = json_parse(req, "", 0);

    if (!new_path_val.empty())
    {
      clean_path(new_path_val);
      auto vpath = split(old_path_val, PATHSEP);
      bool do_chg = true;

      for (std::string p : vpath)
      {
        if (p == new_path_val)
          do_chg = false;
      }

      if (do_chg)
      {
        switch (act)
        {
        case set:
          my_setenv(path_var, new_path_val);
          break;
        case ins: {
          std::string path_val = my_getenv(path_var);
          if (!path_val.starts_with(PATHSEP))
            my_insenv(path_var, PATHSEP);
          my_insenv(path_var, new_path_val);
        }
        break;
        case add:
        default: {
          std::string path_val = my_getenv(path_var);
          if (!path_val.ends_with(PATHSEP))
            my_addenv(path_var, PATHSEP);
          my_addenv(path_var, new_path_val);
        }
        break;
        }
      }
    }
  }

  w.setvar("env", "path", my_getenv(path_var));
  return old_path_val;
}

void env_path(webview_wrapper &w, const std::string &seq, const std::string &req, const env_act act = get) //
{
  // logTrace("SEQ: ", seq);
  std::thread([&, seq, req, act] { w.resolve(seq, 0, env_path(w, req, act)); }).detach();
}

void create_env_binds(webview_wrapper &w)
{
  w.bind_doc(    //
      "env_get", // Is threaded
      [&](const std::string &seq, const std::string &req, void *) { env(w, seq, req, get); },
      "return the value of an environment variable.", //
      1);                                             //

  w.bind_doc(    //
      "env_set", // Is not
      [&](const std::string &req) -> std::string { return env(w, req, set); },
      "set an environment variable to a given value and return the previous one if it existed.", //
      2);                                                                                        //

  w.bind_doc(    //
      "env_ins", // Is not
      [&](const std::string &req) -> std::string { return env(w, req, ins); },
      "insert a value into an environment variable and return the previous one if it existed.", //
      2);                                                                                       //

  w.bind_doc(    //
      "env_add", // Is not
      [&](const std::string &req) -> std::string { return env(w, req, add); },
      "add a value to an environment variable and return the previous one if it existed.", //
      2);                                                                                  //

  w.bind_doc(    //
      "env_del", // Is not
      [&](const std::string &req) -> std::string { return env(w, req, del); },
      "delete an environment variable and return its last value if it existed.", //
      1);                                                                        //

  w.bind_doc(         //
      "env_get_path", // Is threaded
      [&](const std::string &seq, const std::string &req, void *) { env_path(w, seq, req, get); },
      "return the value of the PATH environment variable.", //
      0);                                                   //

  w.bind_doc(         //
      "env_add_path", // Is not
      [&](const std::string &req) -> std::string { return env_path(w, req, add); },
      "add a program path to the PATH env variable if is not yet added.", //
      1);                                                                 //

  w.bind_doc(         //
      "env_ins_path", // Is not
      [&](const std::string &req) -> std::string { return env_path(w, req, ins); },
      "insert a program path to the PATH env variable if is not yet added.", //
      1);                                                                    //

  w.bind_doc(         //
      "env_set_path", // Is not
      [&](const std::string &req) -> std::string { return env_path(w, req, set); },
      "replace the whole value of the PATH environment variable with the provided one.", //
      1);                                                                                //

  w.bind_doc(         //
      "env_del_path", // Is not
      [&](const std::string &req) -> std::string { return env_path(w, req, del); },
      "delete the value of the PATH environment variable.", //
      0);                                                   //

  w.decvar("env", "path", "actual value of the PATH environment variable", my_getenv(path_var));
}
