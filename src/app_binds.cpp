
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
#include <vector>

// clang-format off
#include "util.h"
#include "log.h"
#ifdef _WIN32
#include "winapi.h"
#endif
#include "opts.h"
#include "wrap.h"
// clang-format on

std::string app_title(webview_wrapper &w, const std::string &req)
{
  std::string new_title, old_title = w.get_title();
  js_params(req, new_title);
  if (new_title != old_title && !new_title.empty())
  {
    w.set_title(new_title);
  }

  return (w.json_escape(old_title));
}

// Return true if s is not a DOS command call (cmd /C)
// Pour console powershell, voir par là : https://devblogs.microsoft.com/scripting/use-powershell-to-interact-with-the-windows-api-part-1/
bool not_cmd(std::string s)
{
  // Try to guess if calling a DOS command
  if (ifind(s, "cmd.exe /c") || ifind(s, "cmd /c") || ifind(s, "powershell.exe -c") || ifind(s, "powershell -c"))
    return false;

  return true;
}

bool js_parse_nki(const std::string &req, const int index, int &n)
{
  auto s = json_parse(req, "", index);
  if (s.empty())
    return false;
  n = std::stoi(s);
  return true;
}

#define JPRSI(index, n) js_parse_nki(req, index, n)

#define trc logTrace(__FILE__, ':', __LINE__);
void create_app_binds(webview_wrapper &w)
{
  w.bind_doc(                                    //
      "app_echo",                                //
      [&](const std::string &req) -> std::string //
      {
        std::string p[10], ret = {};
        js_params(req, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9]);
        for (int i = 0; i < 10; i++)
        {
          if (!p[i].empty())
          {
            ret += p[i];
            if (i < 9)
              ret += ", ";
          }
        }

        logTrace(ret);
        std::cout << ret << std::endl;
        return w.json_escape(ret);
      },
      "echo the parameter(s) and display on stdout", //
      -10);                                          //

  w.bind_doc(
      "app_bring_to_top",                        //
      [&](const std::string &req) -> std::string //
      {
        std::string ret = "false";
#ifdef _WIN32
        std::string win_id = json_parse(req, "", 0);

        HWND hw;
        if (win_id.empty())
        {
          hw = (HWND)w.window();
        }
        else
        {
          long long ll;
          if (try_stoll(win_id, ll))
          {
            hw = (HWND)ll;
          }
          else
          {
            hw = (HWND)w.window();
          }
        }

        if (BringWindowToTop(hw))
          ret = "true";
#endif
        return ret;
      },
      "Bring the window whose id is provided (or by default the app window) to the top of the z order.", //
      -1);

  w.bind_doc(
      "app_restore",                             //
      [&](const std::string &req) -> std::string //
      {
        w.exit_fullscreen();
        w.restore();
        w.setvar("app", "state", "normal");
        return "";
      },
      "restore the webapp window." //
  );

  w.bind_doc(
      "app_minimize",                            //
      [&](const std::string &req) -> std::string //
      {
        w.exit_fullscreen();
        w.minimize();
        w.setvar("app", "state", "minimized");
        return "";
      },
      "minimize the webapp window." //
  );

  w.bind_doc(
      "app_maximize",                            //
      [&](const std::string &req) -> std::string //
      {
        w.exit_fullscreen();
        w.maximize();
        w.setvar("app", "state", "maximized");
        return "";
      },
      "maximize the webapp window." //
  );

  w.bind_doc(
      "app_enter_fullscreen",                    //
      [&](const std::string &req) -> std::string //
      {
        w.enter_fullscreen();
        w.setvar("app", "state", "fullscreen");
        return "";
      },
      "set the webapp window in fullscreen mode." //
  );

  w.bind_doc(
      "app_exit_fullscreen",                     //
      [&](const std::string &req) -> std::string //
      {
        w.exit_fullscreen();
        w.restore();
        w.setvar("app", "state", "normal");
        return "";
      },
      "exit the webapp window from fullscreen mode." //
  );

  w.bind_doc(
      "app_show",                                //
      [&](const std::string &req) -> std::string //
      {
        w.exit_fullscreen();
        w.restore();
        w.show();
        w.setvar("app", "state", "normal");
        return "";
      },
      "show the webapp window." //
  );

  w.bind_doc(
      "app_hide",                                //
      [&](const std::string &req) -> std::string //
      {
        w.hide();
        w.setvar("app", "state", "hidden");
        return "";
      },
      "hide the webapp window." //
  );

  // Get and eventually set app title
  w.bind_doc(
      "app_set_title",                                                                      //
      [&](const std::string &req) -> std::string { return app_title(w, req); },             //
      "return the actual app title and change it if a new one is provided as a parameter.", //
      -1);
  // Change or get window position
  w.bind_doc(
      "app_set_pos",                             //
      [&](const std::string &req) -> std::string //
      {
        // Get actual position
        int px, py;
        w.exit_fullscreen();
        w.restore();
        w.get_pos(px, py);
        std::string res = "{\"x\": " + std::to_string(px) + ", \"y\": " + std::to_string(py) + "}";
        js_parami(req, px, py);
        w.set_pos(px, py);
        return res;
      },
      "return the actual app position as a json object (x, y) and if there are parameters then change it.", //
      -2);

  w.bind_doc(
      "app_center", //
      [&](const std::string &req) -> std::string {
        w.exit_fullscreen();
        w.restore();
        w.center();
        w.setvar("app", "state", "normal");
        return "";
      },                                                                                                             //
      "center the window on the screen, the x and y position and width and height will never be out of the screen.", //
      0);

  w.bind_doc(
      "app_set_size", //
      [&](const std::string &req) -> std::string {
        int pw, ph, phn = 0;
        js_parami(req, pw, ph, phn);
        //        if (phn == 0 || phn == 4) { w.exit_fullscreen(); w.restore(); }
        w.set_size(pw, ph, phn);
        w.setvar("app", "state", "normal");
        // Return previous dimensions
        std::string res = "{\"w\": " + std::to_string(pw) + ", \"h\": " + std::to_string(ph) + "}";
        return res;
      },
      "change the app size with the first 2 provided parameters, a third optional parameter may be added to indicate if you want to define one of the following:  0 change the the app window size (this is the default feature).  1 fix a minimum size "
      "bounds.  2 fix a maximum size bounds.  3 set a fixed size. 4 change the app size without taking into account its window borders (default feature is to take it). Return the effectively applied size as a json array [ w, h ]", //
      3);

#ifdef _WIN32
  // Get window size
  w.bind_doc(
      "app_border_size", //
      [&](const std::string &req) -> std::string {
        int l, t, r, b;
        w.border_size(l, t, r, b);
        return '[' + std::to_string(l) + ',' + std::to_string(t) + ',' + std::to_string(r) + ',' + std::to_string(b) + ']';
      },
      "return the border size as a json array [ left, top, right, bottom ].", //
      0);
#endif

  // Change window geoemtry
  w.bind_doc(
      "app_set_geometry", //
      [&](const std::string &req) -> std::string {
        w.exit_fullscreen();
        w.restore();
        int gx, gy, gw, gh;
        js_parami(req, gx, gy, gw, gh);
        w.set_geometry(gx, gy, gw, gh);
        w.setvar("app", "state", "normal");
        return "";
      },
      "return the actual app position and size as a json object (x, y, w, h) and if there are parameters then change it.", //
      -4);

  // Change window icon
  w.bind_doc(
      "app_set_icon",                            //
      [&](const std::string &req) -> std::string //
      {                                          //
        std::string prev_ico = "";
        std::string ico = "";
        js_params(req, ico);
        prev_ico = w.set_icon(ico);
        return w.json_escape(prev_ico);
      },
      "set window top left icon and return the previous one if thre was.", //
      -1);

  // Set a move callback
  w.bind_doc(
      "app_on_geometry",                         //
      [&](const std::string &req) -> std::string //
      {
        auto js = json_parse(req, "", 0);
        w.set_on_geometry(js);
        return "";
      },
      "set callback to detect when webapp has moved.", //
      -1);

  // Set an exit callback
  w.bind_doc(
      "app_on_exit",                             //
      [&](const std::string &req) -> std::string //
      {
        auto js = json_parse(req, "", 0);
        w.set_on_exit(js);
        return "";
      },
      "set callback to detect when webapp is exiting.", //
      -1);

  // Exit
  w.bind_doc(
      "app_exit",                             //
      [&](const std::string &) -> std::string //
      {
        w.terminate();
        return "";
      },
      "exit from webapp." //
  );

  w.bind_doc(
      "app_system",                                               //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::string cmd = json_parse(req, "", 0), res;

          if (not_cmd(cmd))
          {
            // logTrace("External program");
            res = exec_cmd(cmd);
          }
          else
          {
            // logTrace("DOS command");
            res = wexec_cmd(s2ws(cmd));
            res = CP1252_to_UTF8(res);
          }

          w.resolve(seq, 0, w.json_escape(res));
        }).detach();
      },
      "run an external command in a thread and return its output.", //
      1);

  w.bind_doc(
      "app_pipe",                                                 //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::string s = json_parse(req, "", 0);
          std::string res = pipe2s(s);
          w.resolve(seq, 0, w.json_escape(res));
        }).detach();
      },
      "run an external command in a pipe and return its output.", //
      1);

#ifdef _WIN32
  w.bind_doc(
      "app_wpipe",                                                //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          auto s = json_parse(req, "", 0);
          std::string res = wpipe2s(s2ws(s));
          w.resolve(seq, 0, w.json_escape(res));
        }).detach();
      },
      "run an external command in a wstring pipe and return its URI encoded output.", //
      1);

  w.bind_doc(
      "app_shell",                                                //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::string cmd = json_parse(req, "", 0), output;
          // logTrace("SH CMD: ", cmd);

          if (not_cmd(cmd))
          {
            // logTrace("External program");
            shell_cmd(cmd, output, false);
          }
          else
          {
            // logTrace("DOS command", cmd);
            wshell_cmd(s2ws(cmd), output, false);
            output = CP1252_to_UTF8(output);
          }

          // logTrace("SH RES: ", output);
          w.resolve(seq, 0, w.json_escape(output));
        }).detach();
      },
      "open an external windows command in a thread (See ShellExecute) and return its output.", //
      1);
#endif

  // logTrace("Enter app decvars");
  // Set class vars
  std::string sys_info;
#if defined(__APPLE__)
  sys_info = "macOS or iOS code",
#elif defined(__unix__)
#if defined(__linux__)
  sys_info = "Linux";
#endif
  sys_info = "undefined Unix-like system";
#elif defined(_WIN32) || defined(_WIN64)
  sys_info = "Windows";
#else
  sys_info = "Unknown operating system";
#endif

  std::string win_id = std::to_string((unsigned long long)w.window());

  w.decvar("app", "window_id", "window id.", win_id);
  w.decvar("app", "sysname", "provide the name of the system, this may help to assert in your javascript code that you are running a webapp, something like <u>console.log(typeof app.system === 'undefined'?'not a webapp':'is a webapp')</u>.", sys_info);

  int appx, appy, appw, apph;
  w.get_pos(appx, appy);
  w.get_size(appw, apph);

  w.decvar("app", "title", "title of the webapp window, usually displayed one its title bar.", std::filesystem::path(w.get_title()));
  w.decvar("app", "icon", "file name of the icon used by the webapp.", std::filesystem::path(w.icon_file));
  w.decvar("app", "info", "informations about the webapp.", appInfo());

  w.decvar("app", "x", "horizontal position of the upper left corner of the webapp window.", appx);
  w.decvar("app", "y", "vertical position of the upper left corner of the webapp window.", appy);
  w.decvar("app", "w", "horizontal position of the lower left corner of the webapp window.", appw);
  w.decvar("app", "h",
           "vertical position of the upper left corner of the webapp window.<br/>All these coordinate variables are updated in real time. To assert it, try something like:<br/>"
           "<u>(async()=>{ elt=document.createElement('i'); document.body.appendChild(elt); new Promise(function (resolve, reject) { (function _(){ coords=`(${app.x}, ${app.y}, ${app.w}, ${app.h})`; "
           "if (elt.innerText != coords) { elt.innerText=coords; } timeoutId=setTimeout(_, 10); })(); }) })()</u><br/>"
           "And to stop it, later:<br/><u>clearTimeout(timeoutId); elt.remove()</u>",
           apph);
#ifdef _WIN32
  int l_b, t_b, r_b, b_b;
  w.border_size(l_b, t_b, r_b, b_b);
  w.decvar("app", "left_border", "left size of the horizontal border.", l_b);
  w.decvar("app", "top_border", "top size of the vertical border.", t_b);
  w.decvar("app", "right_border", "right size of the horizontal border.", r_b);
  w.decvar("app", "bottom_border", "bottom size of the vertical border.", b_b);
#endif

  logDebug("URL JS ARGS: ", w.js_args);
  w.decvar("app", "args", "", w.js_args);
  w.decvar("app", "state", "contains the state of the application windows, may have one of the following values: normal, maximised, minimised, hidden, full_screen", "normal");
  w.decvar("app", "last_error", "may contains the last app object error", "No error with application object");
}
