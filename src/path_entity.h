
#ifndef PATH_ENTITY_H
#define PATH_ENTITY_H
//
// Conversion facilities between C++ path representation and html entities
//

// Convert string to u32string
std::u32string s2u32s(const std::string s);
// Convert u32string to string
std::string u32s2s(const std::u32string u32s);

// Convert string to u16string
std::u16string s2u16s(const std::string s);
// Convert u16string to string
std::string u16s2s(const std::u16string u16s);

// Return true if char is a printable ascii
bool isprintable_ascii(unsigned char);
bool isprintable_ascii(char16_t);
bool isprintable_ascii(wchar_t);
bool isprintable_ascii(char32_t);

// Convert long long to its hexadecimal string representation
std::string to_hex_string(long long);

// Decode eventual html entities within a string and convert to u32string
std::u32string htent_to_u32(std::string s);
// Encode to html entities all the eventual non printable char, char with code above 127 and the backslash contained within a u32string (default to decimal entities). And convert to a string
std::string u32_to_htent(std::u32string u32s, bool dec_base = true);

// Convert path to html entities
std::filesystem::path htent_to_path(std::string s);
// Convert html entities to path
std::string path_to_htent(std::filesystem::path p, bool dec = true);

#endif /* PATH_ENTITY_H */
