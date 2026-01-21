
#include <algorithm>
#include <cctype>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#endif

#include "options.h"
#include "util.h"
#include "version.h"

option_info::option_info(char p_short_name, std::string p_long_name, t_opt_func p_func, std::string p_help, e_option_mode p_mode, e_option_env p_env)
{
  short_name = p_short_name;
  long_name = p_long_name;
  func = p_func;
  help = p_help;
  mode = p_mode;
  env = p_env;
}

option_info::option_info(const std::string p_help)
{
  mode = only_help;
  help = p_help;
}

std::ostream &operator<<(std::ostream &os, option_info &opt)
{
#ifdef FULL_OUTPUT
  os << "short_name:" << opt.short_name << ", long_name:[" << opt.long_name << "], help:[" << opt.help << "], mode:" << opt.mode << ", env:" << opt.env << std::endl;

#else

  os << "mode:" << opt.mode << ", env:" << opt.env << ", ";

  if (opt.func)
    os << "has A  func, ";
  else
    os << "has NO func, ";

  if (opt.mode != only_help)
  {
    if (opt.short_name != 0)
      os << "short_name:[" << opt.short_name << "], ";
    if (!opt.long_name.empty())
      os << "long_name:[" << opt.long_name << "], ";
  }

  if (!opt.help.empty())
    os << "help:[" << opt.help << ']';
  os << std::endl;
#endif
  return os;
}

void options::set(int argc, char **argv, opti_dq p_opt_inf)
{
  progname = std::filesystem::path(argv[0]).stem().string();
  args.insert(args.begin(), argv + 1, argv + argc);
  if (!p_opt_inf.empty())
    opt_inf = p_opt_inf;
  add_default();
}

options::options(int argc, char **argv, opti_dq p_opt_inf)
{
  set(argc, argv, p_opt_inf);
}

void options::set(std::string p_progname, arg_dq l_args, opti_dq p_opt_inf)
{
  if (!progname.empty())
    progname = p_progname;
  if (!args.empty())
    args = l_args;
  if (!p_opt_inf.empty())
    opt_inf = p_opt_inf;
  add_default();
}

options::options(std::string p_progname, arg_dq l_args, opti_dq p_opt_inf)
{
  set(p_progname, l_args, p_opt_inf);
}

std::string options::version(bool traceability)
{
  std::string vers = progname + " version " + app_info.version;
  if (!app_info.decoration.empty())
    vers += ' ' + app_info.decoration;

  if (traceability)
  {
    if (!app_info.commit.empty())
      vers += "\nCommit " + app_info.commit;
    if (!app_info.created_at.empty())
      vers += ". Creation time " + app_info.created_at;

    vers += ". Built for " + get_build();
  }
  vers += '\n' + app_info.copyright;
  return vers;
}

std::ostream &options::version(std::ostream &os, bool traceability)
{
  os << version(traceability) << std::endl;

  return os;
}

// Insert a substring in a string at every space before 'l' position ...
std::string in_frame(std::string s, std::string ss, size_t l = 80)
{
  std::string res = {};
  size_t last_space_pos = 0;
  for (size_t i = 0; i < s.size(); i++)
  {
    if (i > 0 && (i % l) == 0)
    {
      // Insert substring at last space position
      res.insert(last_space_pos, ss);
    }

    res += s[i];
    if (res[i] == ' ')
      last_space_pos = i;
  }

  return res;
}

std::string options::usage_opt(size_t max_width)
{
  std::string usage = version() + '\n';

  usage += "Usage: " + progname + " [OPTIONS] ARGUMENTS\n";
  usage += "Available options\n";

  size_t longest_opt = 0;
  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      if (opt.short_name != 0 || !opt.long_name.empty())
      {
        size_t curr_l = opt.long_name.size() + 9;
        if (opt.mode == optional)
          curr_l += 6;
        if (opt.mode == required)
          curr_l += 4;
        if (curr_l > longest_opt)
          longest_opt = curr_l;
      }
    }
  }

  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      std::string opt_s = {};
      if (opt.short_name != 0 || !opt.long_name.empty())
      {
        if (opt.short_name != 0)
          opt_s += " -" + std::string(1, opt.short_name);
        else
          opt_s += "  ";

        if (!opt.long_name.empty())
        {
          if (opt.short_name != 0)
            opt_s += ',';
          else
            opt_s += ' ';
          opt_s += " --" + opt.long_name;
        }

        if (opt.mode == optional)
          opt_s += " [ARG]";
        if (opt.mode == required)
          opt_s += " ARG";
        usage += opt_s + std::string(longest_opt - opt_s.size(), ' ');
      }

      if (max_width > 0)
        usage += in_frame(opt.help, '\n' + std::string(longest_opt + 1, ' '), max_width - longest_opt);
      else
        usage += opt.help;
      usage += '\n';
    }
  }

  return usage;
}

std::string options::usage_int(size_t max_width)
{
  std::string usage = {};

  usage += "Available commands/and their shortcut\n";

  size_t longest_opt = 0;
  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      if (opt.short_name != 0 || !opt.long_name.empty())
      {
        size_t curr_l = opt.long_name.size() + 3;
        if (opt.mode == optional)
          curr_l += 6;
        if (opt.mode == required)
          curr_l += 4;
        if (curr_l > longest_opt)
          longest_opt = curr_l;
      }
    }
  }

  bool first = true;
  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      if (!first)
        usage += '\n';
      else
        first = false;

      std::string opt_s = {};
      if (opt.short_name != 0 || !opt.long_name.empty())
      {
        if (!opt.long_name.empty())
        {
          opt_s += opt.long_name;
        }

        if (opt.short_name != 0)
        {
          opt_s += "/" + std::string(1, opt.short_name);
        }

        if (opt.mode == optional)
          opt_s += " [ARG]";
        if (opt.mode == required)
          opt_s += " ARG";
        usage += opt_s + std::string(longest_opt - opt_s.size(), ' ');
      }

      if (max_width > 0)
        usage += in_frame(opt.help, '\n' + std::string(longest_opt + 1, ' '), max_width - longest_opt);
      else
        usage += opt.help;
    }
  }

  return usage;
}

std::string options::usage(size_t max_width)
{
  if (imode)
    return usage_int(max_width);
  else
    return usage_opt(max_width);
}

std::ostream &options::usage(std::ostream &os, size_t max_width)
{
  os << usage(max_width) << std::endl;
  return os;
}

// Add -h/-help/--help and -v/-version/--version
void options::add_default()
{
  bool no_v = true, no_h = true;
  for (auto opt : opt_inf)
  {
    if (opt.short_name == 'h' || opt.long_name == "help")
      no_h = false;
    if (opt.short_name == 'v' || opt.long_name == "version")
      no_v = false;
  }

  if (no_h)
    opt_inf.push_front(option_info(
        'h', "help",
        [this](s_opt_params &) -> void {
          // Could use ncurse (pdcurses.org) to define max_width as the console window width ... Defaulting to 100
          usage(std::cout, 100);
          if (!imode)
            exit(0);
        },
        "Display this message (And exit if not in interpreted mode)."));
  if (no_v)
  {
    opt_inf.push_front(option_info(
        'v', "version",
        [this](s_opt_params &) -> void {
          version(std::cout);
          if (!imode)
            exit(0);
        },
        "Output version information (And exit if not in interpreted mode)."));

    opt_inf.push_front(option_info(
        0, "traceability",
        [this](s_opt_params &) -> void {
          version(std::cout, true);
          if (!imode)
            exit(0);
        },
        "SECRET_OPTION provided for traceability when needed (debug)."));
  }

  opt_inf.push_front(option_info(
      'p', "prompt",
      [this](s_opt_params &p) -> void {
        trim(p.val, "\"");
        prompt = p.val;
      },
      "Change the interpreted mode prompt (default is '>')."));
}

void options::run_opt(option_info opt)
{
  s_opt_params p({opt.short_name, opt.long_name, {}, 0});

  if (opt.mode == optional || opt.mode == required)
  {
    if (p_arg_it != p_args.end() - 1)
    {
      p.val = *(p_arg_it + 1);
      opt.func(p);
      // if (p.ret)
      p_arg_it++;
    }
    else if (opt.mode == e_option_mode::required)
    {
      std::cerr << "Missing argument to '" << opt.short_name << '/' << opt.long_name << "', ignoring this option." << std::endl;
    }
  }
  else
  {
    opt.func(p);
  }
}

void options::run_opt(char short_name)
{
  for (auto opt : opt_inf)
  {
    if (opt.short_name == short_name)
    {
      run_opt(opt);
      return;
    }
  }

  std::cerr << "Uknown short option '-" << short_name << "', ignoring it." << std::endl;
}

void options::run_opt(std::string long_name)
{
  for (auto opt : opt_inf)
    if (opt.long_name == long_name)
    {
      run_opt(opt);
      return;
    }

  std::cerr << "Uknown long option '--" << long_name << "', ignoring it." << std::endl;
}

void options::parse()
{
  p_args = args;
  args.clear();

  for (p_arg_it = p_args.begin(); p_arg_it != p_args.end(); p_arg_it++)
  {
    if ((*p_arg_it)[0] == '-')
    {
      // Simple dash alone
      if (p_arg_it->size() == 1)
      {
        args.push_back(*p_arg_it); // Not option, remaining arg
      }
      // Simple dash not alone
      else if (p_arg_it->size() == 2)
      {
        // Double dash alone
        if ((*p_arg_it)[1] == '-')
        {
          args.push_back(*p_arg_it); // Not option, remaining arg
        }
        // Simple dash with a short option
        else
        {
          run_opt((*p_arg_it)[1]);
        }
      }
      else if (p_arg_it->size() > 2)
      {
        // double dash with a long option
        if ((*p_arg_it)[1] == '-')
        {
          run_opt((*p_arg_it).substr(2));
        }
        // Simple dash with a long option OR multiple short options
        else
        {
          // run_opt((*p_arg_it).substr(1));
          for (size_t i = 1; (*p_arg_it)[i] && !std::isspace((*p_arg_it)[i]); i++)
          {
            run_opt((*p_arg_it)[i]);
          }
        }
      }
    }
    else
    {
      args.push_back(*p_arg_it); // Not option, remaining arg
    }
  }
}

void options::parse(std::istream &is)
{
  imode = true;

  std::string s, r1, r2;
  s_opt_params op;
  bool unknown_cmd;

  for (;;)
  {
    if (&is == &std::cin)
      std::cout << prompt;

    if (!std::getline(is, s))
      break;

    trim(s);

    if (!s.empty() && s[0] > ' ')
    {
      split_1st(r1, r2, s);

      unknown_cmd = true;
      for (auto opt : opt_inf)
      {
        if (r1[0] == opt.short_name || r1 == opt.long_name)
        {
          op = {opt.short_name, opt.long_name, r2, 0};
          if (r2.empty() && opt.mode == e_option_mode::required)
          {
            std::cerr << "Missing argument to '" << opt.short_name << '/' << opt.long_name << "', ignoring this command." << std::endl;
          }
          else
          {
            opt.func(op);
          }

          unknown_cmd = false;
          break;
        }
      }

      if (unknown_cmd)
      {
        std::cerr << "Unknown command '" << s << "', ignoring it." << std::endl;
      }
    }
  }

  imode = false;
}

void options::parse(std::filesystem::path path)
{
  std::ifstream file(path, std::ios::binary);
  if (file)
  {
    //    std::cout << "Parsing file " << path << std::endl;
    parse(file);
  }
  file.close();
}

// From Freak, see :
// https://stackoverflow.com/questions/152016/detecting-cpu-architecture-compile-time
std::string options::get_build()
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

std::ostream &operator<<(std::ostream &os, options &opts)
{
  opts.version(os);

  for (auto opt : opts.opt_inf)
  {
    os << opt;
  }

  return os;
}
