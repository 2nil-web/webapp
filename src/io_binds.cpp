
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ostream>
#include <regex>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>

// clang-format off
#ifdef _WIN32
#include "winapi.h"
#endif
#include "util.h"
#include "log.h"
#include "options.h"
#include "wrap.h"
#include "io_binds.h"
// clang-format on

#ifdef _WIN32
std::streambuf *CinBuffer, *CoutBuffer, *CerrBuffer;
std::fstream ConsoleInput, ConsoleOutput, ConsoleError;

void RedirectIO()
{
  CinBuffer = std::cin.rdbuf();
  CoutBuffer = std::cout.rdbuf();
  CerrBuffer = std::cerr.rdbuf();
  ConsoleInput.open("CONIN$", std::ios::in);
  ConsoleOutput.open("CONOUT$", std::ios::out);
  ConsoleError.open("CONOUT$", std::ios::out);
  std::cin.rdbuf(ConsoleInput.rdbuf());
  std::cout.rdbuf(ConsoleOutput.rdbuf());
  std::cerr.rdbuf(ConsoleError.rdbuf());
}

void ResetIO()
{
  ConsoleInput.close();
  ConsoleOutput.close();
  ConsoleError.close();
  std::cin.rdbuf(CinBuffer);
  std::cout.rdbuf(CoutBuffer);
  std::cerr.rdbuf(CerrBuffer);
  CinBuffer = NULL;
  CoutBuffer = NULL;
  CerrBuffer = NULL;
}

std::wstreambuf *wCinBuffer, *wCoutBuffer, *wCerrBuffer;
std::wfstream wConsoleInput, wConsoleOutput, wConsoleError;

void RedirectWIO()
{
  wCinBuffer = std::wcin.rdbuf();
  wCoutBuffer = std::wcout.rdbuf();
  wCerrBuffer = std::wcerr.rdbuf();
  wConsoleInput.open("CONIN$", std::ios::in);
  wConsoleOutput.open("CONOUT$", std::ios::out);
  wConsoleError.open("CONOUT$", std::ios::out);
  std::wcin.rdbuf(wConsoleInput.rdbuf());
  std::wcout.rdbuf(wConsoleOutput.rdbuf());
  std::wcerr.rdbuf(wConsoleError.rdbuf());
}

void ResetWIO()
{
  wConsoleInput.close();
  wConsoleOutput.close();
  wConsoleError.close();
  std::wcin.rdbuf(wCinBuffer);
  std::wcout.rdbuf(wCoutBuffer);
  std::wcerr.rdbuf(wCerrBuffer);
  wCinBuffer = NULL;
  wCoutBuffer = NULL;
  wCerrBuffer = NULL;
}

bool MadeConsole = false;
bool already_opened = false;
#endif
// Return true if had to Attach and Alloc console else false (Console already Attached and Allocated)
//
bool OpenConsole()
{
#ifdef _WIN32
  if (already_opened)
    return true;

  if (!AttachConsole(ATTACH_PARENT_PROCESS))
  {
    if (!AllocConsole())
    {
      return false;
    }
    MadeConsole = true;
  }

  if (MadeConsole)
  {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (GetConsoleMode(hOut, &dwMode))
    {
      dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, dwMode);
    }

    RedirectWIO();
    // already_opened=true;
  }
  else
  {
    RedirectIO();
  }

  return MadeConsole;
#else
  return true;
#endif
}

void CloseConsole()
{
#ifdef _WIN32
  if (MadeConsole)
  {
    ResetWIO();
  }
  else
  {
    ResetIO();
  }

  if (!already_opened)
    FreeConsole();
#endif
}

void output(std::ostream &out, std::string s, bool addcr)
{
  out << s;
  if (addcr)
    out << std::endl;
  out.flush();
}

void woutput(std::wostream &wout, std::wstring ws, bool addcr)
{
  wout << ws;
  if (addcr)
    wout << std::endl;
  wout.flush();
}

std::string input(std::istream &in, const bool line)
{
  std::string s;
  if (line)
    std::getline(in, s); //, '#');
  else
    in >> s;
  return s;
}

void my_getwline(std::wistream &win, std::wstring &ws)
{
  wchar_t wc[2];
  ws.clear();
  while (win.get(wc, 2))
  {
    ws += wc[0];
  }
}

std::wstring winput(std::wistream &in, const bool line)
{
  std::cout << "Winput" << std::endl << std::flush;
  std::wstring ws;
  if (line)
    my_getwline(in, ws);
  else
    std::wcin >> ws;

  return ws;
}

void write_to_console(webview_wrapper &w, const std::string &seq, const std::string &req, const bool addcr = false, std::ostream &out = std::cout) //
{
  std::thread([&, seq, req, addcr] {
    std::filesystem::path s = json_parse(req, "", 0);

    if (!s.empty())
    {
      if (OpenConsole())
      {
        woutput(std::wcout, s.wstring(), addcr);
      }
      else
      {
        output(std::cout, s.string(), addcr);
      }
    }

    CloseConsole();
    w.resolve(seq, 0, "");
  }).detach();
}

// Test with: io_write("Input a string: ");; outln(io_readln())
void read_from_console(webview_wrapper &w, const std::string &seq, const std::string &req, const bool line = false) //
{
  std::thread([&, seq, req, line] {
    std::filesystem::path s{};
    if (OpenConsole())
    {
      s = winput(std::wcin, line);
    }
    else
    {
      s = input(std::cin, line);
    }

    CloseConsole();
    w.resolve(seq, 0, w.json_escape(s.string()));
  }).detach();
}

// Retourne true si l'appli a une console déjà attachée (msys2; cygwin, windows terminal)
// Sinon false
void has_console(webview_wrapper &w, const std::string &seq, const std::string &req, const bool line = false) //
{
  std::thread([&, seq, req, line] {
    std::string res = "false";
#ifdef _WIN32
    HWND hc = GetConsoleWindow();
    res = (hc == nullptr) ? "false" : "true";
#endif
    w.resolve(seq, 0, w.json_escape(res));
  }).detach();
}

//
void interp_console(webview_wrapper &w, const std::string &seq, const std::string &req, const bool line = false) //
{
  std::thread([&, seq, req, line] {
    std::string res = "false";
    //    extern bool quiet;
    //    extern void getVersion(char = '\0', std::string = "", std::string = "");
    //    if (!quiet) getVersion();

    std::string ln, prompt = "> ";
    std::cout << prompt << std::flush;

    while (std::getline(std::cin, ln))
    {
      trim(ln);
      if (ln == "quit" || ln == "exit")
        break;
      std::cout << '[' << ln << ']' << std::endl;
      if (!ln.empty())
        w.eval(ln);
      std::cout << prompt << std::flush;
    }

    w.resolve(seq, 0, w.json_escape(res));
  }).detach();
}

void create_io_binds(webview_wrapper &w)
{
  w.bind_doc(                                                                                         //
      "io_write",                                                                                     //
      [&](const std::string &seq, const std::string &req, void *) { write_to_console(w, seq, req); }, //
      "write a string to stdout.",                                                                    //
      1                                                                                               //
  );                                                                                                  //

  w.bind_doc(                                                                                               //
      "io_writeln",                                                                                         //
      [&](const std::string &seq, const std::string &req, void *) { write_to_console(w, seq, req, true); }, //
      "write a string and a carriage return to stdout.",                                                    //
      -1);                                                                                                  //

  w.bind_doc(                                                                                                           //
      "io_ewrite",                                                                                                      //
      [&](const std::string &seq, const std::string &req, void *) { write_to_console(w, seq, req, false, std::cerr); }, //
      "write a string to stderr.",                                                                                      //
      1);                                                                                                               //

  w.bind_doc(                                                                                                          //
      "io_ewriteln",                                                                                                   //
      [&](const std::string &seq, const std::string &req, void *) { write_to_console(w, seq, req, true, std::cerr); }, //
      "write a string and a carriage return to stderr.",                                                               //
      -1);                                                                                                             //
                                                                                                                       //
  w.bind_doc(                                                                                                          //
      "io_read",                                                                                                       //
      [&](const std::string &seq, const std::string &req, void *) { read_from_console(w, seq, req); },                 //
      "read a string from stdin and return it."                                                                        //
  );                                                                                                                   //

  w.bind_doc(                                                                                                //
      "io_readln",                                                                                           //
      [&](const std::string &seq, const std::string &req, void *) { read_from_console(w, seq, req, true); }, //
      "read a line ending with carriage return, from stdin and return it."                                   //
  );                                                                                                         //

  w.bind_doc(                                                                                          //
      "io_has_console",                                                                                //
      [&](const std::string &seq, const std::string &req, void *) { has_console(w, seq, req, true); }, //
      "return true if has console else false."                                                         //
  );                                                                                                   //
                                                                                                       /*
                                                                                                       w.bind_doc(
                                                                                                           "io_interp_console",
                                                                                                           [&](const std::string &seq, const std::string &req, void *) { interp_console(w, seq, req, true); },
                                                                                                           "read comands from the console and run them."
                                                                                                           );              */
}
