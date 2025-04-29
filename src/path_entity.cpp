
#ifdef _WIN32
// clang-format off
#include <tchar.h>
#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#ifndef __unix__
#include <versionhelpers.h>
#endif
// clang-format on
#else
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#endif

#include <algorithm>
#include <cctype>
#include <chrono>
#include <codecvt>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <locale>
#include <map>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "log.h"
#include "path_entity.h"

/*
  +--------------------+--------------------+-------------------+-------+
 /      WIN MSVC      / \    WIN GCC       / \    LIN GCC       |        \
 +-+------------------+-+------------------+-+------------------+--------+
 |1|bool              |1|bool              |1|bool              |        |
 |1|char              |1|char              |1|char              |        |
 |1|signed char       |1|signed char       |1|signed char       |        |
 |1|unsigned char     |1|unsigned char     |1|unsigned char     |char8_t |
 |2|char16_t          |2|char16_t          |2|char16_t          |        |
 |2|short             |2|short             |2|short             |        |
 |2|unsigned short    |2|unsigned short    |2|unsigned short    |char16_t|
 |4|char32_t          |4|char32_t          |4|char32_t          |        |
 |4|float             |4|float             |4|float             |        |
 |4|int               |4|int               |4|int               |        |
 |4|unsigned int      |4|unsigned int      |4|unsigned int      |char32_t|
 |8|double            |8|double            |8|double            |        |
 |8|long long         |8|long long         |8|long long         |        |
 |8|nullptr           |8|nullptr           |8|nullptr           |        |
 |8|unsigned long long|8|unsigned long long|8|unsigned long long|char64_t|
 +-+------------------+-+------------------+-+------------------+--------+

Consider improvement to 'Flexible String Representation', whose algorithm might be something close to the following postulate.
If no character in a string has an ordinal greater than one of the values listed below then encode to their corresponding char type :
 1)          255    (      FF) ==> char8_t
 2)        65535    (    FFFF) ==> char16_t
 3)     16777215    (  FFFFFF) ==> char24_t (!) Needs the creation of char24_t (*1) May not be more efficient than using char32_t
 4)   4294967295U   (FFFFFFFF) ==> char32_t
 5) 18446744073709551615ULL (FFFFFFFFFFFFFFFF) ==>  (!) Needs the creation of char64_t (!) really useful ? May be to encode all the semiosis of the universe ...

(*1) See https://cplusplus.com/forum/general/196494/  and run there https://cpp.sh
Point 3) and 5) are optionals ...
*/

std::u32string s2u32s(const std::string s)
{
  std::u32string s32 = {};
  for (unsigned char c : s)
  {
    s32 += c;
  }
  return s32;
  //  return std::u32string(s.begin(), s.end());
}

std::string u32s2s(const std::u32string u32s)
{
#if _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4244)
#endif
  return std::string(u32s.begin(), u32s.end());
#if _MSC_VER
#pragma warning(pop)
#endif
}

bool isprintable_ascii(wchar_t wc)
{
  if (wc > 31 && wc < 127)
    return true;
  return false;
}

bool isprintable_ascii(unsigned char c)
{
  if (c > 31 && c < 127)
    return true;
  return false;
}

bool isprintable_ascii(char32_t c)
{
  if (c > 31 && c < 127 && c != '\\')
    return true;
  return false;
}

std::string to_hex_string_C(unsigned int hi)
{
  size_t l = 4 * sizeof(unsigned int) + 1;
  char *hs = new char[l];

  snprintf(hs, l, "%x", hi);
  std::string s(hs);
  delete[] hs;
  return s;
}

std::string to_hex_string(unsigned int hi)
{
  std::stringstream stream;
  stream << std::hex << hi;
  return stream.str();
}

// Fonction pour décoder les entités HTML numériques éventuellement contenues dans un string et convertir en u32string
std::u32string htent_to_u32(std::string s)
{
  std::u32string u32s = s2u32s(s);
  std::regex decPatt("&#(\\d+);");
  std::regex hexPatt("&#x([0-9A-Fa-f]+);");
  std::smatch sm;

  while (std::regex_search(s, sm, decPatt))
  {
    unsigned int charCode = std::stoi(sm[1].str());
    char32_t decodedChar = static_cast<char32_t>(charCode);
    s.replace(sm.position(0), sm.length(0), 1, '*');
    u32s.replace(sm.position(0), sm.length(0), 1, decodedChar);
  }

  while (std::regex_search(s, sm, hexPatt))
  {
    unsigned int charCode = std::stoi(sm[1].str(), nullptr, 16);
    char32_t decodedChar = static_cast<char32_t>(charCode);
    s.replace(sm.position(0), sm.length(0), 1, '*');
    u32s.replace(sm.position(0), sm.length(0), 1, decodedChar);
  }

  return u32s;
}

// Fonction pour encoder les non ascii contenus dans du u32string en entites html et convertir le u32string en string
std::string u32_to_htent(std::u32string u32s, bool dec_base)
{
  std::string s = {};

  size_t i = 0;
  for (auto c : u32s)
  {
    if (isprintable_ascii(c))
      s += static_cast<char>(c);
    else
    {
      s += std::string("&#");

      if (dec_base && (char32_t)c < 1000000)
      {
        s += std::to_string((char32_t)c);
      }
      else
        s += to_hex_string((char32_t)c);

      s += ';';
    }
    i++;
  }

  return s;
}

// Convert path to html entities
std::filesystem::path htent_to_path(std::string s)
{
  return std::filesystem::path(htent_to_u32(s));
}

// Convert html entities to path
std::string path_to_htent(std::filesystem::path p, bool dec)
{
  return u32_to_htent(p.u32string(), dec);
}
