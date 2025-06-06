
#ifdef _WIN32
#include "winapi.h"
#include <fileapi.h>
#include <windows.h>
#endif

#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

// clang-format off
#include "log.h"
#include "options.h"
#include "util.h"
#include "wrap.h"
#include "app_binds.h"
#include "env_binds.h"
#include "fs_binds.h"
#include "io_binds.h"
#include "gtk_binds.h"
#include "win_binds.h"
// clang-format on

#ifndef _WIN32
typedef void *HWND;
#endif

options myopt;

bool devmode = false, runjs_and_exit = false, html_string = false, call_func_help = false;
std::string url, title = "", init_js = "";
webview_wrapper w;
int ix = 200, iy = 200, iw = 600, ih = 400, hint = 0;

std::string add_bro_args = "";
void set_browser_args(s_opt_params &p)
{
  // logTrace("Adding browser argument [--", val, "]");
  add_bro_args += " --" + p.val;
}

void set_url(s_opt_params &p)
{
  if (starts_with(p.val, "http:") || starts_with(p.val, "https:"))
  {
    url = p.val;
  }
  else
  {
    url = std::filesystem::absolute(p.val).generic_string();
  }

  // logTrace("set_url ", url);
  if (title.empty())
    title = url;

  p.ret = true;
}

void ins_html(s_opt_params &p)
{
  url = "html://";
  url += p.val;
  html_string = true;
  if (title.empty())
    title = "HTML string";
}

// int xpos = -1, ypos = -1, width = -1, height = -1;
int hints = -1;

win_state init_win_state = win_state::normal;
std::string icon_file = "";

std::string idxs = "webview.min.html, webview.html, index.min.html, index.html, index.min.js, index.js";
std::string get_index()
{
  std::vector<std::string> idxv = split(idxs, ",");
  std::string all_idxs = "";
  for (auto &s : idxv)
  {
    s = trim(s);
    std::string idx = std::filesystem::absolute(s).generic_string();
    if (std::filesystem::is_regular_file(idx))
    {
      // logTrace("Found index file: ", idx);
      return idx;
    }
  }

  logWarn("Couldn't find in the current path an index file with one of the following name: ", idxs, '.');
  return "";
}

void set_debug_mode(s_opt_params &p)
{
  devmode = true;
  std::string ll = my_getenv("LOG");

  if (ll.empty())
  {
    my_setenv("LOG", "DEBUG");
  }
}

void set_log_level(s_opt_params &p)
{
  my_setenv("LOG", p.val);
  // Will check and eventually correct the log level env var if it is not correctly set
  check_log_level();
}

void chg_ini_geom(s_opt_params &p)
{
  auto scoo = split(p.val, ',');
  if (scoo.size() > 0)
    ix = std::stoi(scoo[0]);
  if (scoo.size() > 1)
    iy = std::stoi(scoo[1]);
  if (scoo.size() > 2)
    iw = std::stoi(scoo[2]);
  if (scoo.size() > 3)
    ih = std::stoi(scoo[3]);
  if (scoo.size() > 4)
    hint = std::stoi(scoo[4]);
}

void set_path(s_opt_params &p)
{
  std::filesystem::path path(p.val), parent_path;
  std::error_code ec;

  if (std::filesystem::is_regular_file(path))
  {
    // Si c'est un fichier on découpe le dirname et basename
    parent_path = path.parent_path();
    url = std::filesystem::absolute(path).generic_string();
  }
  else if (std::filesystem::is_directory(path))
  {
    // Sinon c'est le dirname
    parent_path = path;
    // Et on ira chercher un basename dans le parent_path.
    url.clear();
  }
  else
  {
    std::cerr << "Neither a regular path for a file nor for a directory." << std::endl;
    return;
  }

  std::filesystem::current_path(parent_path, ec);

  if (ec.value() != 0)
  {
    std::cerr << "Problem when changing directory to " << parent_path.generic_string() << ". " << ec.message() << std::endl;
  }
  else
  {
    // logTrace("PATH: ", std::filesystem::current_path(), ", URL: ", url);
    if (url.empty())
      url = get_index();
  }
}

HWND *wnd = nullptr;

void webview_bind()
{
  // logTrace("Enter webview_bind");
  create_app_binds(w);
  create_env_binds(w);
  create_fs_binds(w);
  create_io_binds(w);
  create_gtk_binds(w);
  create_win_binds(w);
  w.bind_classes();
}

bool run_and_exit = false;
// extern bool help_or_version;
void webview_set(win_state init_win_state, bool devmode = false, bool _run_and_exit = false)
{
  // logTrace("Enter webview_set");

  //  void *wnd = nullptr;
  run_and_exit = _run_and_exit;

#ifdef _WIN32 // A revoir ...
  if (run_and_exit)
  {
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
      // Console mode, webview window will not be rendered.
      // HWND hwnd; extern HWND CreateWin(); hwnd=CreateWin(); wnd=&hwnd;
    }
    else
    {
      // GUI mode is not compatible with run_and_exit option
      run_and_exit = false;
      devmode = true;
    }
  }
#endif
  // if (w) w.conf.debug = devmode;
  logDebug("title: ", title);
  w.conf = {init_win_state, devmode, true, true, true, true, true, ix, iy, iw, ih, hint};
  if (call_func_help /*|| help_or_version */ || title == "Missing parameter")
    w.conf.init_win_state = hidden;
  w.create((void *)wnd);
  logDebug("bef webview_bind");
  webview_bind();
  logDebug("aft webview_bind");
}

void GetGeom(HWND hw, int &x, int &y, int &wi, int &he)
{
#ifdef _WIN32
  RECT rc;
  GetWindowRect(hw, &rc);
  x = rc.left;
  y = rc.top;
  wi = rc.right - rc.left;
  he = rc.bottom - rc.top;
  // logTrace("GetWindow x ", x, ", y ", y, ", w ", wi, ", h ", h);
#endif
}

#define trc std::cout << __LINE__ << std::endl;

void webview_run(std::string url, std::string title = "", std::string init_js = "")
{
  logDebug("title: ", title);
  if (title == "Missing parameter" && init_js == "")
  {
#ifdef _WIN32
    init_js = "(async()=>{ await app.set_size(800, 600, 0); app.center(); app.show(); })()";
#else
    init_js = "(async()=>{ app.center(); await app.set_size(800, 600, 0); app.show(); })()";
#endif
  }

  logDebug("APP.TITLE, webview_run: ", title);
  w.set_title(title);
  w.setvar("app", "title", title);

  if (icon_file != "")
  {
    //    icon_file = std::filesystem::absolute(icon_file).make_preferred().string();
    // logTrace("Setting app icon to ", icon_file);
    w.set_icon(icon_file);
  }

  // webview_bind();

  // logDebug("URL: ", url, ", TITLE: ", title, ", INIT: ", init_js);
  if (run_and_exit)
  {
    if (init_js.back() != ';')
      init_js += ';';
    init_js += " app_exit();";
    w.init(init_js);
    w.set_html("<div></div>");
  }
  else
  {
    // logDebug("URL: ", url);

    if (!init_js.empty())
    {
      w.init(init_js);
    }

    if (starts_with(url, "html://"))
    {
      url.erase(0, 7);
      w.set_html(url);
    }
    else
    {
      {
        if (!starts_with(url, "http://") && !starts_with(url, "https://"))
        {
          url = "file://" + url;
        }

        // logDebug("URL FILE EXT: ", std::filesystem::path(url).extension().string());

        // logDebug("URL: ", url);
        w.navigate(url);
      }
    }
  }

  // logDebug("URL: ", url);
  w.run();
}

// Live html test :
// ./webapp.exe -c "<input type='button' value='Exit web app'
// onclick='exit_webapp()'>"
// ./webapp.exe -f index.html
// ./webapp.exe index.html
// ./webapp.exe
// ./webapp.exe -j "window.confirm('Hello')"
// ./webapp.exe -r "for(i=0; i < 10; i++) writeln(i);" # under a
// cygwin/mingw mintty console start /wait webapp.exe -r "for(i=0; i < 10;
// i++) writeln(i);" # under a windows cmd

/* Because of CORS we have to disable web security with WebView2 in order to alow to send http queries from the app to
remote servers From https://github.com/MicrosoftEdge/WebView2Feedback/issues/4166, env var
"WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS" with following values :
--disable-web-security # Tested fetch works with wiki.space.
--allow-insecure-localhost
--unsafely-treat-insecure-origin-as-secure=http:*
--disable-site-isolation-trials
--disable-block-insecure-private-network-requests
--enable-insecure-private-network-requests-allowed
--disable-private-network-access-respect-preflight-results
--disable-features=AutoupgradeMixedContent,PrivateNetworkAccessSendPreflights,PrivateNetworkAccessRespectPreflightResults,BlockInsecurePrivateNetworkRequests
--enable-features=PrivateNetworkAccessNonSecureContextsAllowed,InsecurePrivateNetworkRequestsAllowed
--enable-blink-features=PrivateNetworkAccessNonSecureContextsAllowed,InsecurePrivateNetworkRequestsAllowed

 See also : https://github.com/MicrosoftEdge/WebView2Feedback/issues/4166
 See there : https://observablehq.com/@mbostock/fetch-with-basic-auth
*/
#if defined _WIN32 && !defined _CONSOLE
int WINAPI WinMain(HINSTANCE /*hInst*/, HINSTANCE /*hPrevInst*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
  LPSTR *argv;
  int argc;
  argv = CommandLineToArgv(GetCommandLine(), &argc);
#else
int main(int argc, char **argv, char **)
{
#endif
  // Must force UTF8 with MSVC
#ifdef _MSC_VER
  setlocale(LC_ALL, ".UTF8");
#endif
#ifdef JUST_TEST
  std::string type_hierarchy(bool, bool);
  std::cout << type_hierarchy(false, true);
  return 0;
#endif

#ifdef _WIN32
  // FreeConsole();

  my_setenv("WEBVIEW2_USER_DATA_FOLDER", "C:\\Temp\\");

  // Set default value of WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS to --disable-web-security
  if (add_bro_args == "")
  {
    my_setenv("WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", "--disable-web-security --unsafely-treat-insecure-origin-as-secure=http:* "
                                                       "--unsafely-treat-insecure-origin-as-secure=https:* "
                                                       "--block-new-web-contents=true "
                                                       "--password-auto-save=true "
                                                       "--general-auto-fill=true");
    // Or to the ones provided through repetitive call to the -b options
  }
  else
  {
    my_setenv("WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS", add_bro_args);
    // Eventuellement voir ICoreWebView2EnvironmentOptions, put_AdditionalBrowserArguments
    // Dans
    // https://learn.microsoft.com/en-us/microsoft-edge/webview2/reference/win32/icorewebview2environmentoptions?view=webview2-1.0.2210.55
    // https://learn.microsoft.com/en-us/microsoft-edge/webview2/concepts/webview-features-flags?tabs=dotnetcsharp
    // extern std::string do_fstat(std::string sp);
    // do_fstat(url);
  }
#endif

  myopt.set(argc, argv,
            {
#ifdef _WIN32
                option_info('b', "browser-args", set_browser_args, "Provide additional browser arguments to the webview2 component.", required, option),
#endif
                option_info('c', "html", ins_html, "Provide an html string that will be directly set to the webview.", required, option),
                option_info('f', "url", set_url, "Provide a url, a remote one must prepended with 'http://', a local one must have one of the following extensions .html, .htm, .webapp or .wa.", required, option),
                option_info('p', "path", set_path, "Provide the path where could be found a file with a name within the following ones: " + idxs + ".", required, option),
                option_info("\tThe use of these 3 previous options is mutually exclusive."),
                option_info("\tIt is also possible to directly provide their argument as the last one of the command (hence witout the option), but prepended with \"html://\", for -c."),
                option_info(
                    't', "title", [](s_opt_params &p) -> void { title = p.val; }, "Set the title of the webview windows, default is to display the url as title if it is provided or nothing if just an html string is provided.", required, option),
                //    option_info('i', "interp", interp, "In console mode allow to enter commands.", no_arg, option),
                option_info(
                    'j', "js", [](s_opt_params &p) -> void { init_js = p.val; }, "Inject a javascript command before loading html page.", required, option),
                option_info(
                    'r', "runjs",
                    [](s_opt_params &p) -> void {
                      init_js = p.val;
                      runjs_and_exit = true;
                    },
                    "Run the provided javascript command and exits.", required, option),
                option_info('d', "debug", set_debug_mode, "Activate the developper mode in the webview.", no_arg, option),
                option_info('l', "log-level", set_log_level, "Set the log level. Their precedence is: ALL < TRACE < DEBUG < INFO < WARN < ERROR < FATAL < OFF.", required, option),
                option_info(
                    'g', "log-file", [](s_opt_params &p) -> void { my_setenv("LOGFILE", p.val); }, "Set the log file name.", required, option),
                option_info(
                    'q', "quiet", [](s_opt_params &) -> void { my_setenv("LOG", "OFF"); }, "Alias for -l OFF."),
                option_info("\tThese three previous option may also be set by the environment variables 'LOG' and 'LOGFILE' but the options have precedence on the environment."),
                option_info("\tIf neither the environment nor the options are set then relies on the debug option if it is used to set the log level to 'DEBUG'."),
                // May only be called after the webview creation
                option_info(
                    'u', "help-js", [](s_opt_params &) -> void { call_func_help = true; }, "List and briefly explain all the javascript objects extending the webview.", no_arg /*optional_argument*/, option),
                option_info(
                    'n', "icon", [](s_opt_params &p) -> void { icon_file = p.val; }, "Set windows icon with the provided .ico file.", required, option),
                option_info('G', "geometry", chg_ini_geom, "Attempt to modify geometry when starting the webapp with the one to four parameter passed, separated by commas (x, y, width, height).", required, option),
#ifdef _WIN32
                option_info(
                    'm', "minimized", [](s_opt_params &) -> void { init_win_state = win_state::minimized; }, "The webview window will be minimized at startup.", no_arg, option),

                option_info(
                    'M', "maximized", [](s_opt_params &) -> void { init_win_state = win_state::maximized; }, "The webview window will be maximized at startup.", no_arg, option),

                option_info(
                    's', "hidden", [](s_opt_params &) -> void { init_win_state = win_state::hidden; }, "The webview window will not be shown at startup.", no_arg, option),

#endif
                option_info(
                    'k', "hints", [](s_opt_params &p) -> void { hints = std::stoi(p.val); },
                    "Set webview hints => 0: width and height are default size, 1 set them as minimum bound, 2 set them as maximum bound. 3 they are fixed. Any other value is ignored.", required, option),
                option_info("A simple arguments will act as -p option."),
            });

  // Calls to logFunctions before getopt_init will not work correctly ...
  myopt.parse();

#ifdef TEST_LOG
  logTrace("Test logTrace");
  logDebug("Test logDebug");
  logDebug("Test logDebug multilignes\nLigne2\nLigne3\n");
  logInfo("Test logInfo");
  logWarn("Test logWarn");
  logError("Test logError");
  logFatal("Test logFatal");
#endif

  std::string tmp_htm = {};

  if (url.empty())
  {
    if (myopt.args.size() > 0)
    {
      url = myopt.args[0];
      // logDebug("URL: ", url);

      if (starts_with(url, "file://"))
        url = std::filesystem::absolute(url.substr(7)).generic_string();
      else if (!starts_with(url, "html://") && !starts_with(url, "http://") && !starts_with(url, "https://"))
      {
        s_opt_params p = {0, "", std::filesystem::absolute(url).generic_string()};
        set_path(p);
      }

      w.set_js_args(myopt.args);
    }
    else
    {
      // Search for index.html or index.js in current directory
      url = get_index();

      if (url.empty() && !call_func_help) // && !help_or_version)
      {
        if (title.empty())
          title = "Missing parameter";
        logDebug("title: ", title);

        std::string hm = myopt.usage();
        replace_all(hm, "\r", "<br>");
        replace_all(hm, "\n", "<br>");
        if (!hm.empty())
          url += "html://<pre style=\"white-space: pre-wrap;\">" + hm + "</pre>";
      }
    }
  }

#ifdef RUN_IT
  // If file ends with .wa or .webapp then replace it by a temporary file ending with .html
  auto ext = std::filesystem::path(url).extension().string();
  if (ext == ".wa" || ext == ".webapp")
  {
    extern bool set_fs_error(webview_wrapper & w, std::filesystem::path pcaller, const std::error_code ec);
    std::error_code ec;

    auto pp = std::filesystem::path(url).parent_path();
    // Create a temporary html file
    tmp_htm = tempfile(pp.string(), ".tmp.XXXXXX", 1) + ".html";
    std::filesystem::copy_file(url, tmp_htm, ec);
#ifdef _WIN32
    SetFileAttributes(tmp_htm.c_str(), FILE_ATTRIBUTE_HIDDEN);
#endif
    if (set_fs_error(w, std::filesystem::path("Failed to copy '" + url + "' to '" + tmp_htm), ec))
      exit(1);

    url = tmp_htm;
  }
#endif

  webview_set(init_win_state, devmode, runjs_and_exit);

  std::string help_func_tit = "List of the variables and functions for the objects extending the webview", help_func_text, help_func_html = w.help_objs();

  help_func_text = help_func_html;

  if (call_func_help || title == "Missing parameter")
  {
    logDebug("title: ", title);
    trim(help_func_text, "\"");
    replace_all(help_func_text, "&#xa;", "\n");
    replace_all(help_func_text, "&#10;", "\n");
    help_func_text = std::regex_replace(help_func_text, std::regex("\\<.*?\\>"), "");
  }

  if (title == "Missing parameter")
  {
    logDebug("title: ", title);
    std::string hf = help_func_html;
    replace_all(hf, "\\r", "<br>");
    replace_all(hf, "\\n", "<br>");
    if (url.empty())
      url += "html://";
    url += "<pre style=\"white-space: pre-wrap;\"><u>" + help_func_tit + "</u><br/>" + hf + "</pre>";

    if (!help_func_html.empty())
      std::cout << help_func_tit << std::endl << help_func_text << std::flush;
  }

  if (starts_with(url, "html://"))
  {
    if (title == "")
      title = "HTML string";
  }
  else
  {
    if (title == "")
      title = url;
  }

  if (call_func_help)
  {
    std::string s = help_func_text;
    std::cout << help_func_tit << std::endl << s << std::flush;
  }
  else // if (!help_or_version)
  {
    webview_run(url, title, init_js);
  }

  if (!tmp_htm.empty() && std::filesystem::exists(tmp_htm))
  {
    std::filesystem::remove(tmp_htm);
  }

  w.terminate();

  return 0;
}
