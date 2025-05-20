
#ifndef MYTAIL_H
#define MYTAIL_H

class tail
{
public:
  tail(std::vector<std::filesystem::path> filepaths = {});
  void run(std::vector<std::filesystem::path> filepaths = {});

  std::vector<std::filesystem::path> filepaths = {};
  bool poll = false, num = false;
  size_t start_line = 10, seek_step = 0;

private:
  size_t once(std::filesystem::path, size_t &);
  size_t from_pos(std::ifstream &file, size_t, size_t, bool forward = true);
  size_t from_pos(std::filesystem::path, size_t, size_t, bool forward = true);
  std::uintmax_t stable_file_size(std::filesystem::path filepath, const int nloop = 10, const int dur = 200);
  size_t linecount(std::ifstream &);
  size_t linecount(std::filesystem::path);
  size_t clinecount(std::filesystem::path);

  friend std::ostream &operator<<(std::ostream &, const tail &);
};

#endif /* MYTAIL_H */
