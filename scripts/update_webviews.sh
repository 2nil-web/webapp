#!/bin/bash

function BoldEcho () {
  echo
  echo "$(tput smso)$*$(tput rmso)"
}

# Updating webview
WVDir=webview
WVRepo=https://github.com/${WVDir}/${WVDir}.git
actual_wv_comit=""

BoldEcho "Checking your local version of $WVDir."

if [ -d ${WVDir} ]
then
  cd $WVDir
  actual_wv_comit=$(git rev-list --abbrev-commit HEAD | head -1)
  git pull >/dev/null
else
  BoldEcho "Downloading latest version of ${WVDir}"
  git clone $WVRepo
  cd $WVDir
fi

new_wv_comit=$(git rev-list --abbrev-commit HEAD | head -1)
cd - >/dev/null

if [ "$actual_wv_comit" != "$new_wv_comit" ]
then
  echo "$(tput smul)>>> webview github repo ($WVRepo) has a new commit $new_wv_comit versus the old one which is $actual_wv_comit <<<$(tput rmul)"
else
  echo "$(tput sitm)Your webview local repo is up to date with commit $actual_wv_comit$(tput ritm)"
fi

cat << EOF
$(tput smul)As a reminder for commit c42767e on Tue Apr 1 20:03:18 2025 +0200$(tput rmul)
$(tput sitm)echo "\$(git rev-parse --short HEAD) on \$(git show \$(git rev-parse HEAD) | sed -n 's/^Date: *//p')"$(tput ritm)
$(tput smul)I. For Windows, on file webview/core/include/webview/detail/backends/win32_edge.hh, changes are:$(tput rmul)
  a) From line 313 to line 322:
public:
  win32_edge_engine(bool debug, void *window) : engine_base{!window} {
    window_init(window);
    window_settings(debug);
#ifndef MY_WEBVIEW_EXT    
    dispatch_size_default();
#endif
  }

  virtual ~win32_edge_engine() {

  b) From line 484 to 490:
      if (!m_is_window_shown) {
        set_default_size_guard(false);
#ifndef MY_WEBVIEW_EXT    
        dispatch_size_default();
#endif
      }
    }
  
  c) From line 708 to 723:
    RegisterClassExW(&message_wc);
    CreateWindowExW(0, L"webview_message", nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE,
                    nullptr, hInstance, this);
    if (!m_message_window) {
      throw exception{WEBVIEW_ERROR_INVALID_STATE, "Message window is null"};
    }
#ifdef MY_WEBVIEW_EXT
#ifndef __GNUC__
      extern
#endif
      void WindowInitialState(HWND hw);
      WindowInitialState(m_window);
#endif    
  }

  void window_settings(bool debug) {

  d) From line 729 to 739:
  noresult window_show() {
    if (owns_window() && !m_is_window_shown) {
#ifndef MY_WEBVIEW_EXT
      ShowWindow(m_window, SW_SHOW);
      UpdateWindow(m_window);
      SetFocus(m_window);
#endif
      m_is_window_shown = true;
    }
    return {};
  }
  
  e) At line 750 "CSIDL_APPDATA" is replaced by "CSIDL_LOCAL_APPDATA"
  $(tput sitm)And to strictly follow MicroSoft recommandations, it may even be reported that the function SHGetFolderPath should be replaced by SHGetKnownFolderPath...$(tput ritm)

  f) From line 812 to 826
     m_controller->put_IsVisible(TRUE);
    ShowWindow(m_widget, SW_SHOW);
    UpdateWindow(m_widget);
    if (owns_window()) {
      focus_webview();
    }
#ifdef MY_WEBVIEW_EXT
#ifndef __GNUC__
      extern
#endif
    bool my_configure(ICoreWebView2 * mwv, ICoreWebView2Controller * mctl);
    my_configure(m_webview, m_controller);
#endif
    return {};
  }

  
$(tput smul)
II. For Linux, on file webview/core/include/webview/detail/backends/gtk_webkitgtk.hh, changes are:$(tput rmul)
  a) From line 101 to line 108:
public:
  gtk_webkit_engine(bool debug, void *window) : engine_base{!window} {
    window_init(window);
    window_settings(debug);
#ifndef MY_WEBVIEW_EXT
    dispatch_size_default();
#endif
  }

  b) From 325 to 342
    gtk_compat::window_set_child(GTK_WINDOW(m_window), GTK_WIDGET(m_webview));
    gtk_compat::widget_set_visible(GTK_WIDGET(m_webview), true);

    if (owns_window()) {
#ifdef MY_WEBVIEW_EXT
      extern void WindowInitialState(GtkWidget *m_window, GtkWidget *m_webview);
      WindowInitialState(m_window, m_webview);
#else
      gtk_widget_grab_focus(GTK_WIDGET(m_webview));
      gtk_compat::widget_set_visible(GTK_WIDGET(m_window), true);
#endif
    }

    m_is_window_shown = true;
    return {};
  }

  void run_event_loop_while(std::function<bool()> fn) override {

EOF

if [ -n "$OS" ] && [ "$OS" = "Windows_NT" ]
then
  # Now updating MS WebView2
  WV2RootDir=${WVDir}/build/external/libs
  rm -rf ${WV2RootDir}/*

  BoldEcho "Checking your local version of Webview2."
  #BoldEcho "Downloading latest version of nuget"
  curl -ksSLO https://dist.nuget.org/win-x86-commandline/latest/nuget.exe >/dev/null

  #BoldEcho "Downloading latest version of Webview2"
  #./nuget.exe install Microsoft.Web.Webview2 -Version 1.0.3065.39 -OutputDirectory ${WV2RootDir} >/dev/null
  ./nuget.exe install Microsoft.Web.Webview2 -OutputDirectory ${WV2RootDir} >/dev/null

  NextWebView2=$(cd ${WV2RootDir} && \ls -1d Microsoft.Web.WebView2.* | sort | tail -1)
  PrevWebView2=$(sed -n 's/.*\(Microsoft\.Web\.WebView2\..*\)\\build.*/\1/p' webapp.vcxproj | head -1)


  if [ ${PrevWebView2} = ${NextWebView2} ]
  then
    BoldEcho "No need to update ${PrevWebView2} for MSBuild and Makefile."
  else
    BoldEcho "Updating from ${PrevWebView2} to ${NextWebView2} for MSBuild and Makefile."
    sed -i "s/${PrevWebView2}/${NextWebView2}/" webapp.vcxproj
    # Update for Makefile
    sed -i "s/^WV2SUBDIR=.*/WV2SUBDIR=${NextWebView2}/" header.mk
  fi

  PrevWebView2=$(sed -n 's/.*\(Microsoft\.Web\.WebView2\..*\)\\build.*/\1/p' webapp-console.vcxproj | head -1)

  if [ ${PrevWebView2} != ${NextWebView2} ]
  then
    sed -i "s/${PrevWebView2}/${NextWebView2}/" webapp-console.vcxproj
  fi
fi

