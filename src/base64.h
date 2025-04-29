#ifndef BASE64_H
#define BASE64_H

#include <cstdint>
#include <string>
#include <vector>

using byte = std::uint8_t;
std::string to_base64(const std::vector<byte> &);
std::vector<byte> from_base64(const std::string &);

#endif /* BASHE_64-H */
