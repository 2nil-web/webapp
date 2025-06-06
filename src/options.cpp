
#include <algorithm>
#include <cctype>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "options.h"
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

void options::set(std::string p_progname, arg_dq p_args, opti_dq p_opt_inf)
{
  if (!progname.empty())
    progname = p_progname;
  if (!args.empty())
    args = p_args;
  if (!p_opt_inf.empty())
    opt_inf = p_opt_inf;
  add_default();
}

options::options(std::string p_progname, arg_dq p_args, opti_dq p_opt_inf)
{
  set(p_progname, p_args, p_opt_inf);
}

std::string options::version()
{
  std::string vers = progname + " version " + app_info.version;
  if (!app_info.decoration.empty())
    vers += ' ' + app_info.decoration;

  if (!app_info.commit.empty())
    vers += "\nCommit " + app_info.commit;
  if (!app_info.created_at.empty())
    vers += ". Creation time " + app_info.created_at;
  vers += '\n' + app_info.copyright;
  return vers;
}

std::ostream &options::version(std::ostream &os)
{
  os << version() << std::endl;

  return os;
}

std::string options::usage()
{
  std::string usage = version() + '\n';

  usage += "Usage: " + progname + " [OPTIONS] ARGUMENTS\n";
  usage += "Available options\n";

  size_t longest=0;
  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      if (opt.short_name != 0)
      {
        size_t curr_l=opt.long_name.size()+9;
        if (opt.mode == optional) curr_l+=6;
        if (opt.mode == required) curr_l+=4;
        if (curr_l > longest) longest=curr_l;
      }
    }
  }

  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      std::string opt_s={};
      if (opt.short_name != 0)
      {
        opt_s += " -" + std::string(1, opt.short_name);
        if (!opt.long_name.empty())
          opt_s += ", --" + opt.long_name;

        if (opt.mode == optional) opt_s+=" [ARG]";
        if (opt.mode == required) opt_s+=" ARG";
        usage += opt_s+std::string(longest-opt_s.size(), ' ');
      }

      usage += opt.help + '\n';
    }
  }

  return usage;
}

std::ostream &options::usage(std::ostream &os)
{
  os << usage() << std::endl;
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
          usage(std::cout);
          exit(0);
        },
        "Display this message and exit."));
  if (no_v)
    opt_inf.push_front(option_info(
        'v', "version",
        [this](s_opt_params &) -> void {
          version(std::cout);
          exit(0);
        },
        "Output version information and exit."));
}

arg_iter options::run_opt(option_info opt)
{
  s_opt_params p({opt.short_name, opt.long_name, {}, 0});

  if (opt.mode == optional || opt.mode == required)
  {
    if (p_arg_it != p_args.end()-1)
    {
      p.val = *(p_arg_it + 1);
      opt.func(p);
      if (p.ret)
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
  return p_arg_it;
}

arg_iter options::run_opt(char short_name)
{
  for (auto opt : opt_inf) {
    if (opt.short_name == short_name) {
      return run_opt(opt);
    }
  }

  std::cerr << "Uknown short option '-" << short_name << "', ignoring it." << std::endl;
  return p_arg_it;
}

arg_iter options::run_opt(std::string long_name)
{
  for (auto opt : opt_inf)
    if (opt.long_name == long_name)
      return run_opt(opt);
  std::cerr << "Uknown long option '--" << long_name << "', ignoring it." << std::endl;
  return p_arg_it;
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
        // Or simple dash with a short option
        else
        {
          p_arg_it = run_opt((*p_arg_it)[1]);
        }
      }
      else if (p_arg_it->size() > 2)
      {
        // double dash with a long option
        if ((*p_arg_it)[1] == '-')
        {
          p_arg_it = run_opt((*p_arg_it).substr(2));
        }
        // Or simple dash with a long option OR multiple short options
        else
        {
          // p_arg_it = run_opt((*p_arg_it).substr(1));
          for (size_t i = 1; (*p_arg_it)[i] && !std::isspace((*p_arg_it)[i]); i++)
          {
            p_arg_it = run_opt((*p_arg_it)[i]);
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
