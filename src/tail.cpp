
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

#include "tail.h"

#ifdef __OSX__
const char newline = '\r';
#else
const char newline = '\n';
#endif

// clear to eol in pure C/C++
inline void clear_line(size_t width = 80)
{
  std::cout << '\r' << std::string(width, ' ') << '\r' << std::flush;
}

// Tries to assert that a file is not currently being written.
// The strategy here is to make
//    a) A maximum of 'nloop' calls (default 10) to std::filesystem::file_size
//    b) With a pause of 'dur' milliseconds (default 200) between each
// And the hope is to retrieve twice the same size in a row leading to conclude that
//    a) The file size is 'stable'
//    b) Hence not being written
std::uintmax_t tail::stable_file_size(std::filesystem::path filepath, const int nloop, const int dur)
{
  std::uintmax_t prev_fs, curr_fs;
  std::error_code ec;

  // Initial retrieval of the file size
  curr_fs = std::filesystem::file_size(filepath, ec);

  // Loop hoping to find curr_fs and prev_fs equal
  for (int i = 0; i < nloop; i++)
  {
    // A little sleep
    std::this_thread::sleep_for(std::chrono::milliseconds(dur));

    // Then retrieval of the file size
    prev_fs = curr_fs;
    curr_fs = std::filesystem::file_size(filepath, ec);
    if (ec.value() != 0)
      continue;
    // We found a 'stable' file size meaning/hoping that the file is not actually being written
    if (prev_fs == curr_fs)
      break;
  }

  return curr_fs;
}

// Get an ifstream lines count
inline size_t tail::linecount(std::ifstream &file)
{
  return std::count_if(std::istreambuf_iterator<char>{file}, {}, [](char c) { return c == newline; });
  // return std::count(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(), newline);
}

// Get a file path lines count
size_t tail::linecount(std::filesystem::path filepath)
{
  size_t count = 0;

  std::ifstream file(filepath);

  if (file)
  {
    count = linecount(file);
    file.close();
  }

  return count;
}

// From the quick test I did, counting lines in pure C is much faster than in C++
// It is around 2 to 3 times faster with gcc/C versus gcc/C++ (Windows and Linux)
// And is around 10(!) times faster with msvc/C versus msvc/C++ (Windows only)
// Just keeping the C++ way in case the C way fails
inline size_t tail::clinecount(std::filesystem::path filepath)
{
  FILE *file = nullptr;

#ifdef _WIN32
  fopen_s(&file, filepath.string().c_str(), "rb");
#else
  file = fopen(filepath.string().c_str(), "rb");
#endif

  if (file == nullptr)
    return 0;

  fseek(file, 0, SEEK_END);
  size_t fsize = ftell(file);
  char *s = new char[fsize + 1];
  fseek(file, 0, SEEK_SET);
  if (fread(s, fsize, 1, file) != 1)
  {
    fclose(file);
    // If pure C fails then tries by falling back to C++ way ...
    return linecount(filepath);
  }

  fclose(file);
  char *pl = s, *p = s;
  size_t count = 0;

  // The 'do while' that nest the 'while' is here to avoid strchr from being fooled by eventual null characters that might be in the file opened in binary mode
  do
  {
    p = pl;
    while ((p = strchr(p, newline)))
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

// Tail an ifstream from a specific position
size_t tail::from_pos(std::ifstream &file, size_t total_line, size_t pos, bool forward)
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
    if (num)
    {
      // Get the widdest line number
      size_t ln_width = std::to_string(1 + positioned_lines.back().s.size()).size();
      // Set the the first line number of the deque

      size_t line_number;
      if (total_line < start_line)
        line_number = 1;
      else
        line_number = (1 + total_line - start_line); // positioned_lines.back().s.size();

      // std::cout << "total_line: " << total_line << ", start_line: " << start_line << std::endl;
      //  Display the numbered positioned_lines
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
  std::ifstream file(filepath);
  size_t file_size = from_pos(file, total_line, pos, forward);
  file.close();
  return file_size;
}

// Tail a file at the character position corresponding to the provided start_line
size_t tail::once(std::filesystem::path filepath, size_t &total_line)
{
  // First line number is 1, and zero is assumed as the 'after last line number', then it is non sense asking to tail a file after its last line
  if (start_line == 0)
    return 0;

  // Computing the number of lines, if needed
  if (num && total_line == 0)
  {
    const std::string msg = "Computing line count, wait ...";
    std::cout << msg << std::flush;
    total_line = clinecount(filepath);
    clear_line(msg.size());
  }

  // Get file size
  std::error_code ec;
  size_t file_size = std::filesystem::file_size(filepath, ec);
  // If problem reading file size or file is empty, then aborting ...
  if (ec.value() != 0 || file_size == 0)
    return 0;

  std::ifstream file(filepath);

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
  //	std::cout << "file: " << file << std::endl;
  //	std::cout << "total_line: " << total_line << std::endl;
  //	std::cout << "pos: " << pos << std::endl;

  size_t ret = from_pos(file, total_line, pos, false);
  file.close();
  return ret;
}

void tail::run(std::vector<std::filesystem::path> p_filepaths)
{
  if (p_filepaths.empty())
  {
    if (filepaths.empty())
      return;
  }
  else
    filepaths = p_filepaths;

  std::vector<size_t> total_lines(filepaths.size());
  std::vector<size_t> file_sizes(filepaths.size());
  std::string last_filename = {};

  // First tail of the provided file(s)
  for (size_t i = 0; i < filepaths.size(); i++)
  {
    if (filepaths.size() > 1)
      std::cout << "==> " << filepaths[i] << " <==" << std::endl;
    file_sizes[i] = once(filepaths[i], total_lines[i]);
    // if (filepaths.size() > 1 && i < filepaths.size() - 1) std::cout << std::endl;
    last_filename = filepaths[i].string();
  }

  // If required to poll the file(s) continuously
  if (poll)
  {
    size_t new_file_size;

    // Entering an endless loop
    for (;;)
    {
      for (size_t i = 0; i < filepaths.size(); i++)
      {
        // Retrieving a 'stable' the file size
        new_file_size = stable_file_size(filepaths[i]);

        // The file is growing
        if (new_file_size > file_sizes[i])
        {
          if (filepaths.size() > 1)
          {
            if (last_filename != filepaths[i].string())
            {
              last_filename = filepaths[i].string();
              std::cout << "==> " << last_filename << " <==" << std::endl;
            }
          }

          if (num)
          {
            file_sizes[i] = from_pos(filepaths[i], total_lines[i], file_sizes[i]);
            total_lines[i]++;
          }
          else
          {
            file_sizes[i] = from_pos(filepaths[i], total_lines[i], file_sizes[i]);
          }

          // if (filepaths.size() > 1 && i < filepaths.size() - 1) std::cout << std::endl;
        }
        else if (new_file_size < file_sizes[i])
        { // The file is shrinking
          if (filepaths.size() > 1)
            std::cout << std::endl;
          std::cout << "tail: the file '" << filepaths[i].string() << "' has been truncated, displaying its last " << start_line << " lines." << std::endl;
          total_lines[i] = 0;
          file_sizes[i] = once(filepaths[i], total_lines[i]);
        }
      }
    }
  }
}

// The main feature interface function
tail::tail(std::vector<std::filesystem::path> filepaths)
{
  run(filepaths);
}

/* To remove the last line from a file without reading the whole file or rewriting anything
file=syslog
tail -n 1 "$file" | wc -c | xargs -I {} truncate "$file" -s -{}
*/

std::ostream &operator<<(std::ostream &os, const tail &t)
{
  os << "poll: " << t.poll << ", " << "num: " << t.num << ", " << "start_line: " << t.start_line << ", " << "seek_step: " << t.seek_step << std::endl;
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
