
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

option_info::option_info(std::string p_help)
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

options::options(int argc, char **argv, opti_dq p_opt_inf)
{
  progname = std::filesystem::path(argv[0]).stem().string();
  args.insert(args.begin(), argv + 1, argv + argc);
  if (!p_opt_inf.empty())
    opt_inf = p_opt_inf;
  add_default();
}

options::options(std::string p_progname, arg_dq p_args, opti_dq p_opt_inf)
{
  if (!progname.empty())
    progname = p_progname;
  if (!args.empty())
    args = p_args;
  if (!p_opt_inf.empty())
    opt_inf = p_opt_inf;
  add_default();
}

std::ostream &options::default_version(std::ostream &os)
{
  os << progname << " version " << app_info.version;
  if (!app_info.commit.empty())
    os << ' ' << app_info.commit;
  if (!app_info.decoration.empty())
    os << ' ' << app_info.decoration;
  if (!app_info.created_at.empty())
    os << ' ' << app_info.created_at;
  os << " - " << app_info.copyright << std::endl;
  return os;
}

std::ostream &options::default_usage(std::ostream &os)
{
  default_version(os);

  os << "Usage: " << progname << " [OPTIONS] ARGUMENTS" << std::endl;
  os << "Available options" << std::endl;

  for (auto opt : opt_inf)
  {
    if (!opt.help.starts_with("SECRET_OPTION"))
    {
      if (opt.short_name != 0)
      {
        os << " -" << opt.short_name;
        if (!opt.long_name.empty())
          os << ", --" << opt.long_name << ": ";
        else
          os << ": ";
      }
      else if (!opt.long_name.empty())
        os << "      " << opt.long_name << ": ";

      os << opt.help << std::endl;
    }
  }

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
          default_usage(std::cout);
          exit(0);
        },
        "display this message and exit."));
  if (no_v)
    opt_inf.push_front(option_info(
        'v', "version",
        [this](s_opt_params &) -> void {
          default_version(std::cout);
          exit(0);
        },
        "output version information and exit."));
}

arg_iter options::run_opt(arg_iter arg_it, option_info opt)
{
  s_opt_params p({opt.short_name, opt.long_name, {}, 0});

  if (opt.mode == optional || opt.mode == required)
  {
    if (arg_it != args.end())
    {
      p.val = *(arg_it + 1);
      opt.func(p);
      if (p.ret)
        arg_it++;
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
  return arg_it;
}

arg_iter options::run_opt(arg_iter arg_it, char short_name)
{
  for (auto opt : opt_inf)
    if (opt.short_name == short_name)
      return run_opt(arg_it, opt);

  std::cerr << "Uknown short option '" << short_name << "', ignoring it." << std::endl;
  return arg_it;
}

arg_iter options::run_opt(arg_iter arg_it, std::string long_name)
{
  for (auto opt : opt_inf)
    if (opt.long_name == long_name)
      return run_opt(arg_it, opt);
  std::cerr << "Uknown long option '" << long_name << "', ignoring it." << std::endl;
  return arg_it;
}

void options::parse()
{
  auto l_args = args;
  args.clear();

  for (auto it = l_args.begin(); it != l_args.end(); ++it)
  {
    if ((*it)[0] == '-')
    {
      // simple dash: one short option
      if (it->size() == 2 && (*it)[1] != '-')
      {
        it = run_opt(it, (*it)[1]);
      }
      else if (it->size() > 2)
      {
        // double dash: one long option
        if ((*it)[1] == '-')
        {
          it = run_opt(it, (*it).substr(2));
        }
        // simple dash: one long option OR multiple short options
        else
        {
          // it = run_opt(it, (*it).substr(1));
          for (size_t i = 1; (*it)[i] && !std::isspace((*it)[i]); i++)
          {
            it = run_opt(it, (*it)[i]);
          }
        }
      }
    }
    else
    {
      // Not option, remaining arg
      args.push_back(*it);
    }
  }
}

std::ostream &operator<<(std::ostream &os, options &opts)
{
  opts.default_version(os);

  for (auto opt : opts.opt_inf)
  {
    os << opt;
  }

  return os;
}
