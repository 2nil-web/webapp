
#ifndef MYTAIL_H
#define MYTAIL_H

#ifdef __OSX__
const char newline = '\r';
#else
const char newline = '\n';
#endif

class tail
{
public:
  std::vector<std::filesystem::path> filepaths = {};
  bool follow = false, line_numbers = false, quiet = false;
  size_t start_line = 0, seek_step = 0;
  char delimiter = newline;

  tail() {};
  tail(std::vector<std::filesystem::path> filepaths);

  void run(std::vector<std::filesystem::path> filepaths = {});

private:
  size_t once(std::filesystem::path, size_t &);
  size_t once(std::istream &, size_t &, size_t);

  size_t from_pos(std::filesystem::path, size_t, size_t, bool forward = true);
  size_t from_pos(std::istream &is, size_t, size_t, bool forward = true);

  size_t clinecount(std::filesystem::path, size_t &);
  size_t clinecount(FILE *, size_t &);

  size_t linecount(std::filesystem::path);
  size_t linecount_from_current_pos(std::istream &);

  friend std::ostream &operator<<(std::ostream &, const tail &);
};

#endif /* MYTAIL_H */
