
#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#ifdef _MSVC_LANG
#include <io.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#define read _read
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "tail.h"

// clear to eol in pure C/C++
inline void clear_line(size_t width = 80)
{
  std::cout << '\r' << std::string(width, ' ') << '\r' << std::flush;
}

std::uintmax_t my_file_size(std::istream &is)
{
  is.seekg(0, is.end);
  return is.tellg();
}

std::uintmax_t my_file_size(std::filesystem::path filename)
{
  if (filename.string() == "-")
  {
    return my_file_size(std::cin);
  }
  return std::filesystem::file_size(filename);
}

// Tries to assert that a file is not currently being written.
// The strategy here is to make
//    a) A maximum of 'nloop' calls (default 10) to my_file_size
//    b) With a pause of 'dur' milliseconds (default 200) between each
// And the hope is to retrieve twice the same size in a row leading to conclude that
//    a) The file size is 'stable'
//    b) Hence not being written
std::uintmax_t stable_file_size(std::filesystem::path filepath, const int nloop = 10, const int dur = 200)
{
  std::uintmax_t prev_fs, curr_fs;

  // Initial retrieval of the file size
  curr_fs = my_file_size(filepath);

  // Loop hoping to find curr_fs and prev_fs equal
  for (int i = 0; i < nloop; i++)
  {
    // A little sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(dur));

    // Then retrieval of the file size
    prev_fs = curr_fs;
    curr_fs = my_file_size(filepath);
    // We found a 'stable' file size meaning/hoping that the file is not actually being written
    if (prev_fs == curr_fs)
      break;
  }

  return curr_fs;
}

// Get an istream lines count from the current position
inline size_t tail::linecount_from_current_pos(std::istream &is)
{
  return std::count_if(std::istreambuf_iterator<char>{is}, {}, [this](char c) { return c == delimiter; });
  // return std::count(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>(), delimiter);
}

// Get a file path lines count
size_t tail::linecount(std::filesystem::path filepath)
{
  size_t count = 0;

  std::ifstream file(filepath);

  if (file)
  {
    count = linecount_from_current_pos(file);
    file.close();
  }

  return count;
}

// From the quick test I did, counting lines in pure C is much faster than in C++
// It is around 2 to 3 times faster with gcc/C versus gcc/C++ (Windows and Linux)
// And is around 10(!) times faster with msvc/C versus msvc/C++ (Windows only)
// Just keeping the C++ way in case the C way fails
// A timing test on a Windows 11 PC with Core I9 and nvme ssd to count a 66 gigabytes syslog file of 432 096 650 lines, gives 2min and 28sec with this function, versus 47sec with the 'wc -l' command line

// Get a FILE* lines count
inline size_t tail::clinecount(FILE *file, size_t &fsize)
{
  fseek(file, 0, SEEK_END);
  fsize = ftell(file);
  char *s = new char[fsize + 1];
  fseek(file, 0, SEEK_SET);
  if (fread(s, fsize, 1, file) != 1)
    return 0;

  char *pl = s, *p = s;
  size_t count = 0;

  // The 'do while' that nests the 'while' is here to go ahead if strchr is being fooled by eventual null characters that might contents the file opened in binary mode
  do
  {
    p = pl;
    while ((p = strchr(p, delimiter)))
    {
      count++;
      p++;
      pl = p;
    }
    pl++;
  } while ((size_t)(pl - s) < fsize);

  delete[] s;

  return count;
}

// Get a file path lines count in the C way
inline size_t tail::clinecount(std::filesystem::path filepath, size_t &fsize)
{
  if (filepath.string() == "-")
    return clinecount(stdin, fsize);

  FILE *file = nullptr;

#ifdef _WIN32
  fopen_s(&file, filepath.string().c_str(), "rb");
#else
  file = fopen(filepath.string().c_str(), "rb");
#endif

  if (file == nullptr)
    return 0;

  size_t count = clinecount(file, fsize);
  fclose(file);

  return count;
}

// Returns 1 on success, 0 when not done, and -1 on failure (check errno)
// str is resetted by this function.
// Taken from https://stackoverflow.com/questions/41558908/how-can-i-use-getline-without-blocking-for-input
bool getline_async_thread_safe(const int &fd, std::string &str, char delim = '\n')
{
  str = {};

  int chars_read;
  do
  {
    char buf[2] = {0};

    chars_read = (int)read(fd, buf, 1);

    if (chars_read == 1)
    {
      if (*buf == delim)
      {
        return true;
      }
      str.append(buf);
    }
    else
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        chars_read = 0;
        break;
      }
    }
  } while (chars_read > 0);

  return (chars_read == 1);
}

// Special case for std::cin, non blocking getline (only tested and working with gcc compiler, does not with msvc).
bool noblock_getline(std::istream &is, std::string &str, char delim = '\n')
{
  if (&is == &std::cin)
  {
    if (getline_async_thread_safe(0, str, delim) == 1)
    {
      // is.seekg(str.size()+1, is.cur);
      return true;
    }
  }
  else
  {
    if (std::getline(is, str, delim))
      return true;
  }

  return false;
}

#define trc std::cout << __LINE__ << std::endl
// Tail an istream from a specific position
size_t tail::from_pos(std::istream &is, size_t total_line, size_t pos, bool forward)
{
  // Position in the is from the start or from the end
  if (forward)
  {
    is.seekg(pos, is.beg);
    // std::cout << "Pos forward: ";
  }
  else
  {
    is.seekg(-((int)pos), is.end);
    // std::cout << "Pos backward: ";
  }

  // Put the rest of is into a double-ended queue
  struct sPositionedLine
  {
    size_t pos;
    std::string s;
  };

  std::deque<sPositionedLine> positioned_lines;
  std::string line = {};
  size_t /*prev_tell = 1,*/ tell = 0;

  while (noblock_getline(is, line, delimiter))
  {
    tell = is.tellg();
    positioned_lines.push_back({tell, line});
  }

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
    if (line_numbers)
    {
      // Get the widdest line number
      size_t ln_width = std::to_string(1 + positioned_lines.back().s.size()).size();
      // Set the the first line number of the deque

      size_t line_number;
      if (total_line < start_line)
        line_number = 1;
      else
        line_number = (1 + total_line - start_line); // positioned_lines.back().s.size();

      // Display the numbered positioned_lines
      for (auto positioned_line : positioned_lines)
        std::cout << std::setfill('0') << std::setw(ln_width) << (size_t)line_number++ << ':' << positioned_line.s << std::endl;
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
size_t tail::from_pos(std::filesystem::path filepath, size_t total_line, size_t pos, bool forward)
{
  if (filepath.string() == "-")
  {
    return from_pos(std::cin, total_line, pos, forward);
  }

  std::ifstream file(filepath);
  size_t file_size = from_pos(file, total_line, pos, forward);
  file.close();
  return file_size;
}

// Tail an istream at the character position corresponding to the provided start_line
size_t tail::once(std::istream &is, size_t &total_line, size_t file_size)
{
  // If seek_step has a value of 0 then set it as an average of 300 characters per line
  if (seek_step == 0)
    seek_step = start_line * 300;

  size_t pos;

  //  std::cout << "file_size: " << file_size << ", seek_step: " << seek_step << std::endl;
  if (file_size < seek_step)
  {
    is.seekg(0, is.beg);
    linecount_from_current_pos(is);
    pos = file_size;
  }
  else
  {
    pos = seek_step;
    size_t ge_nl = 0;

    for (;;)
    {
      is.seekg(-(int)pos, is.end);
      ge_nl = linecount_from_current_pos(is);

      if (ge_nl > start_line || pos > file_size)
        break;
      pos += seek_step;
    }
  }

  // Here we have a position that includes the required line nuber
  // std::cout << "total_line: " << total_line << ", pos: " << pos << std::endl;
  size_t ret = from_pos(is, total_line, pos, false);
  return ret;
}

// Tail a file at the character position corresponding to the provided start_line
size_t tail::once(std::filesystem::path filepath, size_t &total_line)
{
  size_t file_size = 0;
  // Computing the number of lines, if needed
  if (line_numbers && total_line == 0)
  {
    const std::string msg = "Computing line count, wait ...";
    std::cout << msg << std::flush;
    total_line = clinecount(filepath, file_size);
    clear_line(msg.size());
  }
  else
    file_size = my_file_size(filepath);

  if (file_size == 0)
    return 0;

  if (filepath.string() == "-")
    return once(std::cin, total_line, file_size);

  std::ifstream file(filepath);
  size_t ret = once(file, total_line, file_size);
  file.close();
  return ret;
}

void tail::run(std::vector<std::filesystem::path> p_filepaths)
{
  // It is non sense asking to tail a file after its last line, in this case defaulting to 10 last lines
  if (start_line == 0)
    start_line = 10;

  if (p_filepaths.empty())
  {
    if (filepaths.empty())
      return;
  }
  else
    filepaths = p_filepaths;

  std::vector<size_t> total_lines(filepaths.size());
  std::vector<size_t> file_sizes(filepaths.size());
  std::vector<size_t> new_file_sizes(filepaths.size());
  std::string last_filename = {};

  // First tail of the provided file(s)
  for (size_t i = 0; i < filepaths.size(); i++)
  {
    if (filepaths.size() > 1)
    {
      std::string filetitle = filepaths[i].string();
      if (filetitle == "-")
      {
        filetitle = "STDIN";
      }

      if (!quiet)
        std::cout << "==> " << filetitle << " <==" << std::endl;
    }

    file_sizes[i] = once(filepaths[i], total_lines[i]);
    // std::cout << "== " << file_sizes[i] << " ==" << std::endl;
    // if (last_filename.empty() && filepaths[i].string() != "-")
    last_filename = filepaths[i].string();
  }

  // If required to follow the file(s) continuously
  if (follow)
  {
    // Entering an endless loop
    for (;;)
    {
      for (size_t i = 0; i < filepaths.size(); i++)
      {
        // Retrieving a 'stable' the file size
        new_file_sizes[i] = stable_file_size(filepaths[i]);

        // File size has changed
        if (new_file_sizes[i] != file_sizes[i])
        {
          // The file is growing
          if (new_file_sizes[i] > file_sizes[i])
          {
            if (last_filename != filepaths[i].string())
            {
              std::string filetitle = last_filename = filepaths[i].string();
              if (filetitle == "-")
                filetitle = "STDIN";
              if (!quiet)
                std::cout << "==> " << filetitle << " <==" << std::endl;
            }

            file_sizes[i] = from_pos(filepaths[i], total_lines[i], file_sizes[i]);
            if (line_numbers)
              total_lines[i]++;
          }
          // The file is shrinking
          else if (new_file_sizes[i] < file_sizes[i])
          {
            if (filepaths.size() > 1)
              std::cout << std::endl;

            std::string filetitle = last_filename = filepaths[i].string();
            if (filetitle == "-")
            {
              std::cout << "tail: the STDIN";
            }
            else
              std::cout << "tail: the file '" << filetitle << "'";

            std::cout << " has been truncated, displaying its last " << start_line << " lines." << std::endl;
            total_lines[i] = 0;
            file_sizes[i] = once(filepaths[i], total_lines[i]);
          }

          file_sizes[i] = new_file_sizes[i];
        }
      }
    }
  }
}

// The main feature interface function
tail::tail(std::vector<std::filesystem::path> p_filepaths)
{
  run(p_filepaths);
}

// To remove the last line from a file without reading the whole file or rewriting anything
// file=syslog;  tail -n 1 "$file" | wc -c | xargs -I {} truncate "$file" -s -{}

std::ostream &operator<<(std::ostream &os, const tail &t)
{
  os << "follow: " << t.follow << ", " << "line_numbers: " << t.line_numbers << ", " << "start_line: " << t.start_line << ", " << "seek_step: " << t.seek_step << std::endl;
  os << "File(s): ";
  for (auto &filepath : t.filepaths)
  {
    os << filepath;
    if (&filepath != &t.filepaths.back())
      os << ", ";
  }

  os << std::endl;
  return os;
}
