
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
#include "options.h"
#include "wrap.h"
#include "path_entity.h"
// clang-format on

#define trc std::cout << __FILE__ << ": " << __LINE__ << std::endl

#ifdef __linux__
#include <gtk/gtk.h>

enum pathdlg_type
{
  file_open,
  file_save,
  folder_sel
};

struct pathdlg_res
{
  pathdlg_type type;
  bool done = false;
  std::string path = "";
};

// Une chaîne de filtre est composée d'une ou plusieurs descriptions de filtre, séparées par une barre verticale (|). Une description de filtre est composée d'un ou plusieurs modèles de filtre séparés par des virgules, éventuellement terminés par le nom du
// filtre séparé par un point-virgule (;). Tous les séparateurs peuvent être échappés par une barre oblique inverse. (&bsol;), si besoin. Un exemple de filtre : "*.c,*.c++;C/C++ files|*.html,*.js,*.css;Web files (HTML, Javascript and CSS)"

#if GTK_MAJOR_VERSION >= 4 && GTK_MINOR_VERSION >= 10
void build_filter(GtkFileDialog *dialog, std::string s)
#else
void build_filter(GtkWidget *dialog, std::string s)
#endif
{
#if GTK_MAJOR_VERSION >= 4 && GTK_MINOR_VERSION >= 10
  GListStore *filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
  GtkFileFilter *def_filter = nullptr;
#endif

  auto vFilt = split_esc(s, '|', '\\');
  for (size_t i = 0; i < vFilt.size(); i++)
  {
    auto vDesc = split_esc(vFilt[i], ';', '\\');
    if (vDesc.size() > 0)
    {
      std::string name = {};
      // vDesc[1] ==> name
      if (vDesc.size() > 1 && vDesc[1].size() > 0)
        name = vDesc[1];
      else
        name = "Filter for " + vDesc[0]; // Default name

      // vDesc[0] ==> vPatt
      auto vPatt = split_esc(vDesc[0], ',', '\\');
      if (vPatt.size() > 0)
      {
        GtkFileFilter *filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, name.c_str());

        for (auto patt : vPatt)
        {
          gtk_file_filter_add_pattern(filter, patt.c_str());
        }

#if GTK_MAJOR_VERSION >= 4 && GTK_MINOR_VERSION >= 10
        g_list_store_append(filters, filter);
        if (def_filter == nullptr)
          def_filter = filter;
        else
          g_object_unref(filter);
#else
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
#endif
      }
    }
  }

#if GTK_MAJOR_VERSION >= 4 && GTK_MINOR_VERSION >= 10
  if (def_filter != nullptr)
  {
    gtk_file_dialog_set_default_filter(GTK_FILE_DIALOG(dialog), def_filter);
    g_object_unref(def_filter);
  }
  gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
  g_object_unref(filters);
#endif
}

#if GTK_MAJOR_VERSION >= 4 && GTK_MINOR_VERSION >= 10
void ready_cb(GObject *source, GAsyncResult *result, gpointer user_data)
{
  pathdlg_res *pres = (static_cast<pathdlg_res *>(user_data));
  GFile *path = nullptr;

  switch (pres->type)
  {
  case folder_sel:
    path = gtk_file_dialog_select_folder_finish(GTK_FILE_DIALOG(source), result, nullptr);
    break;
  case file_save:
    path = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source), result, nullptr);
    break;
  case file_open:
  default:
    path = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source), result, nullptr);
    break;
  }

  if (path)
    pres->path = g_file_get_parse_name(path);
  pres->done = true;
}

void pathdlg_set(GtkWindow *parent, pathdlg_res &pres, std::string sfilt = {}, std::string initialDir = ".", std::string title = "")
{
  GtkFileDialog *dialog;
  dialog = gtk_file_dialog_new();
  build_filter(dialog, sfilt);

  if (!initialDir.empty() && initialDir != ".")
    gtk_file_dialog_set_initial_folder(dialog, g_file_parse_name(initialDir.c_str()));

  switch (pres.type)
  {
  case folder_sel:
    gtk_file_dialog_select_folder(dialog, parent, nullptr, ready_cb, &pres);
    break;
  case file_save:
    gtk_file_dialog_save(dialog, parent, nullptr, ready_cb, &pres);
    break;
  case file_open:
  default:
    gtk_file_dialog_open(dialog, parent, nullptr, ready_cb, &pres);
    break;
  }
  g_object_unref(dialog);
}

#else

pathdlg_res *gpres;
void ready_cb(GtkDialog *dialog, int response)
{
  if (response == GTK_RESPONSE_ACCEPT)
  {
    GFile *file = gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
    if (file)
      gpres->path = g_file_get_parse_name(file);
  }

#if GTK_MAJOR_VERSION >= 4
  gtk_window_destroy(GTK_WINDOW(dialog));
#else
  gdk_window_destroy(GDK_WINDOW(dialog));
#endif
  gpres->done = true;
}

void pathdlg_set(GtkWindow *parent, pathdlg_res &pres, std::string sfilt = {}, std::string initialDir = ".", std::string title = "")
{
  GtkWidget *dialog;
  GtkFileChooserAction action;
  gpres = &pres;

  switch (pres.type)
  {
  case folder_sel:
    if (title.empty())
      title = "Select folder";
    action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    break;
  case file_save:
    if (title.empty())
      title = "Save file";
    action = GTK_FILE_CHOOSER_ACTION_SAVE;
    break;
  case file_open:
  default:
    if (title.empty())
      title = "Open file";
    action = GTK_FILE_CHOOSER_ACTION_OPEN;
    break;
  }

  dialog = gtk_file_chooser_dialog_new(title.c_str(), parent, action, "_Cancel", GTK_RESPONSE_CANCEL, "_Open", GTK_RESPONSE_ACCEPT, nullptr);

  if (!initialDir.empty() && initialDir != ".")
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), initialDir.c_str());

  if (action == GTK_FILE_CHOOSER_ACTION_SAVE)
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

  build_filter(dialog, sfilt);

  gtk_window_present(GTK_WINDOW(dialog));
  g_signal_connect(dialog, "response", G_CALLBACK(ready_cb), &pres);
}
#endif

std::string pathdlg(webview_wrapper &w, const std::string &req, pathdlg_type ptyp = file_open)
{
#if GTK_MAJOR_VERSION >= 4 && GTK_MINOR_VERSION >= 10
  if (!gtk_init_check())
#else
  if (!gtk_init_check(0, nullptr))
#endif
  {
    // logTrace("gtk_init_check KO, aborting GTK.");
    return "";
  }

  static pathdlg_res pres;
  pres.path = {};
  pres.type = ptyp;
  pres.done = false;
  std::string sfilt, initialDir, title;
  js_params(req, sfilt, initialDir, title);
  pathdlg_set(GTK_WINDOW(w.window()), pres, sfilt, initialDir, title);
  while (!pres.done)
  {
    g_main_context_iteration(nullptr, TRUE);
  }
  std::cout << pres.path << std::endl;
  return w.json_escape(pres.path);
}

#if GTK_MAJOR_VERSION >= 4 && GTK_MINOR_VERSION >= 10
struct msg_res
{
  bool done = false;
  std::string response = "false";
};

void on_msgbox_click(GObject *source_object, GAsyncResult *res, msg_res *ret)
{
  GtkAlertDialog *dialog = GTK_ALERT_DIALOG(source_object);
  GError *err = NULL;
  if (gtk_alert_dialog_choose_finish(dialog, res, &err) == 0)
    ret->response = "true";
  else
    ret->response = "false";
  if (err)
    logError("An error occured!, Error Message: ", err->message);
  ret->done = true;
}

// Possible values for type are : MB_OK=0, MB_OKCANCEL=1, MB_YESNO=2, MB_YESNOCANCEL=3
// Return is "yes", "no", "ok", "cancel"
std::string gtk_msgbox(webview_wrapper &w, std::string msg, int type = 0)
{
  GtkWindow *parent = GTK_WINDOW(w.window());
  if (msg.empty())
    msg = "Alert";
  GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", w.get_title().c_str());
  // logInfo("GTK >= 4.10, TITLE: "+w.get_title());
  gtk_alert_dialog_set_detail(dialog, msg.c_str());

  const char *labels_ok_cancel[] = {g_dgettext("gtk30", "_OK"), g_dgettext("gtk30", "_Cancel"), nullptr};
  const char *labels_yes_no[] = {g_dgettext("gtk30", "_Yes"), g_dgettext("gtk30", "_No"), nullptr};
  const char *labels_yes_no_cancel[] = {g_dgettext("gtk30", "_Yes"), g_dgettext("gtk30", "_No"), g_dgettext("gtk30", "_Cancel"), nullptr};
  const char *label_ok[] = {g_dgettext("gtk30", "_OK"), nullptr};

  switch (type)
  {
  case 1:
    gtk_alert_dialog_set_buttons(dialog, labels_ok_cancel);
    break;
  case 2:
    gtk_alert_dialog_set_buttons(dialog, labels_yes_no);
    break;
  case 3:
    gtk_alert_dialog_set_buttons(dialog, labels_yes_no_cancel);
    break;
  default:
    gtk_alert_dialog_set_buttons(dialog, label_ok);
    break;
  }

  // OK/Yes    button with index 0 is activated if clicked or if enter  key is pressed
  gtk_alert_dialog_set_default_button(dialog, 0);
  // Cancel/No button with index 1 is activated if clicked or if escape key is pressed
  gtk_alert_dialog_set_cancel_button(dialog, 1);

  msg_res ret;
  gtk_alert_dialog_choose(dialog, GTK_WINDOW(parent), NULL, (GAsyncReadyCallback)on_msgbox_click, &ret);
  g_object_unref(dialog);
  while (!ret.done)
    g_main_context_iteration(nullptr, TRUE);
  if (type == 0)
    return "true";
  else
    return ret.response;
}

#else
void OK_callback(GtkWidget *button, gpointer dialog)
{
  gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
}

void Cancel_callback(GtkWidget *button, gpointer dialog)
{
  gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

void Yes_callback(GtkWidget *button, gpointer dialog)
{
  gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_YES);
}

void No_callback(GtkWidget *button, gpointer dialog)
{
  gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO);
}

// Possible values for type are : MB_OK=0, MB_OKCANCEL=1, MB_YESNO=2, MB_YESNOCANCEL=3
// Return is "yes", "no", "ok", "cancel"
std::string gtk_msgbox(webview_wrapper &w, std::string msg, int type = 0)
{
  logTrace("Type: ", type);
  GtkWindow *parent = GTK_WINDOW(w.window());
  GtkWidget *dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
  gtk_window_set_title(GTK_WINDOW(dialog), w.get_title().c_str());
  logInfo("GTK < 4.10, TITLE: " + w.get_title());
  GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *label = gtk_label_new(msg.c_str());
  gtk_container_add(GTK_CONTAINER(content_area), label);
  gtk_widget_show(label);

  GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  GtkWidget *button1; // Button 1 might be Yes, OK, Accept
  GtkWidget *button2; // Button2 might be No, Cancel
  GtkWidget *button3; // Button2 might only be Cancel

  switch (type)
  {
  case 1: // OKCancel
    button1 = gtk_button_new_with_mnemonic(g_dgettext("gtk30", "_OK"));
    gtk_container_add(GTK_CONTAINER(button_box), button1);
    g_signal_connect(button1, "clicked", G_CALLBACK(OK_callback), dialog);
    gtk_widget_show(button1);

    button2 = gtk_button_new_with_mnemonic(g_dgettext("gtk30", "_Cancel"));
    gtk_container_add(GTK_CONTAINER(button_box), button2);
    gtk_widget_show(button2);
    g_signal_connect(button2, "clicked", G_CALLBACK(Cancel_callback), dialog);
    break;
  case 2: // YesNo
    button1 = gtk_button_new_with_mnemonic(g_dgettext("gtk30", "_Yes"));
    gtk_container_add(GTK_CONTAINER(button_box), button1);
    g_signal_connect(button1, "clicked", G_CALLBACK(Yes_callback), dialog);
    gtk_widget_show(button1);

    button2 = gtk_button_new_with_mnemonic(g_dgettext("gtk30", "_No"));
    gtk_container_add(GTK_CONTAINER(button_box), button2);
    gtk_widget_show(button2);
    g_signal_connect(button2, "clicked", G_CALLBACK(No_callback), dialog);
    break;
  case 3: // YesNoCancel
    button1 = gtk_button_new_with_mnemonic(g_dgettext("gtk30", "_Yes"));
    gtk_container_add(GTK_CONTAINER(button_box), button1);
    g_signal_connect(button1, "clicked", G_CALLBACK(Yes_callback), dialog);
    gtk_widget_show(button1);

    button2 = gtk_button_new_with_mnemonic(g_dgettext("gtk30", "_No"));
    gtk_container_add(GTK_CONTAINER(button_box), button2);
    gtk_widget_show(button2);
    g_signal_connect(button2, "clicked", G_CALLBACK(No_callback), dialog);

    button3 = gtk_button_new_with_mnemonic(g_dgettext("gtk30", "_Cancel"));
    gtk_container_add(GTK_CONTAINER(button_box), button3);
    gtk_widget_show(button3);
    g_signal_connect(button3, "clicked", G_CALLBACK(Cancel_callback), dialog);
    break;
  default: // OK
    button1 = gtk_button_new_with_mnemonic(g_dgettext("gtk30", "_OK"));
    gtk_container_add(GTK_CONTAINER(button_box), button1);
    g_signal_connect(button1, "clicked", G_CALLBACK(OK_callback), dialog);
    gtk_widget_show(button1);
    break;
  }

  gtk_widget_set_hexpand(button_box, GTK_ALIGN_CENTER);
  gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), button_box, TRUE, FALSE, 0);

  gtk_widget_show(content_area);
  gtk_widget_show(button_box);
  // gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

  std::string ret;
  int result = gtk_dialog_run(GTK_DIALOG(dialog));
  switch (result)
  {
  case GTK_RESPONSE_OK:
    ret = "ok";
    break;
  case GTK_RESPONSE_YES:
    ret = "yes";
    break;
  case GTK_RESPONSE_ACCEPT:
    ret = "accept";
    break;
  case GTK_RESPONSE_CANCEL:
    ret = "cancel";
    break;
  case GTK_RESPONSE_NO:
    ret = "no";
    break;
  default:
    ret = "ok";
    break;
  }

  gtk_widget_destroy(dialog);

  return ret;
}

#endif
#endif

void create_gtk_binds(webview_wrapper &w)
{
#ifdef __linux__
  w.bind_doc(                                    //
      "gui_msgbox",                              //
      [&](const std::string &req) -> std::string //
      {                                          //
        std::string msg, sbut;
        js_params(req, msg, sbut);
        int but = 0;
        if (!sbut.empty())
          but = std::stoi(sbut);
        return w.json_escape(gtk_msgbox(w, htent_to_path(msg).string(), but));
      },                                                                                                                                                                                                                                           //
      "display a message dialog in a 'close to Windows MessageBox style', First parameter is the message and second one indicate whether we need only an 'OK' button (0), or an 'OK' and a 'Cancel' button (1) or a 'Yes' and a 'No' button (2).", //
      -2);

  std::string filterDesc = ". A filter is made of one or more descriptions, separated by pipes (|). A description is made of one or more patterns separated by commas, optionally terminated by the name, separated by a semicolon (;). When needed, "
                           "separators can be escaped by a backslash (&bsol;). Example of a legit filter string : '*.c,*.cpp,*.c++,*.cc;C/C++ files|*.html,*.js,*.css;Web files (HTML, Javascript and CSS)'";
  w.bind_doc(        //
      "gui_opendlg", //
      [&](const std::string &req) -> std::string { return pathdlg(w, req, file_open); },
      "display a file open dialog with the eventually provided filter, initial directory and title and return immediately" + filterDesc, //
      -1);

  w.bind_doc(        //
      "gui_savedlg", //
      [&](const std::string &req) -> std::string { return pathdlg(w, req, file_save); },
      "display a file save dialog with the eventually provided filter, initial directory and title and return immediately" + filterDesc, //
      -1);

  w.bind_doc(          //
      "gui_folderdlg", //
      [&](const std::string &req) -> std::string { return pathdlg(w, req, folder_sel); },
      "display a folder selection dialog with the eventually provided filter, initial directory and title and return immediately" + filterDesc, //
      -1);
#endif
}
