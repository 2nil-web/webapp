
#ifndef OPTIONS_H
#define OPTIONS_H

#include <deque>
#include <functional>

struct s_opt_params
{
  char short_name = 0;
  std::string long_name = {};
  std::string val = {};
  int ret = 0;
};

// typedef std::function<void(char short_name, std::string long_name, std::string value)> t_opt_func;
typedef std::function<void(s_opt_params &)> t_opt_func;
// typedef [](char short_name, std::string long_name, std::string value)->void t_opt_lambda;

// Environment option, might be used as option, in interpreted mode, both or none (useful to add a comment in the help message)
enum e_option_env
{
  all = 0,
  option = 1,
  interp = 2,
  no_env = 3
};

enum e_option_mode
{
  no_arg = 0,
  optional = 1,
  required = 2,
  only_help = 3
};

class option_info
{
public:
  option_info(const std::string help);
  option_info(char short_name, std::string long_name, t_opt_func func = {}, std::string help = {}, e_option_mode mode = {}, e_option_env env = {});

  char short_name = 0;
  std::string long_name = {};
  t_opt_func func = nullptr;
  std::string help = {};
  e_option_mode mode = no_arg;
  e_option_env env = all;
};

using arg_dq = std::deque<std::string>;
using arg_iter = arg_dq::iterator;

using opti_dq = std::deque<option_info>;

class options
{
public:
  std::string progname = {};
  arg_dq args = {};
  opti_dq opt_inf = {};

  options(std::string progname = {}, arg_dq args = {}, opti_dq opt_inf = {});
  options(int argc, char **argv, opti_dq opt_inf = {});
  void set(int argc, char **argv, opti_dq p_opt_inf);
  void set(std::string p_progname, arg_dq p_args, opti_dq p_opt_inf);
  void add(option_info){};
  void parse();
  void parse(std::istream &is);
  void parse(std::filesystem::path);
  std::string usage(size_t max_width = 0);
  std::ostream &usage(std::ostream &, size_t max_width = 0);
  std::string version(bool traceability = false);
  std::ostream &version(std::ostream &, bool traceability = false);
  std::string get_build();

private:
  std::string prompt = "> ";
  bool imode = false;
  arg_dq p_args = {};
  arg_iter p_arg_it;
  void run_opt(char short_name);
  void run_opt(std::string long_name);
  void run_opt(option_info opt);
  std::string usage_opt(size_t max_width = 0);
  std::string usage_int(size_t max_width = 0);
  void add_default();
  friend std::ostream &operator<<(std::ostream &, options &);
};
#endif /* OPTIONS_H */
