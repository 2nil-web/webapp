
#ifdef __unix__
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif
#elif defined(_WIN32) || defined(WIN32)
// clang-format off
#include <tchar.h>
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <io.h>
// clang-format on
#endif

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <vector>

// clang-format off
#include "wrap.h"

#include "version.h"
#include "util.h"
#include "log.h"
#include "opts.h"
// clang-format on

bool open_console()
{
#ifdef _WIN32
  static bool console_not_opened = true;

  if (console_not_opened)
  {
    SetConsoleOutputCP(CP_UTF8);
    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hOut == INVALID_HANDLE_VALUE)
      return false;
    DWORD dwMode = 0;

    if (!GetConsoleMode(hOut, &dwMode))
      return false;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    console_not_opened = false;
  }
#endif
  return true;
}

bool help_or_version = false;
void plan_exit_success()
{
  help_or_version = true;
}

// Dans une chaine, remplace le caractére précédé par un underscore par un
// caractére souligné en vert si émulation vt possible
std::string parse_vt(std::string &s)
{
  if (!open_console())
    return s;

  std::string ret = "", s2 = "";
  bool close_ul = false;

  for (auto c : s)
  {
    if (c == '_')
    {
      ret += "\033[4m\033[92m";
      close_ul = true;
    }
    else
    {
      ret += c;
      s2 += c;

      if (close_ul)
      {
        ret += "\033[0m";
        close_ul = false;
      }
    }
  }

  s = s2;
  return ret;
}

// name, has_arg, val, help
// has_arg : no_argument (ou 0), si l'option ne prend pas d'argument,
// required_argument (ou 1) si l'option prend un argument, ou optional_argument
// (ou 2) si l'option prend un argument optionnel.
size_t n_opt = 0, longest_opname = 0;
static struct option *long_options = nullptr;
static std::vector<run_opt> my_ropts;
std::string optstr = "";
bool arg_sel = true, interp_on = false, quiet = false, no_quit = false;

size_t index_from_val(char v)
{
  for (size_t i = 0; i < n_opt; i++)
  {
    if (v == my_ropts[i].val)
      return i;
  }

  return n_opt + 1;
}

size_t index_from_name(std::string n)
{
  for (size_t i = 0; i < n_opt; i++)
  {
    if (n == my_ropts[i].name)
      return i;
  }

  return n_opt + 1;
}

std::string name_from_val(char v)
{
  for (size_t i = 0; i < n_opt; i++)
  {
    if (v == my_ropts[i].val)
    {
      return my_ropts[i].name;
    }
  }

  return "";
}

char val_from_name(std::string n)
{
  for (size_t i = 0; i < n_opt; i++)
  {
    if (n == my_ropts[i].name && my_ropts[i].val != '\0')
    {
      return my_ropts[i].val;
    }
  }

  return '\0';
}

// From Freak, see :
// https://stackoverflow.com/questions/152016/detecting-cpu-architecture-compile-time
std::string getBuild()
{
#if defined(__x86_64__) || defined(_M_X64)
  return "x86_64";
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
  return "x86_32";
#elif defined(__ARM_ARCH_2__)
  return "ARM2";
#elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
  return "ARM3";
#elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
  return "ARM4T";
#elif defined(__ARM_ARCH_5_) || defined(__ARM_ARCH_5E_)
  return "ARM5"
#elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2_)
  return "ARM6T2";
#elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
  return "ARM6";
#elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
  return "ARM7";
#elif defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
  return "ARM7A";
#elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
  return "ARM7R";
#elif defined(__ARM_ARCH_7M__)
  return "ARM7M";
#elif defined(__ARM_ARCH_7S__)
  return "ARM7S";
#elif defined(__aarch64__) || defined(_M_ARM64)
  return "ARM64";
#elif defined(mips) || defined(__mips__) || defined(__mips)
  return "MIPS";
#elif defined(__sh__)
  return "SUPERH";
#elif defined(__powerpc) || defined(__powerpc__) || defined(__powerpc64__) || defined(__POWERPC__) || defined(__ppc__) || defined(__PPC__) || defined(_ARCH_PPC)
  return "POWERPC";
#elif defined(__PPC64__) || defined(__ppc64__) || defined(_ARCH_PPC64)
  return "POWERPC64";
#elif defined(__sparc__) || defined(__sparc)
  return "SPARC";
#elif defined(__m68k__)
  return "M68K";
#else
  return "UNKNOWN";
#endif
}

std::string progpath = "";
std::string intro = "";
std::string copyright = "";
std::string usage(std::ostream &out)
{
  if (arg_sel)
    interp_on = false;

  std::ostringstream oss;

  oss << "<b>" << app_info::name << ' ' << app_info::version << '-' << app_info::commit << "</b>";
  if (!app_info::decoration.empty())
    oss << '.' << app_info::decoration;
  oss << std::endl;
  oss << "<i>A tool to create standalone applications based on web technology.</i>" << std::endl;

  if (!interp_on)
    oss << "<u>Usage:</u> " << progpath << " [OPTIONS] ARGUMENT" << std::endl;
  //  oss << intro << std::endl;

  if (interp_on)
    oss << "Available commands and their shortcut, if available." << std::endl;
  else
    oss << "<u><i>Available options</i></u>" << std::endl;

  size_t rion = longest_opname + 4;

  for (auto o : my_ropts)
  {
    std::string uname;
    uname = parse_vt(o.name);

    std::string spc = "";
    spc.append(rion - o.name.size(), ' ');

    if (o.name == "" && o.has_arg == 0 && o.val == 0 && o.func == 0)
    {
      oss << o.help << std::endl;
    }
    else if (interp_on && (o.oi_mode == opt_itr || o.oi_mode == itr_only))
    {
      std::string hlp = o.help;
      if (hlp.size() > 0)
        hlp[0] = tolower(hlp[0]);
      // oss.width(rion); oss << std::left << uname;
      // oss << uname << spc;
      if (isprint(o.val))
        oss << "(" << o.val << ") ";
      else
        oss << "   ";
      oss << hlp << std::endl;
    }
    else if (!interp_on && (o.oi_mode == opt_itr || o.oi_mode == opt_only))
    {
      if (o.val == 0)
        oss << "   ";
      else
        oss << '-' << o.val << " ";
      std::string hlp = o.help;
      if (hlp.size() > 0)
        hlp[0] = toupper(hlp[0]);
      // oss.width(rion); oss << std::left << " --"+uname << hlp << std::endl;
      // oss << " --" + uname + spc << hlp << std::endl;
      oss << hlp << std::endl;
    }
  }

  // Copying oss to a string
  // std::string help_msg;
  // help_msg=oss.str();
  out << std::regex_replace(oss.str(), std::regex("\\<.*?\\>"), "") << std::endl;
  return oss.str();
}

void getUsage(char, std::string, std::string)
{
  //  if (!no_quit) interp_on=false;
  usage();

  if (!interp_on)
  {
    plan_exit_success();
  }
}

bool already_v = false;

#define WVCR ".\n (c) Serge Zaitsev and Steffen André Langnes (MIT License)."

std::string appInfo()
{
  std::string ppath = progpath;
  // if (ppath.size() > 0) ppath[0] = toupper(ppath[0]);

  std::string aInf = ppath + ' ' + app_info::version;
  if (app_info::commit != "")
    aInf += '-' + app_info::commit;
  std::string metadata = "";
  if (app_info::decoration != "")
    metadata += app_info::decoration;
  if (metadata != "")
    aInf += '+' + metadata;
  //  aInf += '.';
  if (copyright.size() > 0)
    aInf += "\n  " + copyright;
  aInf += "\nBased on webview " + webview_wrapper::version() + WVCR;
  aInf += "\nBuild for " + getBuild();
  if (app_info::created_at != "")
    aInf += ", " + app_info::created_at;
  aInf += '.';
  return aInf;
}

std::wstring appInfoW()
{
  std::string ppath = progpath;
  // if (ppath.size() > 0) ppath[0] = toupper(ppath[0]);

  std::wstring aInf = s2ws(ppath) + L' ' + s2ws(app_info::version);
  if (app_info::commit != "")
    aInf += L'-' + s2ws(app_info::commit);
  std::string metadata = "";
  if (app_info::decoration != "")
    metadata += app_info::decoration;
  if (metadata != "")
    aInf += L'+' + s2ws(metadata);
  //  aInf += '.';
  if (copyright.size() > 0)
    aInf += L"\n  " + s2ws(copyright);
  aInf += L"\nBased on webview " + s2ws(webview_wrapper::version()) + s2ws(WVCR);
  aInf += L"\nBuild for " + s2ws(getBuild());
  if (app_info::created_at != "")
    aInf += L", " + s2ws(app_info::created_at);
  aInf += L'.';
  return aInf;
}

void getVersion(char = '\0', std::string = "", std::string = "")
{
  if (!already_v)
    already_v = true;
  std::cout << appInfo() << std::endl;
  if (!interp_on)
    plan_exit_success();
}

void set_options()
{
  n_opt = my_ropts.size();
  long_options = new option[n_opt + 1];
  char v;

  for (size_t i = 0; i < n_opt; i++)
  {
    if (my_ropts[i].name.size() > longest_opname)
      longest_opname = my_ropts[i].name.size();

    if (my_ropts[i].name.size() > 0)
      long_options[i].name = my_ropts[i].name.c_str();
    else
      long_options[i].name = nullptr;

    long_options[i].has_arg = my_ropts[i].has_arg;
    long_options[i].val = my_ropts[i].val;
    long_options[i].flag = nullptr;

    //   std::cout << "set_options: " << long_options[i].name << ", " << long_options[i].has_arg << ", " << long_options[i].flag << ", " << long_options[i].val << ", " << std::endl;

    if (my_ropts[i].val == 0)
      v = val_from_name(my_ropts[i].name);
    else
      v = my_ropts[i].val;

    if (v && optstr.find(v) == std::string::npos)
    {
      optstr += (char)v;

      switch (my_ropts[i].has_arg)
      {
      case required_argument:
        optstr += ':';
        break;
      case optional_argument:
        optstr += "::";
        break;
      default:
        break;
      }
    }
  }

  // std::cout << "optstr : " << optstr << std::endl;
  long_options[n_opt] = {0, 0, 0, 0};
}

std::string get1opts(std::string name, char val, int oi_mode, int has_arg, std::string help, OptFunc func)
{
  std::string ret = {};
  if (!name.empty())
  {
    ret += "name: " + name + " / ";
    ret += "index_from_name(name): " + std::to_string(index_from_name(name)) + '(' + std::to_string(n_opt) + "), ";
  }

  if (val != 0)
  {
    std::string v = std::string(1, val);
    if (!v.empty())
    {
      ret += "(" + v + ") / ";
      ret += "index_from_val(val): " + std::to_string(index_from_val(val)) + '(' + std::to_string(n_opt) + "), ";
    }
  }

  switch (oi_mode)
  {
  case opt_itr:
    break;
  case opt_only:
    ret += "mode: opt_only, ";
    break;
  case itr_only:
    ret += "mode: itr_only ";
    break;
  default:
    break;
  }

  switch (has_arg)
  {
  case no_argument:
    break;
  case required_argument:
    ret += "arg: required_argument ";
    break;
  case optional_argument:
    ret += "arg: optional_argument ";
    break;
  default:
    break;
  }

  if (!help.empty())
    ret += help + ' ';
  if (func)
    ret += "and has a func.";

  return ret;
}

std::string get1opts(run_opt &opt)
{
  return get1opts(opt.name, opt.val, opt.oi_mode, opt.has_arg, opt.help, opt.func);
}
// insert arg with val and name if not already exists, return true if done else false.
bool insert_arg_if_missing(const std::string name, const char val, int oi_mode, int has_a = no_argument, const std::string help = "", OptFunc func = nullptr)
{
  // Do nothing if val or name already exist
  if (index_from_val(val) > n_opt && index_from_name(name) > n_opt)
  {
    my_ropts.insert(my_ropts.begin(), {name, val, oi_mode, has_a, help, func});
    n_opt++;
    return true;
  }

  return false;
}

// Add arg with val and name if not already exists, return true if done else false.
bool add_arg_if_missing(const std::string name, const char val, int oi_mode, int has_a = no_argument, const std::string help = "", OptFunc func = nullptr)
{
  // Do nothing if val or name already exist
  if (index_from_val(val) > n_opt && index_from_name(name) > n_opt)
  {
    my_ropts.push_back({name, val, oi_mode, has_a, help, func});
    n_opt++;
    return true;
  }

  return false;
}

bool interp()
{
  arg_sel = false;
  if (!interp_on)
    return false;

  if (!quiet && !already_v)
  {
    getVersion();
    already_v = false;
  }

  std::string ln, prompt = "> ";
  std::string cmd, param;
  std::string::size_type pos;
  bool found_cmd;

  std::cout << prompt << std::flush;

  no_quit = true;
  while (no_quit && std::getline(std::cin, ln))
  {
    trim(ln);

    // Cas particulier du ! qui n'a pas forcément besoin d'espace après ses
    // paramétres
    if (ln[0] == '!')
    {
      cmd = "!";
      param = ln.substr(1);
      trim(param);
    }
    else
    {
      pos = ln.find_first_of(' ');

      if (pos == std::string::npos)
      {
        cmd = ln;
        param = "";
      }
      else
      {
        cmd = ln.substr(0, pos);
        param = ln.substr(pos);
        trim(param);
      }
    }

    if (cmd.size() > 0 && cmd != "")
    {
      found_cmd = false;
      for (auto myopt : my_ropts)
      {
        if (myopt.oi_mode != opt_only && (myopt.name == cmd || (cmd.size() == 1 && myopt.val == cmd[0])))
        {
          // std::cout << "n [" << myopt.name << "], cmd [" << cmd << ']' << std::endl;
          found_cmd = true;
          if (myopt.func != nullptr)
            myopt.func(myopt.val, myopt.name, param);
        }
      }

      if (!found_cmd && !ln.empty() && ln != "" && any_of_ctype(ln, isgraph))
      {
        std::cout << "Unknown command " << cmd;
        if (param.size() > 0)
          std::cout << ", with parameter(s) " << '[' << param << ']';
        std::cout << std::endl;
      }
    }

    if (no_quit)
      std::cout << prompt << std::flush;
  }

  return true;
}

void getopt_init(int argc, char **argv, std::vector<run_opt> pOptions, const std::string pIntro, const std::string pVersion, const std::string pCopyright)
{
  progpath = std::filesystem::path(argv[0]).stem().string();
  intro = pIntro;
  if (pVersion != "")
    app_info::version = pVersion;
  copyright = pCopyright;
  // std::cout << "1. OPT L: " << pOptions.size() << std::endl;
  auto vo = pOptions.end();
  while (vo != pOptions.begin())
  {
    --vo;
    //     std::cout << get1opts(*vo) << std::endl;
    //    insert_arg_if_missing(vo->name, vo->val, vo->oi_mode, vo->has_arg, vo->help, vo->func);
    my_ropts.insert(my_ropts.begin(), *vo);
    //        {vo->name, vo->val, vo->oi_mode, vo->has_arg, vo->help, vo->func});
    // n_opt++;
  }
  // for (auto vo : pOptions)
  //  {
  // my_ropts.push_back(vo);
  // Remplacer insert_arg_if_missing par "add_arg_if_missing"
  //    add_arg_if_missing(vo.name, vo.val, vo.oi_mode, vo.has_arg, vo.help, vo.func);

  // std::cout << "1. " << get1opts(vo) << std::endl;
  //  }

  //  n_opt=my_ropts.size();

  /*
    insert_arg_if_missing("batch", ' ', opt_only, no_argument, "work in batch mode default is to work in interactive mode if -h or -V are not provided.", [] (char, std::string, std::string) -> void { interp_on=false; });
    insert_arg_if_missing("inter", ' ', opt_only, no_argument, "work in interactive mode, this is the default mode if -h or -V are not provided.", [] (char, std::string, std::string) -> void { arg_sel=false; interp_on=true; });
  */
  // Try to insert --help and --version if not already done
  insert_arg_if_missing("version", 'v', opt_itr, no_argument, "display version information and eventually exit.", getVersion);
  insert_arg_if_missing("help", 'h', opt_itr, no_argument, "print this message and eventually exit.", getUsage);

  // std::cout << "2. OPT L: " << my_ropts.size() << std::endl;
  for (auto vo : my_ropts)
  {
    // std::cout << "2. " << get1opts(vo) << std::endl;
  }

  set_options();

  int option_index = 0, c;

  size_t idx;
  std::string oarg;

  while ((c = getopt_long_only(argc, argv, optstr.c_str(), long_options, &option_index)) != -1)
  {
    if (c == '?')
    {
      usage(std::cerr);
      if (!interp_on)
        exit(ENOTSUP);
    }
    else
    {
      idx = index_from_val(c);

      if (idx < n_opt)
      {

        if (my_ropts[idx].func != nullptr)
        {
          switch (my_ropts[idx].has_arg)
          {
          case required_argument:
            oarg = "";
            if (optarg)
              oarg = optarg;
            if (oarg == "" || (oarg[0] == '-' && oarg[1] != 0))
            {
              std::cerr << "Missing argument for option -" << my_ropts[idx].val << "/--" << my_ropts[idx].name << ")" << std::endl;
              usage(std::cerr);
              if (!interp_on)
                exit(ENOTSUP);
            }
            break;
          case optional_argument:
            if (!optarg && argv[optind] != nullptr && argv[optind][0] != '-')
            {
              oarg = argv[optind++];
            }
            break;
          default:
            oarg = "";
            break;
          }

          my_ropts[idx].func(c, my_ropts[idx].name, oarg);
        }
      }
    }
  }
}
