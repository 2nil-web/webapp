
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
#ifdef _WIN32
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <oleacc.h>
#include <shobjidl.h>
#pragma comment(lib, "Oleacc.lib")
#include <WebView2.h>
#include "reg.h"
#include "sendkeys.h"
#include "winapi.h"
#endif

#include "util.h"
#include "log.h"
//#include "opts.h"
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

int MessageBoxCenteredW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
  // Center message box at its parent window
  static HHOOK hHookCBT{};
  hHookCBT = SetWindowsHookEx(
      WH_CBT,
      [](int nCode, WPARAM wParam, LPARAM lParam) -> LRESULT {
        if (nCode == HCBT_CREATEWND)
        {
          if (((LPCBT_CREATEWND)lParam)->lpcs->lpszClass == (LPCSTR)(ATOM)32770) // #32770 = dialog box class
          {
            RECT rcParent{};
            GetWindowRect(((LPCBT_CREATEWND)lParam)->lpcs->hwndParent, &rcParent);
            ((LPCBT_CREATEWND)lParam)->lpcs->x = rcParent.left + ((rcParent.right - rcParent.left) - ((LPCBT_CREATEWND)lParam)->lpcs->cx) / 2;
            ((LPCBT_CREATEWND)lParam)->lpcs->y = rcParent.top + ((rcParent.bottom - rcParent.top) - ((LPCBT_CREATEWND)lParam)->lpcs->cy) / 3;
          }
        }

        return CallNextHookEx(hHookCBT, nCode, wParam, lParam);
      },
      0, GetCurrentThreadId());

  int iRet{MessageBoxW(hWnd, lpText, lpCaption, uType)};

  UnhookWindowsHookEx(hHookCBT);

  return iRet;
}
// Possible values for type are : MB_OK=0, MB_OKCANCEL=1, MB_YESNO=2, MB_YESNOCANCEL=3
// According to which dialog button is clicked, return one of the following string: "yes", "no", "ok", "cancel"
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
  case 3:
    wbut = MB_YESNOCANCEL | MB_ICONQUESTION;
    break;
  default:
    wbut = MB_OK | MB_ICONINFORMATION;
    break;
  }

  int ret = MessageBoxCenteredW((HWND)(w.window()), (htent_to_path(msg).wstring()).c_str(), w.get_title_w().c_str(), wbut);

  switch (ret)
  {
  case IDOK:
    return "ok";
  case IDCANCEL:
    return "cancel";
  case IDYES:
    return "yes";
  case IDNO:
    return "no";
  default:
    return "ok";
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

void send_ctrl_p()
{
  SendKeyDown(VK_CONTROL);
  SendKeyDown('P');
  SendKeyUp('P');
  SendKeyUp(VK_CONTROL);
}

std::string esc_bs(std::string s)
{
  return creplace_all(s, "\\", "\\\\");
}

std::string esc_brace(std::string s)
{
  return creplace_all(creplace_all(s, "}", "\\}"), "{", "\\{");
}

struct MONITOR_RESULT
{
  int n;
  std::string str;
};

BOOL CALLBACK MonitorEnumProcJSON(HMONITOR monitor, HDC, LPRECT, LPARAM data)
{
  MONITORINFOEXA mi;
  MONITOR_RESULT *my = (MONITOR_RESULT *)data;

  mi.cbSize = sizeof(MONITORINFOEXA);
  GetMonitorInfoA(monitor, &mi);
  if (!my->str.empty())
    my->str += ",";
  my->str += "\"Monitor" + std::to_string(my->n) + "\":{";
  my->str += std::string("\"device name\":\"") + esc_bs(mi.szDevice) + "\",";
  my->str += "\"area\":[" + std::to_string(mi.rcMonitor.left) + ',' + std::to_string(mi.rcMonitor.top) + ',' + std::to_string(mi.rcMonitor.right) + ',' + std::to_string(mi.rcMonitor.bottom) + "],";
  my->str += "\"working area\":[" + std::to_string(mi.rcWork.left) + ',' + std::to_string(mi.rcWork.top) + ',' + std::to_string(mi.rcWork.right) + ',' + std::to_string(mi.rcWork.bottom) + "],";
  my->str += "\"primary monitor\":";
  if (mi.dwFlags == MONITORINFOF_PRIMARY)
    my->str += "true";
  else
    my->str += "false";
  my->str += '}';
  my->n++;
  return TRUE;
}

std::string GetMonitorsInfoJSON()
{
  MONITOR_RESULT my;
  my.n = 0;
  my.str = {};
  EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProcJSON, (LPARAM)&my);
  return '{' + my.str + '}';
}

bool add_dd(DWORD st, DWORD idd, std::string &s, const std::string sdd)
{
  if (!s.empty())
    s += ',';

  s += '"' + sdd + "\":";
  if (st & idd)
  {
    s += "true";
    return true;
  }
  else
    s += "false";
  return false;
}

std::string state_to_json(DWORD st)
{
  std::string s = {};
  if (add_dd(st, DISPLAY_DEVICE_ACTIVE, s, "active"))
  {
    add_dd(st, DISPLAY_DEVICE_MIRRORING_DRIVER, s, "mirroring driver");
    add_dd(st, DISPLAY_DEVICE_MODESPRUNED, s, "more display modes than its output devices support");
    add_dd(st, DISPLAY_DEVICE_PRIMARY_DEVICE, s, "primary device for desktop");
    add_dd(st, DISPLAY_DEVICE_REMOVABLE, s, "removable so cannot be primary display");
    add_dd(st, DISPLAY_DEVICE_VGA_COMPATIBLE, s, "vga compatible");
    add_dd(st, DISPLAY_DEVICE_ATTACHED_TO_DESKTOP, s, "attached to desktop");
    add_dd(st, DISPLAY_DEVICE_MULTI_DRIVER, s, "multi driver");
#if _WIN32_WINNT >= 0x0602
    add_dd(st, DISPLAY_DEVICE_ACC_DRIVER, s, "acc driver");
#endif
    add_dd(st, DISPLAY_DEVICE_TS_COMPATIBLE, s, "ts compatible");
    add_dd(st, DISPLAY_DEVICE_UNSAFE_MODES_ON, s, "unsafe modes_on");
    add_dd(st, DISPLAY_DEVICE_RDPUDD, s, "rdpudd");
    add_dd(st, DISPLAY_DEVICE_REMOTE, s, "remote");
    add_dd(st, DISPLAY_DEVICE_DISCONNECT, s, "disconnected");
    if (!st & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
      add_dd(st, DISPLAY_DEVICE_ATTACHED, s, "attached");
  }
  return s;
}

// Adapter or Monitor
std::string DumpDevice(const DWORD num, const DISPLAY_DEVICEA dd)
{
  std::string ret = {};
  ret += "\"number\":" + std::to_string(num) + ",\"name\":\"" + esc_bs(dd.DeviceName) + "\",\"model\":\"" + esc_bs(dd.DeviceString) + "\",";
  ret += "\"state\":{" + state_to_json(dd.StateFlags) + "},";
  ret += "\"ID\":\"" + esc_bs(dd.DeviceID) + "\",";
  ret += "\"Key\":\"" + esc_bs(esc_brace(dd.DeviceKey)) + "\"";
  return ret;
}

std::string GetDevicesInfoJSON()
{
  DISPLAY_DEVICEA ad;
  ad.cb = sizeof(DISPLAY_DEVICEA);

  DISPLAY_DEVICEA mn;
  mn.cb = sizeof(DISPLAY_DEVICEA);
  DWORD monitorNum;

  std::string json = {};
  DWORD deviceNum = 1;
  while (EnumDisplayDevicesA(NULL, deviceNum, &ad, 0))
  {
    if (deviceNum > 1)
      json += ',';
    json += "\"Adapter" + std::to_string(deviceNum) + "\":{";
    json += DumpDevice(deviceNum, ad);
    monitorNum = 1;
    while (EnumDisplayDevicesA(ad.DeviceName, monitorNum, &mn, 0))
    {
      if (monitorNum > 0)
        json += ',';
      json += ",\"Monitor" + std::to_string(monitorNum) + "\":{" + DumpDevice(monitorNum, mn) + '}';
      monitorNum++;
    }
    json += '}';
    deviceNum++;
  }

  logDebug(json);
  return '{' + json + '}';
}

#define func_die(msg)                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                            \
    logError(msg);                                                                                                                                                                                                                                             \
    return 0;                                                                                                                                                                                                                                                  \
  }

bool InitWallpaper(IDesktopWallpaper **idw)
{
  *idw = nullptr;
  if (FAILED(CoInitialize(NULL)))
    func_die("CoInitialize");
  if (FAILED(CoCreateInstance(__uuidof(DesktopWallpaper), NULL, CLSCTX_LOCAL_SERVER, __uuidof(IDesktopWallpaper), (void **)idw)))
    func_die("CoCreateInstance");

  return true;
}

void ExitWallpaper(IDesktopWallpaper *idw)
{
  if (idw != nullptr)
    idw->Release();
  CoUninitialize();
}

#define MAX_STR 30000
std::string WallpapersInfoJSON()
{
  IDesktopWallpaper *idw;
  std::string s = {};

  if (InitWallpaper(&idw) && idw != nullptr)
  {
    unsigned int n;

    if (idw->GetMonitorDevicePathCount(&n) == S_OK)
    {
      unsigned int i;
      LPWSTR id[MAX_STR], wp[MAX_STR];
      RECT rc;

      bool first_comma = true;
      std::string s_id = {}, s_file = {}, s_rc = {};
      for (i = 0; i < n; i++)
      {
        if (idw->GetMonitorDevicePathAt(i, id) == S_OK)
        {
          s_id = esc_bs(ws2s(*id));

          if (idw->GetWallpaper(*id, wp) == S_OK)
            s_file = esc_bs(ws2s(*wp));
          else
            s_file = {};

          if (idw->GetMonitorRECT(*id, &rc) == S_OK)
            s_rc = std::to_string(rc.left) + ',' + std::to_string(rc.top) + ',' + std::to_string(rc.right) + ',' + std::to_string(rc.bottom);
          else
            s_rc = {};

          if (!first_comma)
            s += ',';
          else
            first_comma = false;
          s += "\"monitor" + std::to_string(i + 1) + "\":{";
          s += "\"id\":\"" + s_id + "\",";
          s += "\"file\":\"" + s_file + "\",";
          s += "\"rect\":[" + s_rc + ']';
          s += '}';
        }
      }
    }
    ExitWallpaper(idw);
  }
  logDebug(s);
  return '{' + s + '}';
}

bool NextWallpaper(LPWSTR *pMonID = nullptr, DESKTOP_SLIDESHOW_DIRECTION dir = DESKTOP_SLIDESHOW_DIRECTION::DSD_FORWARD)
{
  IDesktopWallpaper *idw;
  if (InitWallpaper(&idw) && idw != nullptr)
  {
    if (FAILED(idw->AdvanceSlideshow((LPCWSTR)pMonID, dir)))
      func_die("AdvanceSlideshow");
    if (pMonID != nullptr)
      delete[] pMonID;
    ExitWallpaper(idw);
    return true;
  }
  return false;
}

#endif

void create_win_binds(webview_wrapper &w)
{
#ifdef _WIN32
  w.bind_doc(       //
      "gui_msgbox", //
      [&](const std::string &req) -> std::string { return w.json_escape(gui_msgbox(w, req)); },
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

  w.bind_doc(
      "win_monitors_info",                    //
      [&](const std::string &) -> std::string //
      { return GetMonitorsInfoJSON(); },
      "Return all monitors information in JSON format." //
  );

  w.bind_doc(
      "win_devices_info",                     //
      [&](const std::string &) -> std::string //
      { return GetDevicesInfoJSON(); },
      "Return all devices information in JSON format." //
  );

  w.bind_doc(
      "win_wallpapers_info",                  //
      [&](const std::string &) -> std::string //
      { return WallpapersInfoJSON(); },
      "Return all wallpapers information in JSON format." //
  );

  w.bind_doc(
      "win_next_wallpaper",                   //
      [&](const std::string &) -> std::string //
      { return NextWallpaper() ? "true" : "false"; },
      "Try to advance to the next wallpaper in the slideshow, and return true if ok, else false." //
  );

#endif
}
