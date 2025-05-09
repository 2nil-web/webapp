
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

size_t linecount(std::fstream& file) {
  return std::count_if(std::istreambuf_iterator<char>{file}, {}, [](char c) { return c == '\n'; });
}

size_t linecount(std::string filename) {
  std::fstream file(filename);
  size_t count = linecount(file);
  file.close();
  return count;
}

size_t toline(std::fstream& file, size_t num){
  file.seekg(std::ios::beg);
  size_t i=0;
  while (i < num) {
    file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
    i++;
  }

  return i;
}

struct sCatResult {
  size_t start_line, start_char, end_char;
  std::vector<std::string> lines;
};

sCatResult cat(std::fstream& file, int n=0) {
  if (n < 0) {
      int ln=linecount(file);
      if (ln > -n) n=ln-1+n;
      else n=0;
  }

  sCatResult res;
  res.start_line=toline(file, n);
  res.start_char=file.tellg();
  res.lines={};
  std::string l;
  while (getline(file, l, '\n')) {
    res.lines.push_back(l);
  }

  res.end_char=file.tellg();
  return res;
}

std::string cat(std::string filename, int n=0, bool only_after=false) {
  std::fstream file(filename);
  sCatResult res=cat(file, n);
  file.close();

  std::string s={};
  size_t i=0;

  if (only_after && n > (int)res.start_line) {
    i=n-res.start_line;
  }

  while (i < res.lines.size()) {
    s+=std::to_string(res.start_line+i)+':'+res.lines[i]+'\n';
    i++;
  }

  if (only_after) s+=std::to_string(res.start_line+i);
  else s.pop_back();

  return s;
}


int main(int argc, char **argv) {
  switch (argc) {
    case 2:
      std::cout << linecount(argv[1]) << std::endl;
      break;
    case 3:
      std::cout << cat(argv[1], std::stol(argv[2])) << std::endl;
      break;
    case 4:
      std::cout << cat(argv[1], std::stol(argv[2]), true) << std::endl;
      break;
    default:
      break;
  }

  return 0;
}

