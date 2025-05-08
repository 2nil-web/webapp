
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

unsigned int linecount(std::fstream& file) {
  return std::count_if(std::istreambuf_iterator<char>{file}, {}, [](char c) { return c == '\n'; });
}

unsigned int linecount(std::string filename) {
  std::fstream file(filename);
  unsigned int count = linecount(file);
  file.close();
  return count;
}

std::fstream& toline(std::fstream& file, unsigned int num){
  file.seekg(std::ios::beg);
  for (unsigned int i=0; i < num - 1; ++i){
    file.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
  }
  return file;
}

std::string cat(std::fstream& file, unsigned int n=0) {
  toline(file, n);
  std::string s, l;
  while (getline(file, l)) {
    s+=std::to_string(n)+':'+l+'\n';
    n++;
  }

  return s;
}

std::string cat(std::string filename, unsigned int n=0) {
  std::fstream file(filename);
  std::string s=cat(file, n);
  file.close();
  return s;
}


int main(int argc, char **argv) {
  switch (argc) {
    case 2:
      std::cout << linecount(argv[1]) << std::endl;
      break;
    case 3:
      std::cout << cat(argv[1], std::stol(argv[2]));
      break;
    default:
      break;
  }

  return 0;
}

