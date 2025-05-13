
#ifndef MYTAIL_H
#define MYTAIL_H

size_t linecount(std::ifstream &file);
size_t linecount(const std::string filename);
size_t tail_from_pos(std::ifstream &file, size_t pos, size_t total_line, size_t start_line = 0, bool forward = true);
size_t tail_from_pos(std::string filename, size_t pos, size_t total_line = 0, size_t start_line = 0, bool forward = true);
size_t tail_once(std::string filename, size_t &total_line, bool num, const size_t start_line, size_t seek_step);
size_t tail(std::string filename, bool poll = false, bool num = false, const size_t start_line = 10, size_t seek_step = 0);

#endif /* MYTAIL_H */
