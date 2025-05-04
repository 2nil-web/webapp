
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#ifdef __linux__
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif
#include <webview.h>

#include "log.h"
#include "util.h"
#ifdef _WIN32
#include "winapi.h"
#include "wm_map.h"
#include <commctrl.h>
#endif
#include "path_entity.h"
#include "wrap.h"

// #define WP (static_cast<webview::webview *>(w))
#define WP ((webview::webview *)w)

#if GTK_MAJOR_VERSION >= 4 && GTK_MINOR_VERSION >= 10
#define GTK_410_OR_MORE
#include <X11/Xlib.h>
#include <gdk/x11/gdkx.h>
#endif
/*
my_webview.h
class gtk_webkit_engine
using browser_engine = detail::gtk_webkit_engine;

class cocoa_wkwebview_engine
using browser_engine = detail::cocoa_wkwebview_engine;

class win32_edge_engine
using browser_engine = detail::win32_edge_engine;
*/

webview_wrapper *webview_wrapper::me;
webview_conf webview_wrapper::conf;
#define my_conf webview_wrapper::conf
static rectangle new_geom;

// The new_geom struct must be correctly set before calling on_geom
void webview_wrapper::on_geom()
{
  if (me)
  {
    logDebug("ON_GEOM: new_geom (", new_geom.x, ',', new_geom.y, ',', new_geom.w, ',', new_geom.h, ')');
    // We may have to save the geometry
    me->save_conf();

    // Is there any javascript function connected to the geometry event ?
    if (me->on_geometry_func != "")
    {
      // std::string call = me->on_geometry_func + "(" + std::to_string(conf.xpos) + "," + std::to_string(conf.ypos) + "," + std::to_string(conf.width) + "," + std::to_string(conf.height) + ")";
      // logDebug("CALL_GEOMETRY_FUNC: " + call);
      me->eval(me->on_geometry_func);
    }
  }
}

#ifdef WEBVIEW_PLATFORM_LINUX_WEBKITGTK_COMPAT_HH
#if GTK_MAJOR_VERSION == 3

std::string show_windows_state(GdkWindowState ws)
{
  std::string s = {};
  if (ws & GDK_WINDOW_STATE_ABOVE)
    s += "GDK_WINDOW_STATE_ABOVE:  the window is kept above other windows,";
  if (ws & GDK_WINDOW_STATE_BELOW)
    s += "GDK_WINDOW_STATE_BELOW:  the window is kept below other windows,";
  if (ws & GDK_WINDOW_STATE_BOTTOM_RESIZABLE)
    s += "GDK_WINDOW_STATE_BOTTOM_RESIZABLE:  whether the bottom edge is resizable, Since 3.22.23";
  if (ws & GDK_WINDOW_STATE_BOTTOM_TILED)
    s += "GDK_WINDOW_STATE_BOTTOM_TILED:  whether the bottom edge is tiled, Since 3.22.23";
  if (ws & GDK_WINDOW_STATE_FOCUSED)
    s += "GDK_WINDOW_STATE_FOCUSED:  the window is presented as focused (with active decorations),";
  if (ws & GDK_WINDOW_STATE_FULLSCREEN)
    s += "GDK_WINDOW_STATE_FULLSCREEN:  the window is maximized without";
  if (ws & GDK_WINDOW_STATE_ICONIFIED)
    s += "GDK_WINDOW_STATE_ICONIFIED:  the window is minimized,";
  if (ws & GDK_WINDOW_STATE_LEFT_RESIZABLE)
    s += "GDK_WINDOW_STATE_LEFT_RESIZABLE:  whether the left edge is resizable, Since 3.22.23";
  if (ws & GDK_WINDOW_STATE_LEFT_TILED)
    s += "GDK_WINDOW_STATE_LEFT_TILED:  whether the left edge is tiled, Since 3.22.23";
  if (ws & GDK_WINDOW_STATE_MAXIMIZED)
    s += "GDK_WINDOW_STATE_MAXIMIZED:  the window is maximized,";
  if (ws & GDK_WINDOW_STATE_RIGHT_RESIZABLE)
    s += "GDK_WINDOW_STATE_RIGHT_RESIZABLE:  whether the right edge is resizable, Since 3.22.23";
  if (ws & GDK_WINDOW_STATE_RIGHT_TILED)
    s += "GDK_WINDOW_STATE_RIGHT_TILED:  whether the right edge is tiled, Since 3.22.23";
  if (ws & GDK_WINDOW_STATE_STICKY)
    s += "GDK_WINDOW_STATE_STICKY:  the window is sticky,";
  if (ws & GDK_WINDOW_STATE_TILED)
    s += "GDK_WINDOW_STATE_TILED:  the window is in a tiled state, Since 3.10. Since 3.22.23, this";
  if (ws & GDK_WINDOW_STATE_TOP_RESIZABLE)
    s += "GDK_WINDOW_STATE_TOP_RESIZABLE:  whether the top edge is resizable, Since 3.22.23";
  if (ws & GDK_WINDOW_STATE_TOP_TILED)
    s += "GDK_WINDOW_STATE_TOP_TILED:  whether the top edge is tiled, Since 3.22.23";
  if (ws & GDK_WINDOW_STATE_WITHDRAWN)
    s += "GDK_WINDOW_STATE_WITHDRAWN:  the window is not shown,";
  return s;
}

GdkWindowState webview_wrapper::current_window_state = GDK_WINDOW_STATE_WITHDRAWN;
bool webview_wrapper::gtk_on_configure_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  logDebug("gtk_on_configure_event-current_window_state: ", show_windows_state(current_window_state));

  if (current_window_state != GDK_WINDOW_STATE_WITHDRAWN)
  {
    GtkWindow *hw = GTK_WINDOW(widget);

    // if (!gtk_window_has_toplevel_focus(hw) || current_window_state & GDK_WINDOW_STATE_ICONIFIED || current_window_state & GDK_WINDOW_STATE_MAXIMIZED || current_window_state & GDK_WINDOW_STATE_FULLSCREEN)
    if (!(current_window_state & GDK_WINDOW_STATE_FOCUSED))
    {
      logDebug("window_state_event NOT saving geom");
      return false;
    }

    if (me && GTK_WINDOW(webview_get_window(me->w)) == hw)
    {
      double dx, dy;

#if GTK_MAJOR_VERSION == 3
      gdk_event_get_coords(event, &dx, &dy);
      gtk_window_get_size(hw, &new_geom.w, &new_geom.h);
      logDebug("aft gtk_window_get_size - new_geom.w and h: ", new_geom.w, ',', new_geom.h);
      logDebug("gtk_window_get_size - conf.width and height: ", me->conf.width, ',', me->conf.height);
#elif GTK_MAJOR_VERSION == 4
      gdk_event_get_position(event, &dx, &dy);
      gtk_window_get_default_size(hw, &new_geom.w, &new_geom.h);
// Use gtk_window_is_active(hw)/gtk_window_is_fullscreen(hw)/gtk_window_is_maximized(hw)/gtk_window_is_suspended(hw)
#endif

      new_geom.x = dx, new_geom.y = dy;
      logDebug("window_state_event SAVING geom");
      on_geom();
    }
  }

  return false;
}

bool webview_wrapper::gtk_on_window_state_event(GtkWidget *widget, GdkEventWindowState *stat_ev, gpointer user_data)
{
  GtkWindow *hw = GTK_WINDOW(widget);

  if (me && GTK_WINDOW(webview_get_window(me->w)) == hw)
  {
    current_window_state = stat_ev->new_window_state;
  }
  logDebug("gtk_on_window_state_event-current_window_state: ", show_windows_state(current_window_state));

  return false;
}
#endif

#ifdef __GNUC__
namespace webview
{
namespace detail
{
#endif
void WindowInitialState(GtkWidget *m_window, GtkWidget *m_webview)
{
  logDebug("WindowInitialState: ", my_conf.xpos, ',', my_conf.ypos, ',', my_conf.width, ',', my_conf.height);

#ifdef GTK_410_OR_MORE
  // logTrace("Missing gtk_window_move in gtk4");
#ifdef NE_FONCTIONNE_PAS
  // GtkWidget *m_window = gtk_application_window_new (app);
  // GdkSurface* native = gtk_native_get_surface(GTK_NATIVE (m_window));
  // printf("%p\n",native); //print (nil)
  Window xw = gdk_x11_surface_get_xid(GDK_SURFACE(gtk_native_get_surface(GTK_NATIVE(m_window))));
  Display *xd = gdk_x11_display_get_xdisplay(gtk_widget_get_display(m_window));
  XMoveWindow(xd, xw, 100, 100);
  // gtk_window_present (GTK_WINDOW (m_window));
#endif
#else
  logTrace("GTK410_LESS, Initial move of window");
  gtk_window_move(GTK_WINDOW(m_window), my_conf.xpos, my_conf.ypos);
#endif
  logDebug("gtk_widget_set_size_request: ", my_conf.width, ',', my_conf.height);
  gtk_widget_set_size_request(GTK_WIDGET(m_window), my_conf.width, my_conf.height);
  gtk_widget_grab_focus(GTK_WIDGET(m_webview));
  gtk_compat::widget_set_visible(GTK_WIDGET(m_window), true);
}
#ifdef __GNUC__
}
}
#endif
#endif

#ifdef _WIN32
#include <oleacc.h>
#pragma comment(lib, "Oleacc.lib")

bool getwinrect(HWND hw, rectangle &rc)
{
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);

  if (GetWindowPlacement(hw, &wp))
  {
    // logPlacement(wp);
    rc.x = wp.rcNormalPosition.left;
    rc.y = wp.rcNormalPosition.top;
    rc.w = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
    rc.h = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
    return true;
  }

  return false;
}

LRESULT webview_wrapper::windows_on_event(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
  if (me && (HWND)webview_get_window(me->w) == hWnd && wm_map.count(uMsg))
  {
    switch (uMsg)
    {
    case WM_CLOSE:
    case WM_QUIT:
    case WM_SYSCOMMAND:
    case WM_DESTROY:
      if (me->on_exit_func != "")
      {
        // logTrace("bye bye ", me->on_exit_func);
        // me->init(me->on_exit_func);
        me->eval(me->on_exit_func);
      }
      break;

      //    case WM_SHOWWINDOW:
      //      logTrace("WM_SHOWWINDOW ");
      //      break;

      //    case WM_SIZE:
      //      logTrace("WM_SIZE");
      //      break;

      //    case WM_MOVE:
      //      logTrace("WM_MOVE");
      //      break;

    default:
      break;
    }

    if (getwinrect(hWnd, new_geom))
      on_geom();
  }

  return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

#include <dwmapi.h>
void webview_wrapper::set_dark_bar(bool set)
{

  BOOL dark_mode = true;
  if (!set)
    dark_mode = false;
  DwmSetWindowAttribute((HWND)window(), DWMWA_USE_IMMERSIVE_DARK_MODE, &dark_mode, sizeof(dark_mode));
  ShowWindow((HWND)window(), SW_HIDE);
  ShowWindow((HWND)window(), SW_SHOW);
  SetForegroundWindow((HWND)window());
}

// #include <wil/com.h>
#include <WebView2.h>
ICoreWebView2 *curr_m_webView = nullptr;
ICoreWebView2Controller *curr_mctl = nullptr;

void logMonInfo(HMONITOR hm)
{
  MONITORINFO mi;
  mi.cbSize = sizeof(MONITORINFO);
  if (GetMonitorInfo(hm, &mi))
  {
    logDebug("Monitor l: ", mi.rcMonitor.left, ", t: ", mi.rcMonitor.top, ", r: ", mi.rcMonitor.right, ", b: ", mi.rcMonitor.bottom);
    logDebug("Work    l: ", mi.rcWork.left, ", t: ", mi.rcWork.top, ", r: ", mi.rcWork.right, ", b: ", mi.rcWork.bottom);
  }
}

void win_center(HWND hw)
{
  // logTrace("WIN32 center webapp");
  HMONITOR hm = MonitorFromWindow(hw, MONITOR_DEFAULTTONEAREST);
  // logMonInfo(hm);
  MONITORINFO mi;
  mi.cbSize = sizeof(MONITORINFO);
  int sX, sY, sW, sH;
  if (GetMonitorInfo(hm, &mi) != 0)
  {
    sX = mi.rcWork.left;
    sY = mi.rcWork.top;
    sW = mi.rcWork.right - mi.rcWork.left;
    sH = mi.rcWork.bottom - mi.rcWork.top;
    logDebug("win_center-GetMonitorInfo (", sX, ',', sY, ',', sW, ',', sH, ')');
  }
  else
  {
    sX = 0;
    sY = 0;
    sW = GetSystemMetrics(SM_CXSCREEN);
    sH = GetSystemMetrics(SM_CYSCREEN);
    logDebug("win_center-GetSystemMetrics (", sX, ',', sY, ',', sW, ',', sH, ')');
  }

  rectangle rc;
  getwinrect(hw, rc);
  logDebug("win_center-getwinrect (", rc.x, ',', rc.y, ',', rc.w, ',', rc.h, ')');

  int wW = rc.w;
  int wH = rc.h;
  int wX = sX + (sW - wW) / 2;
  int wY = sY + (sH - wH) / 2;
  if (wW > sW)
    wW = sW;
  if (wH > sH)
    wH = sH;
  if (wX < sX)
    wX = sX;
  if (wY < sY)
    wY = sY;

  logDebug("win_center-MoveWindow (", wX, ',', wY, ',', wW, ',', wH, ')');
  MoveWindow(hw, wX, wY, wW, wH, FALSE);
  // SetWindowPos(hw, NULL, x, y, wW, wH, SWP_NOSIZE | SWP_NOZORDER);
}

#ifdef __GNUC__
namespace webview
{
namespace detail
{
#endif
/*
get_IsGeneralAutofillEnabled	IsGeneralAutofillEnabled controls whether autofill for information like names, street and email addresses, phone numbers, and arbitrary input is enabled.
get_IsPasswordAutosaveEnabled	IsPasswordAutosaveEnabled controls whether autosave for password information is enabled.
put_IsGeneralAutofillEnabled	Set the IsGeneralAutofillEnabled property.
put_IsPasswordAutosav
*/

#ifdef ACOMPILER
// Function to clear the autofill data from the last hour
void ClearAutofillData(ICoreWebView2Controller *m_controller)
{
  wwil::com_ptr<ICoreWebView2> coreWebView2;
  CHECK_FAILURE(m_controller->get_CoreWebView2(&coreWebView2));

  auto webview7 = coreWebView2.try_query<ICoreWebView2_7>();
  if (webview7)
  {
    wil::com_ptr<ICoreWebView2Profile> profile;
    CHECK_FAILURE(webview7->get_Profile(&profile));
    double endTime = (double)std::time(nullptr);
    double startTime = endTime - 3600;
    // Get the current time and offset the current time by 3600 seconds to clear the data
    // from the start time (one hour ago), until the end time (present time).
    // This will clear the data for the last hour.
    COREWEBVIEW2_BROWSING_DATA_KINDS dataKinds = (COREWEBVIEW2_BROWSING_DATA_KINDS)(COREWEBVIEW2_BROWSING_DATA_KINDS_GENERAL_AUTOFILL | COREWEBVIEW2_BROWSING_DATA_KINDS_PASSWORD_AUTOSAVE);
    CHECK_FAILURE(profile->ClearBrowsingDataInTimeRange(dataKinds, startTime, endTime, Callback<ICoreWebView2ClearBrowsingDataCompletedHandler>([this](HRESULT error) -> HRESULT { return S_OK; }).Get()));
  }
}
#endif

bool my_configure(ICoreWebView2 *m_webView, ICoreWebView2Controller *mctl)
{
  // logTrace("MY_WEBVIEW_EXT - my_configure");

  if (m_webView == nullptr || mctl == nullptr)
    return false;

  curr_m_webView = m_webView;
  curr_mctl = mctl;

#ifdef ACOMPILER
  ICoreWebView2Settings *settings = nullptr;
  auto res = m_webView->get_Settings(&settings);
  if (res != S_OK)
  {
    std::cerr << "WEBVIEW_ERROR_UNSPECIFIED, get_Settings failed" << std::endl;
    return false;
  }
  else
  {
    res = settings->put_IsGeneralAutofillEnabled(TRUE); // webview_wrapper::conf.debug ? TRUE : FALSE);
    if (res != S_OK)
    {
      std::cerr << "WEBVIEW_ERROR_UNSPECIFIED, put_IsGeneralAutofillEnabled failed" << std::endl;
      return false;
    }

    res = settings->put_IsPasswordAutosaveEnabled(TRUE); // webview_wrapper::conf.debug ? TRUE : FALSE);
    if (res != S_OK)
    {
      std::cerr << "WEBVIEW_ERROR_UNSPECIFIED, put_IsPasswordAutosaveEnabled failed" << std::endl;
      return false;
    }
  }

  // Get the profile object.
  auto webView2_13 = m_webView->try_query<ICoreWebView2_13>();
  CHECK_FEATURE_RETURN(webView2_13);
  wil::com_ptr<ICoreWebView2Profile> webView2Profile;
  CHECK_FAILURE(webView2_13->get_Profile(&webView2Profile));
  CHECK_FEATURE_RETURN(webView2Profile);
  auto webView2Profile6 = webView2Profile.try_query<ICoreWebView2Profile6>();
  CHECK_FEATURE_RETURN(webView2Profile6);

  BOOL enabled;
  CHECK_FAILURE(webView2Profile6->get_IsGeneralAutofillEnabled(&enabled));
  // Set general-autofill property to the opposite value to current value.
  if (enabled)
  {
    CHECK_FAILURE(webView2Profile6->put_IsGeneralAutofillEnabled(FALSE));
    MessageBox(nullptr,
               L"General autofill will be disabled immediately in all WebView2 with the "
               L"same profile.",
               L"Profile settings change", MB_OK);
  }
  else
  {
    CHECK_FAILURE(webView2Profile6->put_IsGeneralAutofillEnabled(TRUE));
    MessageBox(nullptr,
               L"General autofill will be enabled immediately in all WebView2 with the "
               L"same profile.",
               L"Profile settings change", MB_OK);
  }
#endif
  return true;
}

void WindowInitialState(HWND hw)
{
  // logTrace("MY_WEBVIEW_EXT - WindowInitialState");
  //  logTrace("xpos=", my_conf.xpos, ", ", "ypos=", my_conf.ypos, ", ", "w=", my_conf.width, ", ", "h=", my_conf.height );
  if (my_conf.xpos == -32000 && my_conf.ypos == -32000 && my_conf.init_win_state != win_state::hidden)
    win_center(hw);
  else
    SetWindowPos(hw, NULL, my_conf.xpos, my_conf.ypos, my_conf.width, my_conf.height, 0);

  //  set_geometry(xpos, ypos, width, height);
  switch (my_conf.init_win_state)
  {
  case win_state::hidden:
    break;
  case win_state::normal:
    ShowWindow(hw, SW_SHOW);
    break;
  case win_state::minimized:
    ShowWindow(hw, SW_SHOWMINIMIZED);
    break;
  case win_state::maximized:
    ShowWindow(hw, SW_SHOWMAXIMIZED);
    break;
  default:
    break;
  }

  UpdateWindow(hw);
  SetFocus(hw);
}

#ifdef __GNUC__
}
}
#endif
#endif

int json_parse_c(const char *s, size_t sz, const char *key, size_t keysz, const char **value, size_t *valuesz)
{
  return webview::detail::json_parse_c(s, sz, key, keysz, value, valuesz);
}

std::string json_parse(const std::string &s, const std::string &key, const int index)
{
  return webview::detail::json_parse(s, key, index);
}

std::vector<std::string> js_parse2vs(const std::string r)
{
  std::vector<std::string> vs;
  std::string s;
  int i = 0;
  for (;;)
  {
    s = json_parse(r, "", i++);
    if (s.empty())
      break;
    vs.push_back(s);
  }

  return vs;
}

static size_t param_index = 0;
void reset_param()
{
  param_index = 0;
}

// Inspiré de json_parse mais on lit tous les paramètres d'un même req, séquentiellement du 1er au dernier et à la fin on bloque.
// On débloque quand le req a changé
bool get_param(const std::string req, std::string &pv)
{
  //  if (!req.empty()) logTrace("REQ: ", req );

  if (req == "[]" || req == "[\"\"]" || req == "['']")
    return false;

  static std::string last_req = "";
  auto vr = js_parse2vs(req);

  if (last_req == req)
  {
    // Si on est dans cet état là c'est qu'on a lu tous les paramètre du req courant, donc on sort
    if (param_index > vr.size() - 1)
      return false;
  }
  else
  {
    param_index = 0;
    last_req = req;
  }

  const char *value;
  size_t value_sz;
  webview::detail::json_parse_c(req.c_str(), req.length(), nullptr, param_index++, &value, &value_sz);

  if (value != nullptr)
  {
    std::string v;

    if (value[0] != '"')
    {
      v = {value, value_sz};

      if (v.empty())
      {
        return false;
      }
      else
      {
        pv = v;
        //        logTrace("get_param OK1: ", pv );
        return true;
      }
    }

    int n = webview::detail::json_unescape(value, value_sz, nullptr);

    if (n > 0)
    {
      char *decoded = new char[n + 1];
      webview::detail::json_unescape(value, value_sz, decoded);
      std::string result(decoded, n);
      v = result;
      delete[] decoded;
      if (v.empty())
      {
        return false;
      }
      else
      {
        pv = v;
        //        logTrace("get_param OK2: ", pv );
        return true;
      }
    }
  }

  //  logTrace("get_param: ", pv );
  return false;
}

bool try_stoi(std::string s, int &n)
{
  std::size_t pos{};
  bool ret = false;
  try
  {
    const int i{std::stoi(s, &pos)};
    n = i;
    ret = true;
  }
  catch (std::invalid_argument const &ex)
  {
    logError("std::invalid_argument::what(): ", ex.what());
    ;
  }
  catch (std::out_of_range const &ex)
  {
    logError("std::out_of_range::what(): ", ex.what());
    ;
  }

  return ret;
}

bool try_stoll(std::string s, long long &n)
{
  std::size_t pos{};
  bool ret = false;
  try
  {
    const long long i{std::stoll(s, &pos)};
    n = i;
    ret = true;
  }
  catch (std::invalid_argument const &ex)
  {
    logError("std::invalid_argument::what(): ", ex.what());
    ;
  }
  catch (std::out_of_range const &ex)
  {
    logError("std::out_of_range::what(): ", ex.what());
    ;
  }

  return ret;
}

bool try_stof(std::string s, float &n)
{
  std::size_t pos{};
  bool ret = false;
  try
  {
    const float i{std::stof(s, &pos)};
    n = i;
    ret = true;
  }
  catch (std::invalid_argument const &ex)
  {
    logError("std::invalid_argument::what(): ", ex.what());
    ;
  }
  catch (std::out_of_range const &ex)
  {
    logError("std::out_of_range::what(): ", ex.what());
    ;
  }

  return ret;
}

bool get_param(const std::string r, int &n)
{
  std::string s;
  bool ret = get_param(r, s);
  if (ret)
    try_stoi(s, n);
  return ret;
}

bool get_param(const std::string r, float &f)
{
  std::string s;
  bool ret = get_param(r, s);
  if (ret)
    try_stof(s, f);
  return ret;
}

bool get_params(const std::string r, std::string &s)
{
  return get_param(r, s);
}

bool get_parami(const std::string r, int &n)
{
  return get_param(r, n);
}

bool get_paramf(const std::string r, int &f)
{
  return get_param(r, f);
}

void debug_js_parse(const std::string r)
{
  int v = -1;
  bool gpr = get_param(r, v);
  logDebug("GET_PARAMS RES: ", (gpr ? "true" : "false"), ", REQ: ", r, ", V: ", v);
}

std::string webview_wrapper::json_escape(const std::string &s, bool add_quotes)
{
  return webview::detail::json_escape(s, add_quotes);
}

void webview_wrapper::out_conf(std::string s)
{
  if (!s.empty())
  {
    logDebug("conf.", s, ", win_state: ", conf.init_win_state, ", debug: ", conf.debug, ", status: ", conf.status, ", zoom: ", conf.zoom, ", conf.ctx_menu: ", conf.ctx_menu, ", conf.psw_sav: ", conf.psw_sav, ", conf.auto_fill: ", conf.auto_fill,
             ", geometry: (", conf.xpos, ", ", conf.ypos, ", ", conf.width, ", ", conf.height, ")");
  }
}

// Retourne un message différent selon la valeur du paramètre
std::string getparams(int n)
{
  int an = std::abs(n);
  std::string msg;

  if (an == INT_MAX)
    msg = "Unlimited";
  else
  {
    msg = std::to_string(an);

    if (n < 0)
      msg += " opt";
  }

  msg += " param";
  if (an > 1)
    msg += 's';
  msg += '.';

  return msg;
}

// Retourne un message différent selon la valeur du paramètre
std::string get_help_params(int n)
{
  int an = std::abs(n);
  std::string msg;

  if (an == INT_MAX)
    msg = "variable";
  else
  {
    if (n < 0)
      msg = "up to ";
    msg += std::to_string(an);
  }

  /*msg += " parameter";
  if (an > 1)
    msg += 's';*/
  // msg += '.';

  return msg;
}

std::string webview_wrapper::help_objs(std::string fmt)
{
  std::string res, s = "";

  std::string eol = "\","; //\\r\\n";
  if (fmt == "json")
  {
    // First string is the class name, second string is the function declaration
    for (auto fh : func_help)
    {
      std::string dv = class_descvars(std::get<1>(fh), true);
      // logTrace("DV JSON " + std::get<1>(fh));
      s += dv;

      s += "{";
      if (std::get<0>(fh))
      {
        s += "\"async\":";
        if (std::get<0>(fh))
          s += "true";
        else
          s += "false";
        s += ',';
      }

      s += "\"name\":\"" + std::get<1>(fh) + "\",";

      if (std::get<2>(fh) != 0)
      {
        s += "\"param\":" + std::to_string(std::get<2>(fh)) + ',';
      }

      if (!std::get<3>(fh).empty())
        s += "\"desc\":\"" + std::get<3>(fh) + '"';
      s += "},";
    }
    // Remove last comma
    s.pop_back();
    res = "[" + s + "]";
  }
  else
  {
    std::string vdec = {};
    for (auto fh : func_help)
    {
      // logTrace("DV TEXT" + std::get<1>(fh));
      std::string dv = class_descvars(std::get<1>(fh), false);
      vdec += dv;

      // If function is a promise then prefix its name with a star
      s += (std::get<0>(fh) ? "*" : " ");
      s += std::get<1>(fh) + ':';
      // Mise en forme de l'info de arguments
      int np = std::get<2>(fh);
      std::string vp = std::to_string(abs(np));

      if (np != 0)
      {
        if (np >= 9999)
        {
          s += "at least 1 parameter";
        }
        else if (np <= -9999)
        {
          s += "indefinite number of parameters";
        }
        else
        {
          if (np > 0)
            s += "needs " + vp;
          else if (np < 0)
            s += "accepts up to " + vp;
          s += " parameter";
          if (abs(np) > 1)
            s += "s";
        }
        s += ", ";
      }

      s += std::get<3>(fh) + '\n';
    }
    res = "<i><u>Variables:</u>\nRead-only ones are prepended by a '-'</i>\n" + to_htent(vdec) + "<i><u>Functions:</u>\nAsynchronous ones are prepended by a '*'</i>\n" + to_htent(s);
  }

  // logTrace(res);
  // if (fmt == "json") return json_escape(res);
  return res;
}

void webview_wrapper::create(void *wnd)
{
  if (restore_conf(conf))
  {
    setvar("app", "x", conf.xpos);
    setvar("app", "y", conf.ypos);
    setvar("app", "w", conf.width);
    setvar("app", "h", conf.height);
    out_conf("create");
  }

  if (w != nullptr)
    return;
  w = new webview::webview(conf.debug, wnd);

#ifdef _WIN32
  HWND hw = (HWND)webview_get_window(w);
  SetWindowSubclass(hw, (SUBCLASSPROC)windows_on_event, 0, 0);
#endif

#ifdef WEBVIEW_PLATFORM_LINUX_WEBKITGTK_COMPAT_HH
#if GTK_MAJOR_VERSION == 3

  // static guint id = 0;
  GtkWindow *gw = (GtkWindow *)webview_get_window(w);
  // logTrace("configure-event");
  g_signal_connect(gw, "configure-event", G_CALLBACK(gtk_on_configure_event), nullptr);
  g_signal_connect(gw, "window-state-event", G_CALLBACK(gtk_on_window_state_event), nullptr);
#endif
#endif

  // We could manage a queue of webview here to allow multiple webview windows for the same app ...
  me = this;

  bind_doc(
      "app_help",
      [&](const std::string &req) -> std::string {
        auto fmt = json_parse(req, "", 0);
        return help_objs(fmt);
      },
      "return this help message and the list of all the available variables and functions for the objects extending the app.", -1);
}

void webview_wrapper::create(bool debug, void *wnd)
{
  conf.debug = debug;
  create(wnd);
}

webview_wrapper::webview_wrapper(bool debug, void *wnd)
{
  create(debug, wnd);
}

webview_wrapper::webview_wrapper()
{
}

void webview_wrapper::terminate()
{
  if (me)
  {
    logDebug("terminate");

    if (on_exit_func != "")
    {
      eval(me->on_exit_func);
    }

    WP->terminate();
#if defined _WIN32 && !defined(_MSC_VER)
    delete me;
#endif
    me = nullptr;
    delete WP;
    exit(0);
  }
}

webview_wrapper::~webview_wrapper()
{
  logDebug("Destructor");
  if (me)
    terminate();
}

void webview_wrapper::navigate(const std::string &url)
{
  logDebug("URL: ", url);
  WP->navigate(url);
}

void webview_wrapper::bind(const std::string &name, sync_binding_t fn)
{
  WP->bind(name, fn);
}

void webview_wrapper::bind(const std::string &name, binding_t fn, void *arg)
{
  WP->bind(name, fn, arg);
}

void webview_wrapper::unbind(const std::string &name)
{
  // logTrace("wrap_unbind");
  WP->unbind(name);
}

void webview_wrapper::resolve(const std::string &seq, int status, const std::string &result)
{
  WP->resolve(seq, status, result);
}

void webview_wrapper::run(std::string p_js_args)
{
  js_args = p_js_args;
  logDebug("URL JS ARGS: ", js_args);
  WP->run();
}

void *webview_wrapper::window()
{
  //  std::cout << webview_get_window(w) << std::endl;
  return webview_get_window(w);
}

void webview_wrapper::dispatch(std::function<void()> f)
{
  WP->dispatch(f);
}

std::string webview_wrapper::get_title()
{
#ifdef _WIN32
  return GetWindowText((HWND)window());
#else
  return app_title;
#endif
}

#ifdef _WIN32
std::wstring webview_wrapper::get_title_w()
{
  wchar_t wtitle[2048];
  GetWindowTextW((HWND)window(), wtitle, 2048);
  return std::wstring(wtitle);
}
#endif

std::string webview_wrapper::set_title(const std::string &title)
{
  std::string prev_title = get_title();
  app_title = title;
  WP->set_title(title);
  setvar("app", "title", title);
  // logInfo("APP.TITLE, set_title-prev_title: ", prev_title, ", title: ", title);
  return prev_title;
}

#ifdef _WIN32
bool LoadIconIfExists(HWND hw, std::string ico)
{
  //  std::string icoAbs = std::filesystem::absolute(ico).generic_string();
  std::filesystem::path icoAbs = std::filesystem::absolute(ico);
  // logTrace("Trying to load icon file ", ico, '(', icoAbs, ')');

  if (std::filesystem::is_regular_file(icoAbs))
  {
    // logTrace("Found icon file ", icoAbs);
    HICON hIc = (HICON)LoadImage(GetModuleHandle(NULL), icoAbs.string().c_str(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE | LR_SHARED);
    if (hIc)
    {
      // logTrace("Changing app icon with file ", icoAbs);
      SendMessage(hw, WM_SETICON, ICON_BIG, (LPARAM)hIc);
      SendMessage(hw, WM_SETICON, ICON_SMALL, (LPARAM)hIc);
      //      SendMessage(GetWindow(hw, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIc);
      //      SendMessage(GetWindow(hw, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIc);
      return true;
    }
  }

  // logTrace("Error loading app icon: ", TxtError());
  SendMessage(hw, WM_SETICON, ICON_BIG, (LPARAM) nullptr);
  SendMessage(hw, WM_SETICON, ICON_SMALL, (LPARAM) nullptr);
  return false;
}
#endif

std::string webview_wrapper::set_icon(std::string file)
{
  static std::string prev_file = {};
  bool chg_ok = false;

#ifdef _WIN32
  chg_ok = LoadIconIfExists((HWND)window(), file);
#else
#ifndef GTK_410_OR_MORE
  GError *err = nullptr;
  if (!gtk_window_set_icon_from_file(GTK_WINDOW(window()), file.c_str(), &err))
  {
    // logTrace(err->message);
    chg_ok = true;
  }
#endif
#endif
  if (chg_ok)
  {
    icon_file = file;
    std::string ret_file = prev_file;
    prev_file = file;
    setvar("app", "icon", file);
    return ret_file;
  }

  return prev_file;
}

void webview_wrapper::show()
{
#ifdef _WIN32
  ShowWindow((HWND)window(), SW_SHOW);
  UpdateWindow((HWND)window());
  SetFocus((HWND)window());
#endif
}

void webview_wrapper::hide()
{
#ifdef _WIN32
  ShowWindow((HWND)window(), SW_HIDE);
#endif
}

void webview_wrapper::minimize()
{
#ifdef _WIN32
  ShowWindow((HWND)window(), SW_SHOWMINIMIZED);
#elif !defined(GTK_410_OR_MORE)
  gtk_window_iconify(GTK_WINDOW(window()));
#endif
}

void webview_wrapper::maximize()
{
#ifdef _WIN32
  ShowWindow((HWND)window(), SW_SHOWMAXIMIZED);
#elif !defined(GTK_410_OR_MORE)
  if (!gtk_window_is_maximized(GTK_WINDOW(window())))
    gtk_window_maximize(GTK_WINDOW(window()));
#endif
}

#ifdef _WIN32
// Thank you to https://github.com/MicrosoftEdge/WebView2Samples/blob/def7237dd5b70cc54ee480710df2e0f2ce09690e/SampleApps/WebView2APISample/AppWindow.cpp#L802
RECT rcPrev;
#endif
bool is_fullscreen = false;
void webview_wrapper::enter_fullscreen()
{
  if (!is_fullscreen)
  {
#ifdef _WIN32
    HWND hw = (HWND)window();
    DWORD style = GetWindowLong(hw, GWL_STYLE);
    MONITORINFO mInfo = {sizeof(mInfo)};
    //  m_hMenu = ::GetMenu(hw);
    //  ::SetMenu(hw, nullptr);
    if (GetWindowRect(hw, &rcPrev) && GetMonitorInfo(MonitorFromWindow(hw, MONITOR_DEFAULTTOPRIMARY), &mInfo))
    {
      SetWindowLong(hw, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
      SetWindowPos(hw, HWND_TOP, mInfo.rcMonitor.left, mInfo.rcMonitor.top, mInfo.rcMonitor.right - mInfo.rcMonitor.left, mInfo.rcMonitor.bottom - mInfo.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
#elif GTK_MAJOR_VERSION
    gtk_window_fullscreen(GTK_WINDOW(window()));
#endif
    is_fullscreen = true;
  }
}

void webview_wrapper::exit_fullscreen()
{
  if (is_fullscreen)
  {
#ifdef _WIN32
    HWND hw = (HWND)window();
    DWORD style = GetWindowLong(hw, GWL_STYLE);
    //::SetMenu(hw, m_hMenu);
    SetWindowLong(hw, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
    SetWindowPos(hw, NULL, rcPrev.left, rcPrev.top, rcPrev.right - rcPrev.left, rcPrev.bottom - rcPrev.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

#elif GTK_MAJOR_VERSION
    gtk_window_unfullscreen(GTK_WINDOW(window()));
#endif
    is_fullscreen = false;
  }
}

void webview_wrapper::restore()
{
#ifdef _WIN32
  ShowWindow((HWND)window(), SW_RESTORE);
#elif GTK_MAJOR_VERSION
  if (gtk_window_is_maximized(GTK_WINDOW(window())))
    gtk_window_unmaximize(GTK_WINDOW(window()));
#ifndef GTK_410_OR_MORE
  gtk_window_deiconify(GTK_WINDOW(window()));
#endif
#endif
}

void webview_wrapper::get_pos(int &x, int &y)
{
  x = 0;
  y = 0;
#ifdef _WIN32
  RECT rc;
  GetWindowRect((HWND)window(), &rc);
  x = rc.left;
  y = rc.top;
#else
  x = conf.xpos;
  y = conf.ypos;
#endif
}

void webview_wrapper::center()
{
#ifdef _WIN32
  win_center((HWND)window());
#elif !defined(GTK_410_OR_MORE)
  // logTrace("GTK center webapp");
  gtk_window_set_position(GTK_WINDOW(window()), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_get_position(GTK_WINDOW(window()), &new_geom.x, &new_geom.y);
#endif

  save_conf();
}

void webview_wrapper::set_pos(int x, int y)
{
  if (x == -32000 || y == -32000)
  {
    center();
    return;
  }
#ifdef _WIN32
  SetWindowPos((HWND)window(), NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
#else
#ifdef GTK_410_OR_MORE
  // logTrace("Missing gtk_window_move in gtk4");
#else
  // logTrace("GTK410_LESS, moving window");
  gtk_window_move(GTK_WINDOW(window()), x, y);
#endif
#endif

  new_geom.x = x;
  new_geom.y = y;
  save_conf();
}

void webview_wrapper::get_size(int &wi, int &he)
{
#ifdef _WIN32
  RECT rc;
  GetWindowRect((HWND)window(), &rc);
  wi = rc.right - rc.left;
  he = rc.bottom - rc.top;
#else
  gtk_window_get_default_size(GTK_WINDOW(window()), &wi, &he);
#endif
  logDebug("get_size - width, height: ", wi, ',', he);
}

void webview_wrapper::border_size(int &left_border, int &right_border, int &bottom_border, int &top_border_with_title_bar)
{
#ifdef _WIN32
  HWND hwnd = (HWND)(window());
  RECT wrc;
  GetWindowRect(hwnd, &wrc);
  RECT crc;
  GetClientRect(hwnd, &crc);
  POINT lefttop = {crc.left, crc.top}; // Practically both are 0
  ClientToScreen(hwnd, &lefttop);
  POINT rightbottom = {crc.right, crc.bottom};
  ClientToScreen(hwnd, &rightbottom);

  left_border = lefttop.x - wrc.left;              // Windows 10: includes transparent part
  right_border = wrc.right - rightbottom.x;        // As above
  bottom_border = wrc.bottom - rightbottom.y;      // As above
  top_border_with_title_bar = lefttop.y - wrc.top; // There is no transparent part
/*  logDebug("adjust_size (", wi, ',', he, ") - WindowRect (", wrc.left, ',', wrc.top, ',', wrc.right, ',', wrc.bottom, ") - size(", wrc.right - wrc.left, ',', wrc.bottom - wrc.top, ')');
  logDebug("adjust (", wi, ',', he, ") - ClientRect (", crc.left, ',', crc.top, ',', crc.right, ',', crc.bottom, ") - size(", crc.right - crc.left, ',', crc.bottom - crc.top, ')');
  logDebug("adjust_size left_border : ", left_border, ", right_border : ", right_border, ", bottom_border : ", bottom_border, ", top_border_with_title_bar: ", top_border_with_title_bar);*/
#endif
}

void webview_wrapper::set_size(int &width, int &height, int hints)
{
#ifdef _WIN32
  // If required, then modify width and height to remove decoration borders
  if (hints != 4)
  {
    int lb, tb, rb, bb;
    // logDebug("set_size before adjust_size - width, height: ", width, ',', height);
    border_size(lb, tb, rb, bb);
    width -= (lb + rb);
    height -= (tb + bb);
    // logDebug("set_size  after adjust_size - width, height: ", width, ',', height);
  }
#endif

  if (hints < 0)
    hints = WEBVIEW_HINT_NONE;
  else if (hints >= WEBVIEW_HINT_MIN && hints <= WEBVIEW_HINT_FIXED)
  {
    WP->set_size(width, height, (webview_hint_t)hints);
    /*
        int owi, ohe;
        get_size(owi, ohe);
        if (owi > -1 && ohe > -1)
        {
          logDebug("set_size - old width, height: ", owi, ',', ohe);
          WP->set_size(owi, ohe, WEBVIEW_HINT_NONE);
        }
        else
          WP->set_size(width, height, WEBVIEW_HINT_NONE);
    */
  }
  else
  {
    WP->set_size(width, height, (webview_hint_t)hints);
  }

  new_geom.w = width;
  new_geom.h = height;
  save_conf();
}

void webview_wrapper::set_hint(int hint)
{
  int aw, ah;
  get_size(aw, ah);
  set_size(aw, ah, hint);
}

void disp_hints()
{
  return;
  logDebug("WEBVIEW_HINT_NONE ", WEBVIEW_HINT_NONE);
  logDebug("WEBVIEW_HINT_MIN ", WEBVIEW_HINT_MIN);
  logDebug("WEBVIEW_HINT_MAX ", WEBVIEW_HINT_MAX);
  logDebug("WEBVIEW_HINT_FIXED ", WEBVIEW_HINT_FIXED);
}

void webview_wrapper::set_geometry(int x, int y, int w, int h)
{
  logDebug("set_geometry: ", x, ',', y, ',', w, ',', h);
  set_pos(x, y);
  set_size(w, h);
  // save_conf(); // already done by set_pos and set_size ...
}

#ifdef _WIN32
std::string wp_flags(WINDOWPLACEMENT wp)
{
  std::string ret = {};
  if (wp.flags & WPF_SETMINPOSITION)
    ret += "unspecified minimized window state coordinate in ptMinPosition. ";
  else
    ret += "coordinate in ptMinPosition are specified for the minimized window state. ";
  if (wp.flags & WPF_RESTORETOMAXIMIZED)
    ret += "The next restored window state will be maximized. ";
  if (wp.flags & WPF_ASYNCWINDOWPLACEMENT)
    ret += "Asyncronous window placement.";
  return ret;
}

std::string wp_show_mode(WINDOWPLACEMENT wp)
{
  const std::string show_mode[] = {"hide", "shownormal", "showminimized", "showmaximized", "shownoactivate", "show", "minimize", "showminnoactive", "showna", "restore", "showdefault", "forceminimize"};
  std::string ret = {};
  if (wp.showCmd >= 0 && wp.showCmd < 12)
    ret = show_mode[wp.showCmd];
  else
    ret = "Undefined show mode";
  return ret;
}

void logPlacement(WINDOWPLACEMENT wp)
{
  static WINDOWPLACEMENT previous_wp;

  if (memcmp((void *)&previous_wp, (void *)&wp, sizeof(WINDOWPLACEMENT)) != 0)
  {
    logDebug("WPL. ");
    logDebug("WPL. Flags: ", wp_flags(wp));
    logDebug("WPL. ShowCmd: ", wp_show_mode(wp));
    logDebug("WPL. Coordonnées de l’angle supérieur gauche de la fenêtre lorsque la fenêtre est réduite,  ptMinPosition: (", wp.ptMinPosition.x, ',', wp.ptMinPosition.y, ')');
    logDebug("WPL. Coordonnées de l’angle supérieur gauche de la fenêtre lorsque la fenêtre est agrandie, ptMaxPosition: (", wp.ptMaxPosition.x, ',', wp.ptMaxPosition.y, ')');
    logDebug("WPL. Coordonnées de la fenêtre lorsque la fenêtre est dans la position restaurée, rcNormalPosition: (", wp.rcNormalPosition.left, ',', wp.rcNormalPosition.top, ',', wp.rcNormalPosition.right, ',', wp.rcNormalPosition.bottom, ')');
    //  logDebug("WPL. rcDevice: (", wp.rcDevice.left, ',', wp.rcDevice.top, ',', wp.rcDevice.right, ',', wp.rcDevice.bottom, ')');
    previous_wp = wp;
  }
}
#else

#if GTK_MAJOR_VERSION <= 3
rectangle gtk_get_monitor_rectangle(GtkWindow *gw)
{
  GdkDisplay *gd = gtk_widget_get_display(GTK_WIDGET(gw));
  GdkMonitor *gm = gdk_display_get_monitor_at_window(gd, GDK_WINDOW(gw));
  GdkRectangle gr;
  gdk_monitor_get_geometry(gm, &gr);
  return rectangle({gr.x, gr.y, gr.width, gr.height});
}
#endif
#endif

bool write_ini(std::string fname, webview_conf wvc)
{
  logDebug("IN write_ini: ", wvc.xpos, ',', wvc.ypos, ',', wvc.width, ',', wvc.height);

  if (std::filesystem::exists(fname))
  {
    struct eInf
    {
      bool saved;
      std::string sval;
    };
    std::map<std::string, eInf> to_save = {
        {"debug", {false, wvc.debug ? "true" : "false"}}, {"x", {false, std::to_string(wvc.xpos)}}, {"y", {false, std::to_string(wvc.ypos)}}, {"w", {false, std::to_string(wvc.width)}}, {"h", {false, std::to_string(wvc.height)}}};

    std::fstream fs(fname, std::ios::in | std::ios::binary);
    std::string line, section = "global", key, val, com;
    std::smatch m1, m2;
    int nl = 0;
    const std::regex re1("(.*)=(.*)"), re2("(.*)#(.*)");
    std::ostringstream oss;

    while (std::getline(fs, line))
    {
      trim(line);
      nl++;

      if (line.empty() || line[0] == '#')
      {
        oss << line << std::endl;
        continue;
      }

      if (line[0] == '[' && line.back() == ']')
      {
        oss << line << std::endl;
        line.pop_back();
        section = line.substr(1);
        continue;
      }

      if (std::regex_match(line, m1, re1) && m1.size() == 3)
      {
        key = ctrim(std::ssub_match(m1[1]).str());
        val = ctrim(std::ssub_match(m1[2]).str());

        if (std::regex_match(val, m2, re2) && m2.size() == 3)
        {
          val = ctrim(std::ssub_match(m2[1]).str());
          com = " # " + ctrim(std::ssub_match(m2[2]).str());
        }
        else
        {
          com = {};
        }

        // What do we do regarding the currents section and key name ?
        if (to_save.count(key))
        {
          to_save[key].saved = true;
          oss << key << " = " << to_save[key].sval << com << std::endl;
          logDebug("to save: ", key, ':', to_save[key].sval);
        }
        else
        {
          oss << key << " = " << val << com << std::endl;
        }
      }
      else
      {
        // logWarn("Commenting the syntax error at line: ", nl, ": '", line, "'");
        oss << "# Line with syntax error that has been commented out: [" << line << ']' << std::endl;
      }
    }

    for (auto elt : to_save)
    {
      if (!elt.second.saved)
        oss << elt.first << " = " << elt.second.sval << std::endl;
    }

    // Replace the config file by the updated one
    fs.close();
    fs.open(fname, std::ios::out | std::ios::trunc | std::ios::binary);
    fs << oss.str();
    fs.close();
    return true;
  }
  else
  {
    // logTrace("NOT parsing conf file", fname, " as it does not exists or has empty file name");
  }

  return false;
}

typedef std::map<std::string, std::string> ConfigInfo;

// Not taking section into account for now ...
bool read_ini(std::string fname, ConfigInfo &cv)
{
  if (std::filesystem::exists(fname))
  {
    std::ifstream ifs(fname);
    std::string line, key;

    while (std::getline(ifs, line))
    {
      trim(line);
      std::istringstream is_line(line);

      if (std::getline(is_line, key, '='))
      {
        if (key[0] == '#')
          continue;
        trim(key);
        std::getline(is_line, cv[key], '#');
        trim(cv[key]);
        logDebug("read_ini, key: [", key, "] = [", cv[key], "]");
      }
    }

    ifs.close();
    return true;
  }

  return false;
}

// Si le fichier fname existe alors on essaye de récupérer les valeurs qu'il contient.
// Format du fichier : une paire "clef=valeur" par ligne. Les lignes vides et tout ce qui suit # (commentaire) sont ignorés.
// Les clefs prises en compte pour l'instant sont : x, y, w, h
bool webview_wrapper::restore_conf(webview_conf &p_cnf, std::string fname)
{
  ConfigInfo cv;
  logDebug("restore_conf bef read_ini");
  if (read_ini(fname, cv))
  {
    logDebug("restore_conf aft read_ini");
    int x_offset = 0, y_offset = 0;
#ifndef WIN32
    // Trying to compensate the offset I noticed between 2 gtk_move_window
    x_offset = -5;
    y_offset = -29;
#endif

    if (cv.count("init_state"))
      p_cnf.init_win_state = (win_state)std::stoi(cv["init_state"]); // => hidden = 0, normal = 1, minimized = 2, maximized = 3

    if (cv.count("debug"))
      p_cnf.debug = str2bool(cv["debug"]);

    if (cv.count("x"))
      p_cnf.xpos = std::stoi(cv["x"]) + x_offset;

    if (cv.count("y"))
      p_cnf.ypos = std::stoi(cv["y"]) + y_offset;

    if (cv.count("w"))
      p_cnf.width = std::stoi(cv["w"]);

    if (cv.count("h"))
      p_cnf.height = std::stoi(cv["h"]);

    return true;
  }

  return false;
}

// new_geom must be correctly set before calling may_save_conf
bool webview_wrapper::may_save_conf()
{
  static bool wait_for_same_size = true;
  static webview_conf *old_conf = nullptr;

  if (!old_conf)
  {
    old_conf = new webview_conf;
    if (!restore_conf(*old_conf))
      old_conf = nullptr;
  }

  bool ret = false;

  logDebug("conf (", conf.xpos, ",", conf.ypos, ",", conf.width, ",", conf.height, ")");
  logDebug("new_geom (", new_geom.x, ",", new_geom.y, ",", new_geom.w, ",", new_geom.h, ")");
  // Here we have the object new_geom that is correctly set
  if (conf.xpos != new_geom.x)
  {
    conf.xpos = new_geom.x;
    setvar("app", "x", new_geom.x);
    ret = true;
  }
  if (conf.ypos != new_geom.y)
  {
    conf.ypos = new_geom.y;
    setvar("app", "y", new_geom.y);
    ret = true;
  }

  if (wait_for_same_size)
  {
    if (conf.width != new_geom.w && conf.height != new_geom.h)
    {
      if (me)
      {
        //        logDebug("conf.width: ", conf.width, ", conf.height: ", conf.height, ", conf.hint: ", conf.hint);
        me->set_size(conf.width, conf.height, conf.hint);
      }
      return false;
    }
    else
      wait_for_same_size = false;
  }

  if (conf.width != new_geom.w)
  {
    conf.width = new_geom.w;
    setvar("app", "w", new_geom.w);
    ret = true;
  }
  if (conf.height != new_geom.h)
  {
    conf.height = new_geom.h;
    setvar("app", "h", new_geom.h);
    ret = true;
  }

  if (old_conf)
  {
    old_conf->xpos = new_geom.x;
    old_conf->ypos = new_geom.y;
    old_conf->width = new_geom.w;
    old_conf->height = new_geom.h;
  }

  return ret;
}

// new_geom must be correctly set before calling save_conf
bool webview_wrapper::save_conf(std::string fname)
{
  if (me && may_save_conf())
  {
    out_conf("save on: " + fname);
    logDebug("bef write_ini");
    write_ini(fname, conf);
    return true;
  }

  return false;
}

void webview_wrapper::set_on_geometry(const std::string js)
{
  on_geometry_func = js;
  //  logTrace("set_on_geometry ", on_geometry_func );
}

void webview_wrapper::set_on_exit(const std::string js)
{
  on_exit_func = js;
  // logTrace("set_on_exit ", on_exit_func);
}

void webview_wrapper::set_html(const std::string &html)
{
  WP->set_html(html);
}

void webview_wrapper::init(const std::string &js)
{
  WP->init(js);
}

void webview_wrapper::eval(const std::string &js)
{
  // logTrace(js);
  WP->eval(js);
}

std::string webview_wrapper::version()
{
  return std::string(webview_version()->version_number) + std::string(webview_version()->pre_release) + std::string(webview_version()->build_metadata);
}

//// Class management
// Create and instanciate a class
// Class name is name with first character in uppercase
// Instance name is name with first character in lowercase

extern bool js_instance;
void mk_arg_list(int n, std::string &al, std::string &ali)
{
  // On limite les fonctions variadic à 20 parametres ...
  if (n == INT_MAX)
    n = 20;

  al = ali = {};
  for (int i = 0; i < n; i++)
  {
    al += "p" + std::to_string(i);
    ali += "p" + std::to_string(i) + "=''";
    if (i < n - 1)
    {
      al.push_back(',');
      ali.push_back(',');
    }
  }
}

bool sort_class_def(const tupple_class &a, const tupple_class &b)
{
  return std::tie(std::get<0>(a), std::get<1>(a), std::get<2>(b), std::get<3>(b)) < std::tie(std::get<0>(b), std::get<1>(b), std::get<2>(a), std::get<3>(b));
}

// Bind name must contains at least one underscore '_' which separate class name from method name
// A class will be declared, if not already done, with a name corresponding to the left hand side of the underscore whose first char is converted to uppercase
// A methode will be defined with a name corresponding to the right hand side of the underscore with all characters converted to lowercase
// If there is no undescore in the bind name then the method will put in class named Catchall
// tupple_class element are :
// 0: instance name, 1: func name, 2: nb args, 3: bind_name
// class instance name is different from class name only by the first character:
//  it is uppercase for class and lower case for instance
// Return "class_instance_name.func_name"
std::string webview_wrapper::store_classes(std::string bind_name, int narg)
{
  if (!js_instance)
    return bind_name;

  std::string bname = bind_name;
  auto pos = bname.find_first_of('_');
  narg = abs(narg);

  if (pos == std::string::npos)
  {
    // Convert whole bind name to lower case as it will be the func name
    // std::transform(bname.begin(), bname.end(), bname.begin(), ::tolower);
    classes_def.push_back({"catchall", bname, narg, bind_name});
    return "catchall." + bname;
  }
  else
  {
    std::string cinam = bname.substr(0, pos);
    // First char of class name to uppercase
    cinam[0] = tolower(cinam[0]);
    std::string fnam = bname.substr(pos + 1);
    // Convert func name to lower case
    // std::transform(fnam.begin(), fnam.end(), fnam.begin(), ::tolower);
    classes_def.push_back(std::make_tuple(cinam, fnam, narg, bind_name));
    return cinam + '.' + fnam;
  }
}

// using sync_binding_t = std::function<std::string(std::string)>;
void webview_wrapper::bind_doc(const std::string &name, sync_binding_t fn, const std::string desc, int narg) //, bool indoc)
{
  std::string cfinam = store_classes(name, narg);
  WP->bind(name, fn);
  //  if (indoc)
  func_help.push_back(std::make_tuple(false, cfinam, narg, desc));
}

// using binding_t = std::function<void(std::string, std::string, void *)>;
void webview_wrapper::bind_doc(const std::string &name, binding_t fn, const std::string desc, int narg) //, void *arg, bool indoc)
{
  std::string cfinam = store_classes(name, narg);
  WP->bind(name, fn, nullptr);
  //  if (indoc)
  func_help.push_back(std::make_tuple(true, cfinam, narg, desc));
}

std::string assign_jsvar(std::string &cname, std::string vname, std::filesystem::path pval, const bool readonly, const bool is_string, const bool del = true)
{
  //	sval=creplace_all(sval, "\\", "\\\\");
  std::string sval;
  if (is_string)
    sval = path_to_htent(pval);
  else
    sval = pval.string();

  if (is_string)
    sval = '"' + sval + '"';
  std::string iname;

  if (cname.empty())
    iname = "window";
  else
  {
    iname = cname;
    iname[0] = tolower(iname[0]);
    cname[0] = toupper(cname[0]);

    if (!js_instance)
    {
      vname = iname + '_' + vname;
      iname = "window";
      cname = "";
    }
  }

  std::string assign = "";
  if (del)
    assign = "delete " + iname + '.' + vname + ";\n";

  // assign += "Object.defineProperty(" + iname + ", \"" + vname + "\", {writable:true});";
  assign += iname + '.' + vname + '=' + sval + ";\n";

  if (readonly)
    assign += "Object.defineProperty(" + iname + ", \"" + vname + "\", {writable:false});";
  // logTrace("ASSGNVAR: ", assign);
  return assign;
}

void webview_wrapper::decvar(const std::filesystem::path &pcname, const std::filesystem::path &pvname, const std::string &desc, const std::filesystem::path &sval, const bool readonly, const bool is_string)
{
  std::string cname = path_to_htent(pcname);
  std::string vname = path_to_htent(pvname);

  std::string assign = assign_jsvar(cname, vname, sval, readonly, is_string, false);
  vars_init.insert({cname, assign});

  cname[0] = tolower(cname[0]);
  std::string ro, more_desc_text;

  if (readonly)
  {
    ro = "true";
    more_desc_text = "-";
  }
  else
  {
    ro = "false";
    more_desc_text = " ";
  }

  more_desc_text += vname + ": " + desc;

  // { "object": "app", "name":  "x", "readonly": true, "desc":  "...", }
  std::string more_desc_json = "{\"object\": \"" + cname + "\",\"var_name\": \"" + vname + "\", \"readonly\": " + ro + ", \"desc\": \"" + desc + "\" }";
  // logDebug("DECVAR:" + more_desc_json+" ; val: "+sval);

  logDebug("DECVAR:", more_desc_text, " ; val: ", sval);

  vars_desc_json.insert({cname, more_desc_json});
  vars_desc_text.insert({cname, more_desc_text});
}

void webview_wrapper::decvar(const std::string &cname, const std::string &vname, const std::string &desc, const std::string &sval, const bool readonly, const bool is_string)
{
  decvar(std::filesystem::path(cname), std::filesystem::path(vname), desc, std::filesystem::path(sval), readonly, is_string);
}

void webview_wrapper::decvar(const std::string &cname, const std::string &vname, const std::string &desc, const char *sval, const bool readonly, const bool is_string)
{
  if (sval != nullptr)
    decvar(std::filesystem::path(cname), std::filesystem::path(vname), desc, std::filesystem::path(sval), readonly, is_string);
}

void webview_wrapper::decvar(const std::string &cname, const std::string &vname, const std::string &desc, const bool bval, const bool readonly)
{
  if (bval)
    decvar(cname, vname, desc, std::string("true"), readonly, false);
  else
    decvar(cname, vname, desc, std::string("false"), readonly, false);
}

void webview_wrapper::decvar(const std::string &cname, const std::string &vname, const std::string &desc, const int ival, const bool readonly)
{
  decvar(cname, vname, desc, std::to_string(ival), readonly, false);
}

void webview_wrapper::decvar(const std::string &cname, const std::string &vname, const std::string &desc, const float fval, const bool readonly)
{
  decvar(cname, vname, desc, std::to_string(fval), readonly, false);
}

void webview_wrapper::decvar(const std::string &cname, const std::string &vname, const std::string &desc, const unsigned long long ullval, const bool readonly)
{
  decvar(cname, vname, desc, std::to_string(ullval), readonly, false);
}

std::string class_declvars(std::string cname, std::multimap<std::string, std::string> vars_init)
{
  std::string res = "";
  for (auto range = vars_init.equal_range(cname); auto &[_, value] : std::ranges::subrange(range.first, range.second))
  {
    // logTrace("DECLVARS: ", value);
    res += value + '\n';
  }

  return res;
}

std::string webview_wrapper::class_descvars(std::string fname, bool json)
{
  // logTrace(fname);
  static std::string last_cname = "";
  std::string cname = split(fname, '.')[0];
  cname[0] = tolower(cname[0]);

  if (last_cname == cname)
    return "";
  last_cname = cname;
  std::string res = "";

  if (json)
  {
    res += "{\"vars\": [";
    for (auto range = vars_desc_json.equal_range(cname); auto &[_, value] : std::ranges::subrange(range.first, range.second))
    {
      res += value + ",";
      logDebug("DESCVARS_JSON: " + value);
    }

    if (res == "{\"vars\": [")
      res = "";
    else
    {
      // Remove last comma
      res.pop_back();
      res += "]},";
    }
  }
  else
  {
    for (auto range = vars_desc_text.equal_range(cname); auto &[_, value] : std::ranges::subrange(range.first, range.second))
    {
      res += value.insert(1, cname + '.') + '\n';
      // res += value + '\n';
      logDebug("DESCVARS_TEXT: ", value, ", cname: ", cname);
    }
    //    if (!res.empty()) res = "<i><u>Variables for object '"+cname+"':</u>\nRead-only ones are prepended by a '-'</i>\n" + res + "<i><u>Functions:</u>\nAsynchronous ones are prepended by a '*'</i>\n";
  }

  logDebug("class_descvars", res);
  return res;
}

void webview_wrapper::setvar(const std::filesystem::path &pcname, const std::filesystem::path &pvname, const std::filesystem::path &sval, const bool readonly, const bool is_string)
{
  if (me)
  {
    std::string cname = path_to_htent(pcname);
    std::string vname = path_to_htent(pvname);

    std::string s = assign_jsvar(cname, vname, sval, readonly, is_string, true);
    // logTrace("SETVAR: ", s);
    eval(s);
  }
}

void webview_wrapper::setvar(const std::string &cname, const std::string &vname, const std::string &sval, const bool readonly, const bool is_string)
{
  setvar(std::filesystem::path(cname), std::filesystem::path(vname), std::filesystem::path(sval), readonly, is_string);
}

void webview_wrapper::setvar(const std::string &cname, const std::string &vname, const char *sval, const bool readonly, const bool is_string)
{
  if (sval != nullptr)
    setvar(std::filesystem::path(cname), std::filesystem::path(vname), std::filesystem::path(sval), readonly, is_string);
}

void webview_wrapper::setvar(const std::string &cname, const std::string &vname, const bool bval, const bool readonly)
{
  if (bval)
    setvar(cname, vname, std::string("true"), readonly, false);
  else
    setvar(cname, vname, std::string("false"), readonly, false);
}

void webview_wrapper::setvar(const std::string &cname, const std::string &vname, const int ival, const bool readonly)
{
  setvar(cname, vname, std::to_string(ival), readonly, false);
}

void webview_wrapper::setvar(const std::string &cname, const std::string &vname, const float fval, const bool readonly)
{
  setvar(cname, vname, std::to_string(fval), readonly, false);
}

// Finalize classes_def by defining methods and adding instance of all classes
void webview_wrapper::bind_classes()
{
  std::sort(classes_def.begin(), classes_def.end(), sort_class_def);

  std::string def = {}, ccl = {}, cci = {};
  // Declare all the classes and their methods
  for (auto cd : classes_def)
  {
    if (std::get<0>(cd) != cci)
    {
      // Instantiate previous class
      if (!cci.empty())
      {
        if (js_instance)
          def += "}\nvar " + cci + " = new " + ccl + "();\n";
        def += class_declvars(ccl, vars_init) + '\n';
      }

      // And declare next class
      cci = std::get<0>(cd);
      ccl = std::toupper(cci.front());
      ccl += cci.substr(1);
      if (js_instance)
        def += "class " + ccl + " {\n";
    }

    std::string al, ali;
    mk_arg_list(std::get<2>(cd), al, ali);

    // Define method with its eventual arguments
    if (js_instance)
      def += std::get<1>(cd) + "(" + ali + ") { return " + std::get<3>(cd) + "(" + al + "); }\n";
  }

  // Instantiate last class
  if (std::get<0>(classes_def.back()) == cci)
  {
    if (js_instance)
      def += "}\nvar " + cci + " = new " + ccl + "();\n";
    def += class_declvars(ccl, vars_init) + '\n';
  }

  // logTrace("=== Start of Classes and instances/objects declaration ===");
  // logTrace(def);
  // logTrace("=== End   of Classes and instances/objects declaration ===");
  init(def);
}
