
#ifndef WINAPI_PP_H
#define WINAPI_PP_H

#include <string>
#include <windows.h>

#ifdef _WIN32
std::string SimpleError(DWORD);
std::string SimpleError();
std::string TxtError(DWORD);
std::string TxtError();
std::string StrError(const char *fmt, ...);
std::string StrError(std::string);
void WinError(const char *fmt, ...);
void WinError(std::string);
#ifdef UNICODE
#define CommandLineToArgv CommandLineToArgvW
#else
PCHAR *CommandLineToArgvA(PCHAR CmdLine, int *_argc);
#define CommandLineToArgv CommandLineToArgvA
#endif
#endif

int MessageBox(HWND hWnd, LPCWSTR text, LPCWSTR caption, UINT uType = MB_OK);
int MessageBox(LPCWSTR text, LPCWSTR caption, UINT uType = MB_OK);
int MessageBox(HWND hWnd, const std::string text, const std::string caption, UINT uType = MB_OK);
int MessageBox(const std::string text, const std::string caption, UINT uType = MB_OK);
int MessageBox(HWND hWnd, const std::wstring text, const std::wstring caption, UINT uType = MB_OK);
int MessageBox(const std::wstring text, const std::wstring caption, UINT uType = MB_OK);

std::string ExpandEnvironmentStrings(const std::string src);
char *ExpandEnvironmentStrings(const char *src);

std::string GetCurrentDirectory();

std::string GetDlgItemText(HWND hDlg, int nIDDlgItem);
std::string GetWindowText(HWND hDlg);
bool SetWindowText(HWND hWnd, std::string text);
bool SetDlgItemText(HWND hDlg, int nIDDlgItem, std::string text);
// Return true if app is running in gui mode only else false if the app is also running with a console.
bool gui_only();
#endif /* WINAPI_PP_H */
