
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

#ifdef _WIN32
#include "reg.h"
#include "sendkeys.h"
#include "winapi.h"
#include <commctrl.h>
#include <commdlg.h>
#include <oleacc.h>
#include <shobjidl.h>
#pragma comment(lib, "Oleacc.lib")
#include <WebView2.h>
#endif

// clang-format off
#include "util.h"
#include "log.h"
#include "opts.h"
#include "wrap.h"
#include "path_entity.h"
// clang-format on

#ifdef _WIN32
extern ICoreWebView2 *curr_m_webView;

enum pathdlg_type
{
  file_open = 0,
  file_save = 1,
  folder_sel = 2
};

// Replace the character f by the character r in a std::string if f is not preceded by a '\'
// Else the preceding '\' is removed and f is left unchanged.
// Return the number of replacement done
int StringChrRep(std::string &s, char f, char r)
{
  size_t i = 0, i2 = 0, l = s.size();
  int nb = 0;

  for (; i < l; i++)
  {
    if (s[i] == f)
    {
      if (i > 0 && s[i - 1] == '\\')
      {
        s[--i2] = f;
      }
      else
      {
        s[i2] = r;
        nb++;
      }
    }
    else
      s[i2] = s[i];
    i2++;
  }
  while (i2 < i)
    s[i2++] = '\0';

  return nb;
}

void IFOD_build_filter(IFileOpenDialog *pFileOpen, std::string s)
{
  // int nFilt = StringChrRep(s, '|', '#');
  auto vFilt = split(s, '#');

  COMDLG_FILTERSPEC *filters = new COMDLG_FILTERSPEC[vFilt.size() / 2];

  for (size_t i = 0; i < vFilt.size(); i++)
  {
    if (i % 2 == 0)
    {
      if (vFilt[i].size() > 0)
      {
        // std::cout <<"Filter name" << i/2 << ": " << vFilt[i] << ", " << std::flush;
        filters[i / 2].pszName = _wcsdup(s2ws(vFilt[i]).c_str());
      }
      else
        filters[i / 2].pszName = nullptr;
    }
    else
    {
      if (vFilt[i].size() > 0)
      {
        // std::cout <<"Filter spec" << i/2 << ": " << vFilt[i];
        filters[i / 2].pszSpec = _wcsdup(s2ws(vFilt[i]).c_str());
      }
      else
        filters[i / 2].pszSpec = nullptr;
      std::cout << std::endl << std::flush;
    }
  }

  if (pFileOpen->SetFileTypes(UINT(vFilt.size() / 2), filters) >= 0)
    pFileOpen->SetFileTypeIndex(1);
  else
    std::cout << "Filter not set" << std::endl;

  for (size_t i = 0; i < vFilt.size() / 2; i++)
  {
    if (filters[i].pszName)
      std::wcout << "Filter name: " << filters[i].pszName;
    if (filters[i].pszSpec)
      std::wcout << " and spec: " << filters[i].pszSpec;
    std::wcout << std::endl;
  }
  std::wcout << std::endl;
  std::cout << std::flush;
}

std::string FolderDialog(HWND hDlg = NULL, std::string title = {}, std::string filter = {}, std::string initialDir = {})
{
  // Utiliser IFileDialog avec option 'FOS_PICKFOLDERS' ==> shobjidl_core.h
  // ==> https://learn.microsoft.com/fr-fr/windows/win32/learnwin32/example--the-open-dialog-box
  // ==> https://learn.microsoft.com/fr-fr/windows/win32/shell/common-file-dialog
  std::string path = "";

  if (CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE) >= 0)
  {
    IFileOpenDialog *pFileOpen;

    if (CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void **>(&pFileOpen)) >= 0)
    {
      if (title.empty())
        title = "Select a folder";
      pFileOpen->SetTitle(s2ws(title).c_str());
      pFileOpen->SetOptions(FOS_PICKFOLDERS);

      // if (!filter.empty()) IFOD_build_filter(pFileOpen, filter);

      if (!initialDir.empty())
      {
        IShellItem *pCurFolder = NULL;

        if (std::filesystem::path(initialDir).is_relative())
          initialDir = std::filesystem::absolute(initialDir).string();

        if (SHCreateItemFromParsingName(s2ws(initialDir).c_str(), NULL, IID_PPV_ARGS(&pCurFolder)) >= 0)
        {
          pFileOpen->SetFolder(pCurFolder);
          pCurFolder->Release();
        }
      }

      // Show the Open dialog box. Get the file name from the dialog box.
      if (pFileOpen->Show(hDlg) >= 0)
      {
        IShellItem *pItem;

        if (pFileOpen->GetResult(&pItem) >= 0)
        {
          PWSTR pszFilePath;

          // Get the file name.
          if (pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath) >= 0)
          {
            path = ws2s(pszFilePath);
            CoTaskMemFree(pszFilePath);
          }
          pItem->Release();
        }
      }
      pFileOpen->Release();
    }
    CoUninitialize();
  }

  return path;
}

// From a 'pathdlg filter'    : "*.c,*.c++;C/C++ files|*.html,*.js,*.css;Web files (HTML, Javascript and CSS)"
// To a 'OPENFILENAME filter' : "C/C++ files\0*.c;*.c++\0Web files (HTML, Javascript and CSS)\0*.html;*.js;*.css\0\0"
char *OFN_build_filter(std::string src, size_t &lFilter)
{
  trim(src);
  // logTrace("src: ", src);
  if (src.size() < 1)
    return nullptr;

  std::string sdst = {};
  auto vFilt = split_esc(src, '|', '\\');
  for (size_t i = 0; i < vFilt.size(); i++)
  {
    auto vDesc = split_esc(vFilt[i], ';', '\\');
    if (vDesc.size() > 0)
    {
      if (vDesc.size() > 1 && vDesc[1].size() > 0)
        sdst += vDesc[1];
      else
        sdst += "Filter for " + vDesc[0]; // Default name

      sdst.push_back('\0');

      // vDesc[0] ==> vPatt
      auto vPatt = split_esc(vDesc[0], ',', '\\');
      if (vPatt.size() > 0)
      {
        for (size_t j = 0; j < vPatt.size(); j++)
        {
          sdst += vPatt[j];
          if (j < vPatt.size() - 1)
            sdst += ';';
        }
      }
      else
      {
        sdst += "*";
      }

      sdst.push_back('\0');
    }
  }

  sdst.push_back('\0');

  lFilter = sdst.size();
  char *pdst = new char[lFilter + 1];
  //  logTrace("IN OFN_build_filter:");
  for (size_t i = 0; i < lFilter; i++)
  {
    pdst[i] = sdst[i];
    //    std::cout << (int)sdst[i] << " ";
  }
  //  std::cout << std::endl;

  return pdst;
}

std::string FileDialog(HWND hDlg = NULL, pathdlg_type ptyp = file_open, std::string title = {}, std::string filter = {}, std::string initialDir = {})
{

  OPENFILENAME ofn;
  char fname[MAX_PATH] = "";

  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hDlg;

  size_t lf;
  ofn.lpstrFilter = OFN_build_filter(filter, lf);
  /*
    std::cout << "Source filter :" << filter << std::endl;
    std::cout << "Give dest filter :";
    for (size_t i=0; i < lf; i++) {
      if (ofn.lpstrFilter[i] == '\0') std::cout << " 0 ";
      else std::cout << ofn.lpstrFilter[i];
    }
    std::cout << std::endl;
  */

  if (ofn.lpstrFilter != nullptr)
    ofn.nFilterIndex = 1;

  ofn.lpstrFile = fname;
  ofn.nMaxFile = MAX_PATH;

  ofn.lpstrFileTitle = NULL;
  if (!initialDir.empty() && initialDir != ".")
    ofn.lpstrInitialDir = initialDir.c_str();
  ofn.lpstrTitle = title.c_str();
  ofn.Flags = OFN_SHOWHELP;

  if (ptyp == file_open)
  {
    ofn.Flags |= OFN_FILEMUSTEXIST;
    GetOpenFileName(&ofn);
  }
  else
  {
    ofn.Flags |= OFN_OVERWRITEPROMPT;
    GetSaveFileName(&ofn);
  }

  // logTrace("FileDialog: ", fname);
  return fname;
}

std::string pathdlg(webview_wrapper &w, const std::string &req, pathdlg_type ptyp = file_open)
{
  std::string initialDir, title, ret;
  ret.clear();

  if (ptyp == folder_sel)
  {
    js_params(req, initialDir, title);
    // logTrace("pathdlg (folder), initialDir: ", initialDir, ", title: ", title);
    ret = FolderDialog((HWND)(w.window()), title, {}, initialDir);
  }
  else
  {
    std::string filter;
    js_params(req, filter, initialDir, title);
    // logTrace("pathdlg (file), filter: ", filter, ", initialDir: ", initialDir, ", title: ", title);
    ret = FileDialog((HWND)(w.window()), ptyp, title, filter, initialDir);
  }

  // logTrace("pathdlg: ", ret);

  return w.json_escape(ret);
}

void pathdlg_thread(webview_wrapper &w, const std::string &seq, const std::string &req, pathdlg_type ptyp)
{
  std::thread([&w, seq, req, ptyp] { w.resolve(seq, 0, pathdlg(w, req, ptyp)); }).detach();
}

std::string gui_msgbox(webview_wrapper &w, const std::string &req)
{
  std::string msg, sbut;
  js_params(req, msg, sbut);
  int but = 0, wbut;
  if (!sbut.empty())
    but = std::stoi(sbut);

  switch (but)
  {
  case 1:
    wbut = MB_OKCANCEL | MB_ICONQUESTION;
    break;
  case 2:
    wbut = MB_YESNO | MB_ICONQUESTION;
    break;
  default:
    wbut = MB_OK | MB_ICONINFORMATION;
    break;
  }

  int ret = MessageBoxW((HWND)(w.window()), (htent_to_path(msg).wstring()).c_str(), w.get_title_w().c_str(), wbut);

  switch (ret)
  {
  case IDOK:
  case IDYES:
    return "true";
    break;
  default:
    return "false";
    break;
  }
}

std::string win_dark_bar(webview_wrapper &w, const std::string &req)
{
  std::string s_dark_mode;
  js_params(req, s_dark_mode);
  w.set_dark_bar((s_dark_mode == "true"));
  return s_dark_mode;
}
#endif

void send_ctrl_p()
{
#ifdef WIN32
  SendKeyDown(VK_CONTROL);
  SendKeyDown('P');
  SendKeyUp('P');
  SendKeyUp(VK_CONTROL);
#endif
}

void create_win_binds(webview_wrapper &w)
{
#ifdef _WIN32
  w.bind_doc(       //
      "gui_msgbox", //
      [&](const std::string &req) -> std::string { return gui_msgbox(w, req); },
      "display a message dialog in a 'close to Windows MessageBox style', First parameter is the message and second one indicate whether we need only an 'OK' button (0), or an 'OK' and a 'Cancel' button (1) or a 'Yes' and a 'No' button (2).", //
      -2);

  std::string filterDesc = ". The filter consist in pairs of strings with the first of each providing the name and the second providing the pattern, eventual multiple pattern are separated by a comma (,) and eventual multiple pairs are separated by a "
                           "pipe (|). If needed ',' & '|' might be neutralized by '&bsol;' (a doubled one in javascript).";
  // ASYNC
  //	w.bind_doc(        //
  //	    "gui_opendlg", //
  //	    [&](const std::string &seq, const std::string &req, void *) { pathdlg_thread(w, seq, req, file_open); },
  //	    "display a Windows file selection dialog with the eventually provided filter, initial directory and title" + filterDesc, //
  //	    -3);

  w.bind_doc(        //
      "gui_opendlg", //
      [&](const std::string &req) -> std::string { return pathdlg(w, req, file_open); },
      "display a Windows file selection dialog with the eventually provided filter, initial directory and title" + filterDesc, //
      -3);

  w.bind_doc(        //
      "gui_savedlg", //
      [&](const std::string &req) -> std::string { return pathdlg(w, req, file_save); },
      "display a Windows file selection dialog with the eventually provided filter, initial directory and title" + filterDesc, //
      -3);

  w.bind_doc(          //
      "gui_folderdlg", //
      [&](const std::string &req) -> std::string { return pathdlg(w, req, folder_sel); },
      "display a Windows folder selection dialog with the eventually provided initial directory and title (Windows does not allow filter with folder selection).", //
      -3);

  w.bind_doc(         //
      "win_dark_bar", //
      [&](const std::string &req) -> std::string { return win_dark_bar(w, req); },
      "Set (true) on unset (false) the dark mode for the window bar. It is the responsability of web application to manage a dark mode for the DOM.", //
      -1);

  w.bind_doc(         //
      "win_printdlg", //
      [&](const std::string &req) -> std::string {
        // Send the Control+Shitf+P keystroke to make the print dialog appears
        send_ctrl_p();
        return "";
      },
      "Open a print or preprint dialog (in this case parameter to true or 'preprint'.)", //
      0);

  // Windows registry storage and retrieval
  w.bind_doc(                                                     //
      "win_regsto",                                               //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::string key, var, val;
          js_params(req, key, var, val);
          PutRegString(key, var, val);
          w.resolve(seq, 0, '"' + val + '"');
        }).detach();
      },
      "store a string to the Windows registry.", //
      -3);

  w.bind_doc(                                                     //
      "win_regget",                                               //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::string key, var, defval;
          js_params(req, key, var, defval);
          auto val = GetRegString(key, var, defval);
          w.resolve(seq, 0, '"' + val + '"');
        }).detach();
      },
      "retrieve a string from the Windows registry.", //
      -3);
#endif
}
