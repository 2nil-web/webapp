#ifndef MY_GETOPT_H
#define MY_GETOPT_H

#include <functional>
#include <string>
#include <vector>

#include <getopt.h>

typedef std::function<void(char c, std::string, std::string)> OptFunc;
// typedef [](char c, std::string name, std::string val)->void LambdaOptFunc;

enum io_mode
{
  OPTION_INTERP = 0,
  OPTION = 1,
  INTERP = 2
};
#define opt_itr 0
#define opt_only 1
#define itr_only 2

struct run_opt
{
  std::string name;
  char val;
  int oi_mode;
  int has_arg;
  std::string help;
  OptFunc func;
  //  std::function<void(char c, std::string, std::string)> func;
  // void (*func) (char c, std::string, std::string);
};

bool interp();
std::string appInfo();
std::wstring appInfoW();
std::string usage(std::ostream &out = std::cout);
void getopt_init(int argc, char **argv, std::vector<run_opt> pOptions, const std::string pIntro = "Some introductory help message", const std::string pVersion = "1.0.0", const std::string copyright = "");

#endif /* MY_GETOPT_H */
