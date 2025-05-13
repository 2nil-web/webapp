
#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "tail.h"

std::string progname;

// clear to eol in pure C/C++
inline void clear_line(size_t width = 80)
{
  std::cout << '\r' << std::string(width, ' ') << '\r' << std::flush;
}

// Tries to assert that a file is not currently being written
// The strategy here is to make a maximum of 'nloop' calls to std::filesystem::file_size with a pause of 'dur' milliseconds between each
// And the hope is to retrieve twice the same size that will lead to conclude that the file size is 'stable'
std::uintmax_t stable_file_size(const std::string &filename, const int nloop = 10, const int dur = 200)
{
  std::uintmax_t prev_fs, curr_fs;
  std::error_code ec;

  // Initial retrieval of the file size
  curr_fs = std::filesystem::file_size(filename, ec);

  // Loop hoping to find curr_fs and prev_fs equal
  for (int i = 0; i < nloop; i++)
  {
    // A little sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(dur));

    // Then retrieval of the file size
    prev_fs = curr_fs;
    curr_fs = std::filesystem::file_size(filename, ec);
    if (ec.value() != 0)
      continue;
    // We found a 'stable' file size meaning/hoping that the file is not actually being written
    if (prev_fs == curr_fs)
      break;
  }

  return curr_fs;
}

// Get an ifstream lines count
inline size_t linecount(std::ifstream &file)
{
  return std::count_if(std::istreambuf_iterator<char>{file}, {}, [](char c) { return c == '\n'; });
}

// Get a file path lines count
size_t linecount(const std::string filename)
{
  std::ifstream file(filename);
  size_t count = linecount(file);
  file.close();
  return count;
}

// Tail an ifstream from a specific position
size_t tail_from_pos(std::ifstream &file, size_t pos, size_t total_line, size_t start_line, bool forward)
{
  // Position in the file from the start or from the end
  if (forward)
    file.seekg(pos, file.beg);
  else
    file.seekg(-((int)pos), file.end);

  // Put the rest of file into a double-ended queue
  struct sPositionedLine
  {
    size_t pos;
    std::string s;
  };
  std::deque<sPositionedLine> positioned_lines;
  std::string line;
  while (getline(file, line))
    positioned_lines.push_back({(size_t)(file.tellg()), line});

  // Is there something in the deque ?
  if (positioned_lines.size() > 0)
  {
    // Remove out of bounds lines, if necessary
    if (start_line > 0)
    {
      while (positioned_lines.size() > start_line)
        positioned_lines.pop_front();
    }

    // Is line numbering required ?
    if (total_line > 0)
    {
      // Get the widdest line number
      size_t ln_width = std::to_string(1 + positioned_lines.back().s.size()).size();
      // Set the the first line number of the deque
      size_t line_number = 1 + total_line - start_line; // positioned_lines.back().s.size();
      // Display the numbered positioned_lines
      for (auto positioned_line : positioned_lines)
        std::cout << std::setfill('0') << std::setw(ln_width) << line_number++ << ':' << positioned_line.s << std::endl;
    }
    else
    { // Or not ?
      for (auto positioned_line : positioned_lines)
        std::cout << positioned_line.s << std::endl;
    }

    return positioned_lines.back().pos;
  }

  return 0;
}

// Tail a file path from a specific position
size_t tail_from_pos(std::string filename, size_t pos, size_t total_line, size_t start_line, bool forward)
{
  std::ifstream file(filename);
  size_t file_size = tail_from_pos(file, pos, total_line, start_line, forward);
  file.close();
  return file_size;
}

// Tail a file at the character position corresponding to the provided start_line
size_t tail_once(std::string filename, size_t &total_line, bool num, const size_t start_line, size_t seek_step)
{
  // First line number is 1, and zero is assumed as the 'after last line number', then it is non sense asking to tail a file after its last line
  if (start_line == 0)
    return 0;

  std::ifstream file(filename);

  // Computing the number of lines, if needed
  if (num && total_line == 0)
  {
    const std::string msg = "Computing line count, wait ...";
    std::cout << msg << std::flush;
    total_line = linecount(file);
    clear_line(msg.size());
  }

  // Get file size
  file.seekg(0, file.end);
  size_t file_size = file.tellg();

  // File is empty, then aborting ...
  if (file_size == 0)
  {
    file.close();
    return 0;
  }

  // If seek_step has a value of 0 then set it as an average of 300 characters per line
  if (seek_step == 0)
    seek_step = start_line * 300;

  size_t pos;

  if (file_size < seek_step)
  {
    file.seekg(0, file.beg);
    linecount(file);
    pos = file_size;
  }
  else
  {
    pos = seek_step;
    size_t ge_nl;

    for (;;)
    {
      file.seekg(-(int)pos, file.end);
      ge_nl = linecount(file);
      if (ge_nl > start_line || pos > file_size)
        break;
      pos += seek_step;
    }
  }

  // Here we have a position that includes the required line nuber
  size_t ret = tail_from_pos(file, pos, total_line, start_line, false);
  file.close();
  return ret;
}

// The main feature interface function
size_t tail(std::string filename, bool poll, bool num, const size_t start_line, size_t seek_step)
{
  // First tail of the provided file
  size_t total_line = 0, file_size = tail_once(filename, total_line, num, start_line, seek_step);

  // Is it required to poll the file continuously
  if (poll)
  {
    size_t new_file_size;

    // Entering an endless loop
    for (;;)
    {
      // Retrieving a 'stable' the file size
      new_file_size = stable_file_size(filename);

      // The file is growing
      if (new_file_size > file_size)
      {
        if (num)
        {
          file_size = tail_from_pos(filename, file_size, total_line++);
        }
        else
        {
          file_size = tail_from_pos(filename, file_size, 0);
        }
      }
      else if (new_file_size < file_size)
      { // The file is shrinking
        std::cerr << progname << ": file '" << filename << "' has been truncated, displaying only its last " << start_line << " lines." << std::endl;
        total_line = 0;
        file_size = tail_once(filename, total_line, num, start_line, seek_step);
      }
    }
  }

  return file_size;
}

#ifdef STAND_ALONE
// A little help message
void usage()
{
  std::cout << "Usage: " << progname << " [-h] [-n] [-f] filename [line number]." << std::endl;
  std::cout << progname << R"EOF( may have one of the following options:
  -h display this message and exit.
  -n: add the line number at the beginning of each line. This could result in a longer startup time.
  -f: output appended data as the file grows.
And, at least one or eventually two parameters:
  The mandatory filename.
  The optional line number, counted from the end, to start the display. (It is 10 by default).)EOF";
  //  And the optional seek step increment to guess the position corresponding to the required number of lines you want to display (300 by default).)EOF";
}

// The main function
int main(int argc, char **argv, char **)
{
  progname = std::filesystem::path(argv[0]).stem().string();
  std::deque<std::string> args(argv + 1, argv + argc);
  bool poll = false, num = false;

  while (args.size() >= 1 && args[0][0] == '-')
  {
    if (args[0] == "-f")
    {
      poll = true;
      args.pop_front();
    }

    if (args[0] == "-n")
    {
      num = true;
      args.pop_front();
    }

    if (args[0] == "-h")
    {
      usage();
      return 0;
    }
  }

  switch (args.size())
  {
  case 1:
    tail(args[0], poll, num);
    break;
  case 2:
    tail(args[0], poll, num, std::stol(args[1]));
    break;
  case 3:
    tail(args[0], poll, num, std::stol(args[1]), std::stol(args[2]));
    break;
  default: {
    if (args.size() == 0)
    {
      std::cerr << "Missing at least one mandatory parameter: the filename." << std::endl;
      usage();
    }
    else if (args.size() > 3)
    {
      std::cerr << "Too many parameters, we continue anyway...";
      tail(args[0], poll, num, std::stol(args[1]), std::stol(args[2]));
    }

    return EINVAL;
  }
  break;
  }

  return 0;
}
#endif
