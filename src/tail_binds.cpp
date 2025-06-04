
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
#include "util.h"
#include "log.h"
#include "wrap.h"
#include "tail.h"
// clang-format on

#define trc std::cout << __LINE__ << std::endl

void create_tail_binds(webview_wrapper &w)
{
  w.bind_doc( //
      "tail_threaded",
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::string cmd = json_parse(req, "", 0), res;

          w.resolve(seq, 0, w.json_escape(res));
        }).detach();
      },
      "DOC.", //
      1       // One mandatory params
  );

  w.bind_doc(
      "tail_not_threaded",                    //
      [&](const std::string &) -> std::string //
      { return ""; },
      "DOC."  //
          - 1 // One optional params
  );
}
