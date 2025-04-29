#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <iostream>
#include <sstream>

#include "util.h"

// Log priorities
// ALL < TRACE < DEBUG < INFO < WARN < ERROR < FATAL < OFF
enum eLogLevel
{
  LEVEL_ALL = 0,
  LEVEL_TRACE = 1,
  LEVEL_DEBUG = 2,
  LEVEL_INFO = 3,
  LEVEL_WARN = 4,
  LEVEL_ERROR = 5,
  LEVEL_FATAL = 6,
  LEVEL_UNDEFINED = 7,
  LEVEL_OFF = 8
};

// Check and eventually correct the log level
void check_log_level();

void send2log(eLogLevel, std::string, int, std::string);

template <typename... Args> void level_log(eLogLevel llvl, std::string fn, int ln, Args &&...log_args)
{
  std::stringstream ss;
  ((ss << std::forward<Args>(log_args)), ...);
  send2log(llvl, fn, ln, ss.str());
}

#define llog(lvl, ...) level_log(LEVEL_##lvl, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)

#define logTrace(...) llog(TRACE, __VA_ARGS__)
#define logT(...) logTrace(__VA_ARGS__)

#define logDebug(...) llog(DEBUG, __VA_ARGS__)
#define logD(...) logDebug(__VA_ARGS__)

#define logInfo(...) llog(INFO, __VA_ARGS__)
#define logI(...) logInfo(__VA_ARGS__)

#define logWarn(...) llog(WARN, __VA_ARGS__)
#define logW(...) logWarn(__VA_ARGS__)

#define logError(...) llog(ERROR, __VA_ARGS__)
#define logE(...) logError(__VA_ARGS__)

#define logFatal(...) llog(FATAL, __VA_ARGS__)
#define logF(...) logFatal(__VA_ARGS__)

#endif // LOG_H
