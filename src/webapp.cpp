
#ifdef _WIN32
#include "winapi.h"
#endif

#include <chrono>
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
#include "opts.h"
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

bool devmode = false, runjs_and_exit = false, html_string = false, js_instance = true, call_func_help = false;
std::string url, title = "", init_js = "";
webview_wrapper w;
int ix = 200, iy = 200, iw = 600, ih = 400, hint = 0;

std::string add_bro_args = "";
void set_browser_args(char, std::string, std::string val)
{
  // logTrace("Adding browser argument [--", val, "]");
  add_bro_args += " --" + val;
}

void set_url(char, std::string, std::string val)
{
  if (starts_with(val, "http:") || starts_with(val, "https:"))
  {
    url = val;
  }
  else
  {
    url = std::filesystem::absolute(val).generic_string();
  }

  // logTrace("set_url ", url);
  if (title.empty())
    title = url;
}

void ins_html(char, std::string, std::string val)
{
  url = "html://";
  url += val;
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

void set_debug_mode(char, std::string, std::string val)
{
  devmode = true;
  std::string ll = my_getenv("LOG");

  if (ll.empty())
  {
    my_setenv("LOG", "DEBUG");
  }
}

void set_log_level(char, std::string, std::string val)
{
  my_setenv("LOG", val);
  // Will check and eventually correct the log level env var if it is not correctly set
  check_log_level();
}

void chg_ini_geom(char val, std::string name, std::string param)
{
  auto scoo = split(param, ',');
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

// sed -n "/{/s/.*, \('.'\).*/\1/p" src/webapp.cpp src/opts.cpp | sort
void set_path(char, std::string, std::string spath);
std::vector<run_opt> r_opts = {
#ifdef _WIN32
    {"browser-args", 'b', opt_only, required_argument, "Provide additional browser arguments to the webview2 component.", set_browser_args},
#endif
    {"html", 'c', opt_only, required_argument, "Provide an html string that will be directly set to the webview.", ins_html},
    {"url", 'f', opt_only, required_argument, "Provide a url, a remote one must prepended with 'http://', a local one must have one of the following extensions .html, .htm, .webapp or .wa.", set_url},
    {"path", 'p', opt_only, required_argument, "Provide the path where could be found a file with a name within the following ones: " + idxs + ".", set_path},
    {"", '\0', 0, 0, "\tThe use of these 3 previous options is mutually exclusive.", nullptr},
    {"", '\0', 0, 0, "\tIt is also possible to directly provide their argument as the last one of the command (hence witout the option), but prepended with \"html://\", for -c.", nullptr},
    {"title", 't', opt_only, required_argument,
     "Set the title of the webview windows, default is to display the url as "
     "title if it is provided or nothing if just an html string is provided.",
     [](char, std::string, std::string val) -> void { title = val; }},
    //    {"interp", 'i', opt_only, no_argument, "In console mode allow to enter commands.", interp},
    {"js", 'j', opt_only, required_argument, "Inject a javascript command before loading html page.", [](char, std::string, std::string val) -> void { init_js = val; }},
    {"runjs", 'r', opt_only, required_argument, "Run the provided javascript command and exits.",
     [](char, std::string, std::string val) -> void {
       init_js = val;
       runjs_and_exit = true;
     }},
    {"debug", 'd', opt_only, no_argument, "Activate the developper mode in the webview.", set_debug_mode},
    {"log-level", 'l', opt_only, required_argument, "Set the log level. Their precedence is: ALL < TRACE < DEBUG < INFO < WARN < ERROR < FATAL < OFF.", set_log_level},
    {"log-file", 'g', opt_only, required_argument, "Set the log file name.", [](char, std::string, std::string val) -> void { my_setenv("LOGFILE", val); }},
    {"quiet", 'q', opt_only, no_argument, "Alias for -l OFF.", [](char, std::string, std::string val) -> void { my_setenv("LOG", "OFF"); }},
    {"", '\0', 0, 0, "\tThese three previous option may also be set by the environment variables 'LOG' and 'LOGFILE' but the options have precedence on the environment.", nullptr},
    {"", '\0', 0, 0, "\tIf neither the environment nor the options are set then relies on the debug option if it is used to set the log level to 'DEBUG'.", nullptr},
    // May only be called after the webview creation
    {"help-js", 'u', opt_only, no_argument /*optional_argument*/, "List and briefly explain all the javascript objects extending the webview.", [](char, std::string, std::string val) -> void { call_func_help = true; }},
    {"icon", 'n', opt_only, required_argument, "Set windows icon with the provided .ico file.", [](char, std::string, std::string val) -> void { icon_file = val; }},
    {"geometry", 'G', opt_only, required_argument, "Attempt to modify geometry when starting the webapp with the one to four parameter passed, separated by commas (x, y, width, height).", chg_ini_geom},
#ifdef _WIN32
    {"minimized", 'm', opt_only, no_argument, "The webview window will be minimized at startup.", [](char, std::string, std::string) -> void { init_win_state = win_state::minimized; }},

    {"maximized", 'M', opt_only, no_argument, "The webview window will be maximized at startup.", [](char, std::string, std::string) -> void { init_win_state = win_state::maximized; }},

    {"hidden", 's', opt_only, no_argument, "The webview window will not be shown at startup.", [](char, std::string, std::string) -> void { init_win_state = win_state::hidden; }},

#endif
    {"hints", 'k', opt_only, required_argument,
     "Set webview hints => 0: width and height are default size, 1 set them as "
     "minimum bound, 2 set them as maximum bound. 3 they are fixed. Any other "
     "value is ignored.",
     [](char, std::string, std::string val) -> void { hints = std::stoi(val); }},
    {"no-js-class", 'a', opt_only, no_argument, "Do not generate javascript class instanciation for the webview extension functions.", [](char, std::string, std::string val) -> void { js_instance = false; }},
};

void set_path(char, std::string, std::string spath)
{
  std::filesystem::path path(spath), parent_path;
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

void set_path(std::string spath)
{
  set_path(0, "", spath);
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
extern bool help_or_version;
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
  if (call_func_help || help_or_version || title == "Missing parameter")
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

void webview_run(std::string url, std::string title = "", std::string init_js = "", std::string wa_args = "")
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

  logDebug("URL: ", url, ", TITLE: ", title, ", INIT: ", init_js);
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
    logDebug("URL: ", url);

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
      // If file ends with .wa or .webapp extension then load and run it as html string
      auto ext = std::filesystem::path(url).extension().string();
      if (ext == ".wa" || ext == ".webapp")
      {
        auto s = file2s(url);
        w.set_html(s);
      }
      else
      {
        if (!starts_with(url, "http://") && !starts_with(url, "https://"))
        {
          url = "file://" + url;
        }

        logDebug("URL FILE EXT: ", std::filesystem::path(url).extension().string());

        logDebug("URL: ", url);
        w.navigate(url);
      }
    }
  }

  logDebug("URL: ", url);
  w.run(wa_args);
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

  // Calls to logFunctions before getopt_init will not work correctly ...
  getopt_init(argc, argv, r_opts, appInfo(), "", "(c) Denis Lalanne. Provided as is. NO WARRANTY of any kind.");

#ifdef TEST_LOG
  logTrace("Test logTrace");
  logDebug("Test logDebug");
  logDebug("Test logDebug multilignes\nLigne2\nLigne3\n");
  logInfo("Test logInfo");
  logWarn("Test logWarn");
  logError("Test logError");
  logFatal("Test logFatal");
#endif

  std::string wa_args = {};

  if (url.empty())
  {
    if (optind < argc)
    {
      url = argv[optind];
      logDebug("URL: ", url);

      if (starts_with(url, "file://"))
        url = std::filesystem::absolute(url.substr(7)).generic_string();
      else if (!starts_with(url, "html://") && !starts_with(url, "http://") && !starts_with(url, "https://"))
        set_path(std::filesystem::absolute(url).generic_string());

      for (int i = optind + 1; i < argc; i++)
      {
        wa_args += argv[i];
        if (i < argc - 1)
          wa_args += ", ";
      }
      if (!wa_args.empty())
        wa_args = "[ " + wa_args + " ]";

      logDebug("URL ARGS", wa_args);
    }
    else
    {
      // Search for index.html or index.js in current directory
      url = get_index();

      if (url.empty() && !call_func_help && !help_or_version)
      {
        if (title.empty())
          title = "Missing parameter";
        logDebug("title: ", title);

        std::string hm = usage();
        replace_all(hm, "\r", "<br>");
        replace_all(hm, "\n", "<br>");
        if (!hm.empty())
          url += "html://<pre style=\"white-space: pre-wrap;\">" + hm + "</pre>";
      }
    }
  }

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
  else if (!help_or_version)
  {
    logDebug("URL ARGS: ", wa_args);
    webview_run(url, title, init_js, wa_args);
  }

  w.terminate();

  return 0;
}
