
#ifndef WEBVIEW_WRAPPER_H
#define WEBVIEW_WRAPPER_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <functional>
#include <map>
#include <string>
#include <vector>

std::wstring my_widen_string(const std::string &input);
std::string my_narrow_string(const std::wstring &input);

using binding_t = std::function<void(std::string, std::string, void *)>;
using sync_binding_t = std::function<std::string(std::string)>;

/*
 Format de la doc des fonctions : un n-uplet  "[*] clef=valeur" par ligne.
 Une étoile devant le nom indique une fonction asynchrone/promesse
 On peut insérer des <RC>

 Organisation des objets du n-uplet :
 1er : fonction asynchrone (true) ou pas (false)
 2nd : nom de la fonction
 3th : nombre de parametres de la fonction:
     0 : pas de paramètre
     1 : 1 parametre requis
     2 : 2 parametres requis
     n : n parametres requis
    -1 : 0 ou 1 parametre requis
    -2 : de 0 à 2 parametres requis.
    -n : de 0 à n parametres requis.
    ==> INT_MAX indique une fonction varadique
 4rd : description de la fonction
*/
using tupple_doc = std::tuple<bool, std::string, int, std::string>;

// tupple_class element are :
// 0: instance name, 1: func name, 2: nb args, 3: bind_name
using tupple_class = std::tuple<std::string, std::string, int, std::string>;

std::string json_parse(const std::string &s, const std::string &key, const int index);
void debug_js_parse(const std::string);

bool try_stoi(std::string s, int &n);
bool try_stoll(std::string s, long long &n);
bool try_stof(std::string s, float &n);

bool get_params(const std::string, std::string &);
bool get_parami(const std::string, int &);
bool get_paramf(const std::string, float &);
void reset_param();

template <typename... TypeStr> void js_params(const std::string r, TypeStr &...arg)
{
  ([&] { get_params(r, arg); }(), ...);
  reset_param();
}
template <typename... TypeInt> void js_parami(const std::string r, TypeInt &...arg)
{
  ([&] { get_parami(r, arg); }(), ...);
  reset_param();
}
template <typename... TypeFlt> void js_paramf(const std::string r, TypeFlt &...arg)
{
  ([&] { get_paramf(r, arg); }(), ...);
  reset_param();
}

constexpr bool is_json_special_char(unsigned int c);
constexpr bool is_control_char(unsigned int c);

enum win_state
{
  hidden = 0,
  normal = 1,
  minimized = 2,
  maximized = 3
};

struct rectangle
{
  int x;
  int y;
  int w;
  int h;
};

struct webview_conf
{
  win_state init_win_state;
  bool debug, status, zoom, ctx_menu, psw_sav, auto_fill;
  int xpos, ypos, width, height, hint;
};

class webview_wrapper
{
private:
  static webview_wrapper *me;

  std::string on_geometry_func = {}, on_exit_func = {};
  void *w = nullptr;
  std::vector<tupple_doc> func_help = {};
  std::string app_title;
  std::vector<tupple_class> classes_def = {};
  std::string store_classes(std::string bind_name, int narg);
  std::multimap<std::string, std::string> vars_init = {};
  std::multimap<std::string, std::string> vars_desc_json = {};
  std::multimap<std::string, std::string> vars_desc_text = {};
  std::string class_descvars(std::string fname, bool json);

public:
  static webview_conf conf;
  std::string js_args = "[]";

  void out_conf(std::string s = {});
  webview_wrapper();
  webview_wrapper(bool debug, void *wnd = nullptr);
  void create(void *wnd = nullptr);
  void create(bool debug = false, void *wnd = nullptr);
  ~webview_wrapper();
  void bind(const std::string &name, sync_binding_t fn);
  void bind(const std::string &name, binding_t fn, void *arg);

  void bind_doc(const std::string &name, sync_binding_t fn, const std::string desc = {}, int narg = 0); //, bool indoc = true);
  void bind_doc(const std::string &name, binding_t fn, const std::string desc = {}, int narg = 0);      //, void *arg = nullptr, bool indoc = true);

  void def_class_instance(std::string name, std::string construct_code = "constructor() {}");

  std::string json_escape(const std::string &s, bool add_quotes = true);
  void resolve(const std::string &seq, int status, const std::string &result);

  void navigate(const std::string &url);
  void set_js_args(int argc, int optind, char **argv);
  void run();
  void unbind(const std::string &name);
  void *window();
  void terminate();
  void dispatch(std::function<void()>);

  void show();
  void hide();
  void restore();
  void minimize();
  void maximize();
  void enter_fullscreen();
  void exit_fullscreen();
  std::string get_title();
#ifdef _WIN32
  std::wstring get_title_w();
#endif
  std::string set_title(const std::string &);
  std::string icon_file; // Always updated by set_icon
  std::string set_icon(const std::string);
  void set_pos(int, int);
  void get_pos(int &, int &);
  void get_size(int &, int &);
  void border_size(int &, int &, int &, int &);
  void set_size(int &, int &, int hints = -1);
  void set_geometry(int x, int y, int w, int h);
  void set_hint(int);
  void center();
  bool restore_conf(webview_conf &, std::string fname = ".config.ini");
  bool may_save_conf();
  bool save_conf(std::string fname = ".config.ini");
  std::string help_objs(std::string fmt = "txt");

  void decvar(const std::filesystem::path &cname, const std::filesystem::path &vname, const std::string &desc = {}, const std::filesystem::path &sval = {}, const bool readonly = true, const bool is_string = true);
  void decvar(const std::string &cname, const std::string &vname, const std::string &desc = {}, const std::string &sval = {}, const bool readonly = true, const bool is_string = true);
  void decvar(const std::string &cname, const std::string &vname, const std::string &desc = {}, const char *sval = nullptr, const bool readonly = true, const bool is_string = true);
  void decvar(const std::string &cname, const std::string &vname, const std::string &desc = {}, const bool bval = false, const bool readonly = true);
  void decvar(const std::string &cname, const std::string &vname, const std::string &desc = {}, const int ival = 0, const bool readonly = true);
  void decvar(const std::string &cname, const std::string &vname, const std::string &desc = {}, const float fval = 0, const bool readonly = true);
  void decvar(const std::string &cname, const std::string &vname, const std::string &desc = {}, const unsigned long long ullval = 0, const bool readonly = true);

  void setvar(const std::filesystem::path &cname, const std::filesystem::path &vname, const std::filesystem::path &sval = {}, const bool readonly = true, const bool is_string = true);
  void setvar(const std::string &cname, const std::string &vname, const std::string &sval = {}, const bool readonly = true, const bool is_string = true);
  void setvar(const std::string &cname, const std::string &vname, const char *sval = nullptr, const bool readonly = true, const bool is_string = true);
  void setvar(const std::string &cname, const std::string &vname, const bool bval = false, const bool readonly = true);
  void setvar(const std::string &cname, const std::string &vname, const int ival = 0, const bool readonly = true);
  void setvar(const std::string &cname, const std::string &vname, const float fval = 0, const bool readonly = true);
  //  template <typename... TypeStr> void setvars(const std::string cname, TypeStr &...arg) { ([&] { setvar(cname, arg); }(), ...); }

  static void on_geom();
#ifdef _WIN32
  static LRESULT windows_on_event(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
  void set_dark_bar(bool set = true);
#endif

#ifdef WEBVIEW_PLATFORM_LINUX_WEBKITGTK_COMPAT_HH
#if GTK_MAJOR_VERSION == 3
  static bool gtk_on_configure_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
  static bool gtk_on_window_state_event(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data);
  // #else
  //   static bool gtk_on_configure_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
  //   static bool gtk_on_window_state_event(GtkWidget *widget, GdkEvent *event, gpointer user_data);
  static GdkWindowState current_window_state;
#endif
//  GtkWindow* m_window();
#endif

  void set_on_exit(const std::string);
  void set_on_geometry(const std::string);
  void set_html(const std::string &);
  void init(const std::string &);
  void eval(const std::string &);
  static std::string version();

  void bind_classes();
};

#endif /* WEBVIEW_WRAPPER_H */
