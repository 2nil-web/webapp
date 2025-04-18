
// #define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

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
#include <cstdint>

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
#endif

#include "path_entity.h"

#ifdef _WIN32
#include "winapi.h"
#endif
#include "wrap.h"
#include "log.h"
#include "util.h"

// For lack of starts_with in gcc < 13
bool starts_with(std::string str, std::string prefix)
{
#if __GNUC__ > 13 || defined(_MSC_VER)
  return str.starts_with(prefix);
#else
  if (prefix.size() > str.size())
    return false;
  return str.compare(0, prefix.size(), prefix) == 0;
#endif
}

static const wchar_t CP1252_UNICODE_TABLE[] = L"\u0000\u0001\u0002\u0003\u0004\u0005\u0006\u0007"
                                              L"\u0008\u0009\u000A\u000B\u000C\u000D\u000E\u000F"
                                              L"\u0010\u0011\u0012\u0013\u0014\u0015\u0016\u0017"
                                              L"\u0018\u0019\u001A\u001B\u001C\u001D\u001E\u001F"
                                              L"\u0020\u0021\u0022\u0023\u0024\u0025\u0026\u0027"
                                              L"\u0028\u0029\u002A\u002B\u002C\u002D\u002E\u002F"
                                              L"\u0030\u0031\u0032\u0033\u0034\u0035\u0036\u0037"
                                              L"\u0038\u0039\u003A\u003B\u003C\u003D\u003E\u003F"
                                              L"\u0040\u0041\u0042\u0043\u0044\u0045\u0046\u0047"
                                              L"\u0048\u0049\u004A\u004B\u004C\u004D\u004E\u004F"
                                              L"\u0050\u0051\u0052\u0053\u0054\u0055\u0056\u0057"
                                              L"\u0058\u0059\u005A\u005B\u005C\u005D\u005E\u005F"
                                              L"\u0060\u0061\u0062\u0063\u0064\u0065\u0066\u0067"
                                              L"\u0068\u0069\u006A\u006B\u006C\u006D\u006E\u006F"
                                              L"\u0070\u0071\u0072\u0073\u0074\u0075\u0076\u0077"
                                              L"\u0078\u0079\u007A\u007B\u007C\u007D\u007E\u007F"
                                              L"\u20AC\u0020\u201A\u0192\u201E\u2026\u2020\u2021"
                                              L"\u02C6\u2030\u0160\u2039\u0152\u0020\u017D\u0020"
                                              L"\u0020\u2018\u2019\u201C\u201D\u2022\u2013\u2014"
                                              L"\u02DC\u2122\u0161\u203A\u0153\u0020\u017E\u0178"
                                              L"\u00A0\u00A1\u00A2\u00A3\u00A4\u00A5\u00A6\u00A7"
                                              L"\u00A8\u00A9\u00AA\u00AB\u00AC\u00AD\u00AE\u00AF"
                                              L"\u00B0\u00B1\u00B2\u00B3\u00B4\u00B5\u00B6\u00B7"
                                              L"\u00B8\u00B9\u00BA\u00BB\u00BC\u00BD\u00BE\u00BF"
                                              L"\u00C0\u00C1\u00C2\u00C3\u00C4\u00C5\u00C6\u00C7"
                                              L"\u00C8\u00C9\u00CA\u00CB\u00CC\u00CD\u00CE\u00CF"
                                              L"\u00D0\u00D1\u00D2\u00D3\u00D4\u00D5\u00D6\u00D7"
                                              L"\u00D8\u00D9\u00DA\u00DB\u00DC\u00DD\u00DE\u00DF"
                                              L"\u00E0\u00E1\u00E2\u00E3\u00E4\u00E5\u00E6\u00E7"
                                              L"\u00E8\u00E9\u00EA\u00EB\u00EC\u00ED\u00EE\u00EF"
                                              L"\u00F0\u00F1\u00F2\u00F3\u00F4\u00F5\u00F6\u00F7"
                                              L"\u00F8\u00F9\u00FA\u00FB\u00FC\u00FD\u00FE\u00FF";

std::string decodeURIComponent(std::string encoded)
{

  std::string decoded = encoded;
  std::smatch sm;
  std::string haystack;

  size_t dynamicLength = decoded.size() - 2;

  if (decoded.size() < 3)
    return decoded;

  for (size_t i = 0; i < dynamicLength; i++)
  {

    haystack = decoded.substr(i, 3);

    if (std::regex_match(haystack, sm, std::regex("%[0-9A-F]{2}")))
    {
      haystack = haystack.replace(0, 1, "0x");
      std::string rc = {(char)std::stoi(haystack, nullptr, 16)};
      decoded = decoded.replace(decoded.begin() + i, decoded.begin() + i + 3, rc);
    }

    dynamicLength = decoded.size() - 2;
  }

  return decoded;
}

std::string encodeURIComponent(std::string decoded)
{
  std::ostringstream oss;
  std::regex r("[!'\\(\\)*-.0-9A-Za-z_~]");
  for (char &c : decoded)
  {
    if (std::regex_match(std::string(1, c), r))
      oss << c;
    else
      oss << "%" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << (int)(c & 0xff);
  }
  return oss.str();
}

std::string encodeURIComponent(std::wstring wdecoded)
{
  return encodeURIComponent(ws2s(wdecoded));
}

// Get env var
// Return true if exists else empty string
std::string my_getenv(const std::string var, bool msg_if_empty)
{
  char *pVal = nullptr;
  std::string sVal = "";
  if (msg_if_empty)
    sVal = "No value found for " + var;

#ifdef _WIN32
  DWORD vl = GetEnvironmentVariable(var.c_str(), NULL, 0);
  if (vl > 0)
  {
    pVal = new char[vl + 1];
    GetEnvironmentVariable(var.c_str(), pVal, vl);
    if (pVal != nullptr)
    {
      sVal = pVal;
      delete[] pVal;
    }
  }
#else
  pVal = getenv(var.c_str());
  if (pVal != nullptr)
    sVal = pVal;
#endif

  return sVal;
}

// Set env var
// Return true if OK else false
bool my_setenv(const std::string var, const std::string val)
{
#ifdef _WIN32
  return SetEnvironmentVariable(var.c_str(), val.c_str()) != 0;
#else
  return setenv((char *)var.c_str(), (char *)val.c_str(), 1) == 0;
#endif
}

// Insert val at the start of env var
bool my_insenv(const std::string var, const std::string val)
{
  std::string vval = my_getenv(var);
  vval = val + vval;
  return my_setenv(var, vval);
}

// Add val at the end of env var
bool my_addenv(const std::string var, const std::string val)
{
  std::string vval = my_getenv(var);
  vval += val;
  return my_setenv(var, vval);
}

// Delete env var
bool my_delenv(const std::string var)
{

#ifdef _WIN32
  return SetEnvironmentVariable(var.c_str(), nullptr) != 0;
#else
  return (unsetenv((char *)var.c_str()) == 0);
#endif
}

// return if string == true/ok/yes/1 else false
bool str2bool(std::string s)
{
  if (s.empty() || s == "false" || s == "ko" || s == "0" || s == "no" || s == "nok" || s == "not" || s == "disagree" || s == "vary" || s == "discord" || s == "disaccord")
  {
    return false;
  }

  if (s == "true" || s == "ok" || s == "1" || s == "yes" || s == "assent" || s == "consent" || s == "accede" || s == "support" || s == "agree" || s == "approve")
  {
    return true;
  }

  logDebug("Cannot convert '", s, "' to boolean value assuming false.");
  return false;
}

// Return true if found ndl in hstck, ignoring case
bool ifind(const std::string &hstck, const std::string &ndl)
{
  auto it = std::search(hstck.begin(), hstck.end(), ndl.begin(), ndl.end(), [](unsigned char ch1, unsigned char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
  return (it != hstck.end());
}

bool any_of_ctype(const std::string s, std::function<int(int)> istype)
{
  return std::any_of(s.begin(), s.end(), [istype](char c) { return istype(c); });
}

bool all_of_ctype(const std::string s, std::function<int(int)> istype)
{
  return std::all_of(s.begin(), s.end(), [istype](char c) { return istype(c); });
}

std::vector<std::string> split(const std::string &str, const char delim)
{
  std::vector<std::string> strings;
  size_t start;
  size_t end = 0;
  while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
  {
    end = str.find(delim, start);
    strings.push_back(str.substr(start, end - start));
  }
  return strings;
}

std::vector<std::string> split_esc(const std::string &s, const char delim, const char esc)
{
  size_t l = s.size();
  std::string rs = {};
  std::vector<std::string> vs;

  for (size_t i = 0; i < l; i++)
  {
    if (s[i] == delim)
    {
      if (i > 0 && s[i - 1] == esc)
      {
        rs.pop_back();
        rs.push_back(delim);
      }
      else
      {
        vs.push_back(rs);
        rs = {};
      }
    }
    else
      rs.push_back(s[i]);
  }

  if (rs.size() > 0)
    vs.push_back(rs);

  return vs;
}

std::string ltrim(std::string &s, const std::string ws)
{
  s.erase(0, s.find_first_not_of(ws));
  return s;
}

std::string rtrim(std::string &s, const std::string ws)
{
  s.erase(s.find_last_not_of(ws) + 1);
  return s;
}

std::string trim(std::string &s, const std::string ws)
{
  s.erase(0, s.find_first_not_of(ws));
  s.erase(s.find_last_not_of(ws) + 1);
  return s;
}

std::wstring trim(std::wstring &s, const std::wstring wss)
{
  s.erase(0, s.find_first_not_of(wss));
  s.erase(s.find_last_not_of(wss) + 1);
  return s;
}

std::string ctrim(std::string s, const std::string ws)
{
  s.erase(0, s.find_first_not_of(ws));
  s.erase(s.find_last_not_of(ws) + 1);
  return s;
}

std::wstring ctrim(std::wstring s, const std::wstring wss)
{
  s.erase(0, s.find_first_not_of(wss));
  s.erase(s.find_last_not_of(wss) + 1);
  return s;
}

std::string to_upper(const std::string cs)
{
  std::string s = cs;
  std::transform(s.begin(), s.end(), s.begin(), ::toupper);
  return s;
}

std::string file2s(std::filesystem::path p)
{
  std::string s;
  std::ifstream ifs(p);

  if (ifs.good())
  {
    s = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    ifs.close();
  }
  else
    s = "Unable to open the file '" + p.string() + "'";

  // logTrace("LEN: ", s.length());
  if (s.length() == 0)
    s = "File " + p.string() + " is empty.";
  return s;
}

bool file2bin(std::filesystem::path p, std::vector<std::uint8_t> &buffer)
{
  std::ifstream input(p, std::ios::binary);
  if (input.good())
  {
    std::vector<std::uint8_t> buffer1(std::istreambuf_iterator<char>(input), {});
    buffer = buffer1;
    input.close();
    return true;
  }
  return false;
}

bool bin2file(std::filesystem::path p, std::vector<std::uint8_t> &buffer)
{
  std::ofstream output(p, std::ios::binary);
  if (output.good())
  {
    output.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    output.close();
    return true;
  }
  return false;
}

std::wstring file2ws(std::filesystem::path p)
{
  std::ifstream ifs(p);
  std::wstring ws;

  if (ifs.good())
  {
    std::wstring ws = std::wstring((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    ifs.close();
  }
  else
    ws = L"Unable to open the file '" + p.wstring() + L"'";

  // logTrace("LEN: ", ws.length());
  if (ws.length() == 0)
    ws = L"File " + p.wstring() + L" is empty.";
  return ws;
}

std::filesystem::path file2p(std::filesystem::path p)
{
  std::string s;
  std::ifstream ifs(p);

  if (ifs.good())
  {
    try
    {
      s = std::string((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
    }
    catch (std::filesystem::filesystem_error const &ex)
    {
      logError("what():  ", ex.what());
      logError("path1(): ", ex.path1());
      logError("path2(): ", ex.path2());
      logError("code().value():    ", ex.code().value());
      logError("code().message():  ", ex.code().message());
      logError("code().category(): ", ex.code().category().name());
      s = "Problem when reading the file '" + p.string() + "': " + ex.what();
    }

    ifs.close();
  }
  else
    s = "Unable to open the file '" + p.string() + "'";

  return std::filesystem::path(s);
}

std::string join(const std::vector<std::string> &vec, const std::string delim)
{
  std::stringstream res;
  std::copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(res, delim.c_str()));
  return res.str();
}

std::vector<std::string> split(std::string str, std::string delimiters)
{
  std::vector<std::string> v;
  if (!str.empty())
  {
    size_t start = 0;
    do
    {
      // Find the index of occurrence
      size_t idx = str.find(delimiters, start);
      if (idx == std::string::npos)
      {
        break;
      }

      // If found add the substring till that occurrence in the vector
      size_t length = idx - start;
      v.push_back(str.substr(start, length));
      start += (length + delimiters.size());
    } while (true);
    v.push_back(str.substr(start));
  }

  return v;
}

void replace_all(std::string &s, const std::string srch, const std::string repl)
{
  size_t pos = 0, retpos;
  for (;;)
  {
    retpos = s.find(srch, pos);
    if (retpos == std::string::npos)
      break;
    s.replace(retpos, srch.length(), repl);
    pos = retpos + repl.length();
  }
}

std::string creplace_all(const std::string cs, const std::string srch, const std::string repl)
{
  std::string s = cs;
  replace_all(s, srch, repl);
  return s;
}

void replace_all(std::wstring &s, const std::wstring srch, const std::wstring repl)
{
  size_t pos = 0, retpos;
  for (;;)
  {
    retpos = s.find(srch, pos);
    if (retpos == std::string::npos)
      break;
    s.replace(retpos, srch.length(), repl);
    pos = retpos + repl.length();
  }
}

std::wstring creplace_all(const std::wstring cws, const std::wstring srch, const std::wstring repl)
{
  std::wstring ws = cws;
  replace_all(ws, srch, repl);
  return ws;
}

std::string rep_crlf(std::string s)
{
  replace_all(s, "\r", "\\r");
  replace_all(s, "\n", "\\n");
  return s;
}

std::string temppath()
{
  std::string tpath = "";
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
  tpath = std::filesystem::temp_directory_path().generic_string();
#else
#ifdef _WIN32
  char tpw[MAX_PATH - 14];
  if (GetTempPath(MAX_PATH - 14, tpw) != 0)
    tpath = std::string(tpw);
#else
  tpath = "/tmp";
#endif
#endif
  return tpath;
}

std::string tempfile(std::string tpath, std::string pfx)
{
  std::string tfn = "";

  if (tpath.empty())
    tpath = temppath();
  if (pfx.empty())
    pfx = "tmp.XXXXXX";
#ifdef _WIN32
  replace_all(tpath, "/", "\\");
  char tfnw[MAX_PATH];
  if (GetTempFileName(tpath.c_str(), pfx.c_str(), 0, tfnw) != 0)
    tfn = tfnw;
#else
  tfn = std::string((tpath + '/' + pfx).c_str());
  char *stfn = strdup(tfn.c_str());
  int id = mkstemp(stfn);

  if (id != -1)
  {
    close(id);
    unlink(stfn);
    tfn = stfn;
  }
#endif

  return std::string(tfn);
}
#ifdef PASFAIRE // defined(_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING) || defined(__GNUG__)
std::string ws2s(std::wstring ws)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(ws);
}
std::wstring s2ws(std::string s)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(s);
}
#ifdef WIN32
std::string ws2s(const std::wstring ws)
{
  size_t lw = sizeof(wchar_t) * ws.length();
  char *pch = new char[lw + 1];
  pch[lw] = '\0';
  WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, pch, (int)lw, NULL, NULL);
  std::string s(pch);
  delete[] pch;
  return s;
}

std::wstring s2ws(const std::string s)
{
  // Convert an string to a utf8 wstring
  size_t l = sizeof(char) * (s.length());
  wchar_t *pwch = new wchar_t[l + 1];
  pwch[l] = L'\0';
  MultiByteToWideChar(CP_ACP, 0, s.c_str(), -1, pwch, (int)l);
  std::wstring ws = pwch;
  delete[] pwch;
  return ws;
}
#else

std::string ws2s(const std::wstring ws)
{
  if (ws.empty())
    return std::string();

  size_t l = sizeof(wchar_t) * ws.size();
  char *mbs = new char[l + 1];
  mbs[l] = '\0';
#ifdef WIN32
  size_t retval;
  wcstombs_s(&retval, mbs, l, ws.c_str(), l);
#else
  std::wcstombs(mbs, ws.c_str(), l);
#endif
  std::string s = mbs;
  delete[] mbs;
  return s;
}

std::wstring s2ws(const std::string s)
{
  size_t l = s.size();
  wchar_t *wcs = new wchar_t[l + 1];
  wcs[l] = L'\0';
#ifdef WIN32
  size_t retval;
  mbstowcs_s(&retval, wcs, l + 1, s.c_str(), l);
#else
  std::mbstowcs(wcs, s.c_str(), l);
#endif
  std::wstring ws(wcs);
  delete[] wcs;
  return ws;
}

#endif
#else
std::string ws2s(const std::wstring ws)
{
#pragma warning(push)
#pragma warning(disable : 4244)
  return std::string(ws.begin(), ws.end());
#pragma warning(pop)
}

std::wstring s2ws(const std::string s)
{
  return std::wstring(s.begin(), s.end());
}
#endif
// Convert a/ * Windows-1252 (aka CP1252) stored as an std::string
// into an * UTF8 std::string.
std::string CP1252_to_UTF8(const std::string &byte_array)
{

  // Byte array => Unicode.
  std::wstring unicode(byte_array.size(), ' ');
  for (size_t i = 0; i < unicode.size(); ++i)
    unicode[i] = CP1252_UNICODE_TABLE[(uint8_t)byte_array[i]];

    // Unicode => UTF8.
#ifdef _MSC_VER
  return ws2s(unicode);

#else
  std::wstring_convert<std::codecvt_utf8<wchar_t>> unicode_to_utf8;
  return unicode_to_utf8.to_bytes(unicode);
#endif
}

// Convert non ascii characters of a wstring to html entities form in decimal (default) or hexa &#[x]value;
std::string whole_to_htent(const std::wstring ws, bool dec_base)
{
  std::stringstream ss;

  for (auto wc : ws)
  {
    ss << "&#";
    if (dec_base)
      ss << std::dec;
    else
      ss << 'x' << std::hex;
    ss << (unsigned int)wc << ';';
  }

  return ss.str();
}

// Convert non ascii characters of a wstring to html entities form in decimal (default) or hexa &#[x]value;
std::string non_ascii_to_htent(const std::wstring ws, bool dec_base)
{
  std::stringstream ss;

  for (auto wc : ws)
  {
    if (isprintable_ascii(wc) && wc != '\\')
      ss << (char)wc;
    else
    {
      ss << "&#";
      if (dec_base)
        ss << std::dec;
      else
        ss << 'x' << std::hex;
      ss << (unsigned int)wc << ';';
      // if (wc == '\\') logInfo(ss.str());
    }
  }

  return ss.str();
}

// Convert non ascii characters of a string to html entities form in decimal (default) or hexa &#[x]value;
std::string non_ascii_to_htent(const std::string s, bool dec_base = false)
{
  std::stringstream ss;

  for (unsigned char c : s)
  {
    if (isprintable_ascii(c) && c != '\\')
      ss << c;
    else
    {
      ss << "&#";
      if (dec_base)
        ss << std::dec;
      else
        ss << 'x' << std::hex;
      ss << (unsigned int)c << ';';
    }
  }

  // logTrace("RES: ", ss.str());
  return ss.str();
}

std::string to_htent(const std::wstring ws, bool dec_base)
{
  return non_ascii_to_htent(ws, dec_base);
}

// Same as previous for string
std::string to_htent(const std::string s, bool dec_base)
{
  return non_ascii_to_htent(s, dec_base);
}

std::wstring replace_all_lambda(const std::wstring &input, const std::wstring &pattern, const std::function<std::wstring(const std::wsmatch &)> &replacer)
{
  std::wregex re(pattern);
  std::wstring result;
  auto it = std::wsregex_iterator(input.begin(), input.end(), re);
  auto end = std::wsregex_iterator();
  size_t last_pos = 0;

  for (; it != end; ++it)
  {
    result.append(input, last_pos, it->position() - last_pos);
    result.append(replacer(*it));
    last_pos = it->position() + it->length();
  }
  result.append(input, last_pos, input.length() - last_pos);

  return result;
}
std::wstring from_htent_w(const std::string html, std::wstring &res)
{
  // logTrace("from_htent_w1: ", html);
  res = s2ws(html);
  std::wstring decimalPattern(L"&#(\\d+);");
  auto decReplacer = [](const std::wsmatch &m) {
    wchar_t ch = static_cast<wchar_t>(std::stoi(m[1].str()));
    return std::wstring(1, ch);
  };

  std::wstring hexPattern(L"&#x([0-9A-Fa-f]+);");
  auto hexReplacer = [](const std::wsmatch &m) {
    wchar_t ch = static_cast<wchar_t>(std::stoi(m[1].str(), nullptr, 16));
    return std::wstring(1, ch);
  };

  // Remplacer les entités numériques décimales
  res = replace_all_lambda(res, decimalPattern, decReplacer);
  res = replace_all_lambda(res, hexPattern, hexReplacer);

  // std::wcout << L"from_htent_w2: " << res << std::endl;
  return res;
}

std::wstring from_htent_w(const std::string htent)
{
  std::wstring ws;
  return from_htent_w(htent, ws);
}

std::string replace_all_lambda(const std::string &input, const std::string &pattern, const std::function<std::string(const std::smatch &)> &replacer)
{
  std::regex re(pattern);
  std::string result;
  auto it = std::sregex_iterator(input.begin(), input.end(), re);
  auto end = std::sregex_iterator();
  size_t last_pos = 0;

  for (; it != end; ++it)
  {
    result.append(input, last_pos, it->position() - last_pos);
    result.append(replacer(*it));
    last_pos = it->position() + it->length();
  }
  result.append(input, last_pos, input.length() - last_pos);

  return result;
}
std::string from_htent_a(const std::string html, std::string &res)
{
  // logTrace("from_htent_a1: ", html);
  res = html;
  std::string decimalPattern("&#(\\d+);");
  auto decReplacer = [](const std::smatch &m) {
    char ch = static_cast<char>(std::stoi(m[1].str()));
    return std::string(1, ch);
  };

  std::string hexPattern("&#x([0-9A-Fa-f]+);");
  auto hexReplacer = [](const std::smatch &m) {
    char ch = static_cast<char>(std::stoi(m[1].str(), nullptr, 16));
    return std::string(1, ch);
  };

  // Remplacer les entités numériques décimales
  res = replace_all_lambda(res, decimalPattern, decReplacer);
  res = replace_all_lambda(res, hexPattern, hexReplacer);

  // logTrace("from_htent_a2: ", res);
  // std::cout << "from_htent_a2: " << res << std::endl;
  return res;
}

std::string from_htent_a(const std::string htent)
{
  std::string s;
  return from_htent_a(htent, s);
}

#ifdef _WIN32
const wchar_t *getcmdw()
{
  static wchar_t cmd_path[MAX_PATH];
  if (SearchPathW(nullptr, L"cmd", L".exe", MAX_PATH, cmd_path, nullptr))
    return cmd_path;
  else
    return L"C:\\Windows\\System32\\cmd.exe";
}

const char *getcmda()
{
  static char cmd_path[MAX_PATH];
  if (SearchPathA(nullptr, "cmd", ".exe", MAX_PATH, cmd_path, nullptr))
    return cmd_path;
  else
    return "C:\\Windows\\System32\\cmd.exe";
}
#endif

#ifdef _WIN32
std::string wpipe2s(const std::wstring command)
{
  FILE *fp = _wpopen((command + L" 2>&1").c_str(), L"r");

  if (fp)
  {
    std::ostringstream oss;
    constexpr std::size_t MAX_LINE_SZ = 1024;
    wchar_t line[MAX_LINE_SZ];
    while (fgetws(line, MAX_LINE_SZ, fp))
      oss << ws2s(line);
    _pclose(fp);
    std::string res = oss.str();
    return res;
  }

  return "";
}
#define popen _popen
#define pclose _pclose
#endif
std::string pipe2s(const std::string command)
{
  FILE *fp = popen((command + " 2>&1").c_str(), "r");

  if (fp)
  {
    std::ostringstream oss;
    constexpr std::size_t MAX_LINE_SZ = 1024;
    char line[MAX_LINE_SZ];
    while (fgets(line, MAX_LINE_SZ, fp))
      oss << line;
    pclose(fp);
    std::string res = oss.str();
    // logTrace("RES: ", res);
    return res;
  }

  return "";
}

#ifdef _WIN32
bool CmdOutputToFile(std::string cmd, std::string fname, bool show_window)
{
  SECURITY_ATTRIBUTES sa = {};
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  HANDLE hFile;
  if ((hFile = CreateFile(fname.c_str(), FILE_WRITE_DATA, FILE_SHARE_DELETE | FILE_SHARE_READ, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY, 0)) == NULL)
  {
    logError(StrError("CreateFile"));
    return false;
  }

  AllocConsole();
  if (show_window)
    ShowWindow(GetConsoleWindow(), SW_SHOW);
  else
    ShowWindow(GetConsoleWindow(), SW_HIDE);

  SetStdHandle(STD_OUTPUT_HANDLE, hFile);
  SetStdHandle(STD_ERROR_HANDLE, hFile);

  PROCESS_INFORMATION pi = {};
  STARTUPINFO si = {};
  si.cb = sizeof(STARTUPINFO);
  if (CreateProcess(NULL, const_cast<char *>(cmd.c_str()), 0, 0, 0, 0, 0, 0, &si, &pi) == FALSE)
  {
    logError(StrError("CreateProcess"));
    return false;
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  FreeConsole();
  CloseHandle(hFile);
  return true;
}

bool CmdOutputToWFile(std::wstring cmd, std::string fname, bool show_window)
{
  SECURITY_ATTRIBUTES sa = {};
  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  HANDLE hFile;
  if ((hFile = CreateFileW(s2ws(fname).c_str(), FILE_WRITE_DATA, FILE_SHARE_DELETE | FILE_SHARE_READ, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_TEMPORARY, 0)) == NULL)
  {
    logError(StrError("CreateFile"));
    return false;
  }

  AllocConsole();
  if (show_window)
    ShowWindow(GetConsoleWindow(), SW_SHOW);
  else
    ShowWindow(GetConsoleWindow(), SW_HIDE);

  SetStdHandle(STD_OUTPUT_HANDLE, hFile);
  SetStdHandle(STD_ERROR_HANDLE, hFile);

  PROCESS_INFORMATION pi = {};
  STARTUPINFOW si = {};
  si.cb = sizeof(STARTUPINFO);

  if (CreateProcessW(NULL, const_cast<wchar_t *>(cmd.c_str()), 0, 0, 0, 0, 0, 0, &si, &pi) == FALSE)
  {
    logError(StrError("CreateProcess"));
    return false;
  }

  WaitForSingleObject(pi.hProcess, INFINITE);

  FreeConsole();
  CloseHandle(hFile);
  return true;
}

#endif

bool shell_cmd(std::string cmd, std::string &shell_output, bool show_window)
{
  if (cmd == "")
  {
    logError("Command cannot be empty");
    return false;
  }

#ifdef _WIN32
  std::string tmpf = tempfile();

  if (CmdOutputToFile(cmd, tmpf, show_window))
  {
    shell_output = file2s(tmpf);
    // logTrace(shell_output);
    std::filesystem::remove(tmpf);
    return true;
  }

  return false;
#else
  shell_output = pipe2s(cmd);
  if (shell_output.empty())
    return false;
  return true;
#endif
}

bool wshell_cmd(std::wstring cmd, std::string &shell_output, bool show_window)
{
  if (cmd == L"")
  {
    logError("Command cannot be empty");
    return false;
  }

#ifdef _WIN32
  std::string tmpf = tempfile();

  if (CmdOutputToWFile(cmd, tmpf, show_window))
  {
    shell_output = file2s(tmpf);
    std::filesystem::remove(tmpf);
    return true;
  }

  return false;
#else
  shell_output = pipe2s(ws2s(cmd));
  if (shell_output.empty())
    return false;
  return true;
#endif
}

std::string wexec_cmd(std::wstring cmd)
{
  if (cmd == L"")
    return "";
  std::string res;
  std::string tmpf = tempfile();

#ifdef _WIN32
  // wres=s2ws(CmdOutputToWFile(cmd, tmpf, true));
  _wsystem((cmd + L" > " + s2ws(tmpf) + L" 2>&1").c_str());
#else
  std::system(ws2s((L"( " + cmd + L" ) > " + s2ws(tmpf) + L" 2>&1")).c_str());
#endif
  res = file2s(tmpf);
  // logTrace(tmpf);
  std::filesystem::remove(tmpf);

  return res;
}

std::string exec_cmd(std::string cmd)
{
  if (cmd == "")
    return "";
  std::string res = "", tmpf = tempfile();
  std::system(("( " + cmd + " ) >" + tmpf + " 2>&1").c_str());
  res = file2s(tmpf);
  trim(res);
  // logTrace(tmpf);
  std::filesystem::remove(tmpf);
  // logTrace("RES: ", res);
  return res;
}

// Découpe une chaine en 2 la partie commande et la partie arguments
void split_cmd_and_params(const std::string s, std::string &prog, std::string &params)
{
  std::string cmd(s);
  trim(cmd);
  size_t end_exe = 0;

  if (cmd[0] == '\"')
  {
    end_exe = cmd.find_first_of('\"', 1);
    if (std::string::npos != end_exe)
    {
      prog = cmd.substr(1, end_exe - 1);
      params = cmd.substr(end_exe + 1);
    }
    else
    {
      prog = cmd.substr(1, end_exe);
      params = "";
    }
  }
  else
  {
    do
    {
      end_exe = cmd.find_first_of(' ', end_exe);
      if (end_exe > 0 && cmd[end_exe - 1] == '\\')
        end_exe++;
      else
        break;
    } while (end_exe != std::string::npos);

    prog = cmd.substr(0, end_exe);
    if (std::string::npos != end_exe)
    {
      params = cmd.substr(end_exe + 1);
    }
    else
    {
      params = "";
    }
  }
}

void split_cmd_and_params(const std::wstring s, std::wstring &prog, std::wstring &params)
{
  std::wstring cmd(s);
  trim(cmd);
  size_t end_exe = 0;

  if (cmd[0] == '\"')
  {
    end_exe = cmd.find_first_of('\"', 1);
    if (std::wstring::npos != end_exe)
    {
      prog = cmd.substr(1, end_exe - 1);
      params = cmd.substr(end_exe + 1);
    }
    else
    {
      prog = cmd.substr(1, end_exe);
      params = L"";
    }
  }
  else
  {
    do
    {
      end_exe = cmd.find_first_of(' ', end_exe);
      if (end_exe > 0 && cmd[end_exe - 1] == '\\')
        end_exe++;
      else
        break;
    } while (end_exe != std::wstring::npos);

    prog = cmd.substr(0, end_exe);
    if (std::wstring::npos != end_exe)
    {
      params = cmd.substr(end_exe + 1);
    }
    else
    {
      params = L"";
    }
  }
}

#ifdef _WIN32
// See https://stackoverflow.com/questions/15826188/what-most-correct-way-to-set-the-encoding-in-c
void SetConsoleToUnicodeFont()
{
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (IsWindows8OrGreater())
  {
    // Call the documented function.
    typedef BOOL(WINAPI * pfSetCurrentConsoleFontEx)(HANDLE, BOOL, PCONSOLE_FONT_INFOEX);
    HMODULE hMod = GetModuleHandle(TEXT("kernel32.dll"));
    pfSetCurrentConsoleFontEx pfSCCFX = (pfSetCurrentConsoleFontEx)GetProcAddress(hMod, "SetCurrentConsoleFontEx");

    CONSOLE_FONT_INFOEX cfix;
    cfix.cbSize = sizeof(cfix);
    cfix.nFont = 12;
    cfix.dwFontSize.X = 8;
    cfix.dwFontSize.Y = 14;
    cfix.FontFamily = FF_DONTCARE;
    cfix.FontWeight = 400; // normal weight
    StrCpyW(cfix.FaceName, L"Lucida Console");

    pfSCCFX(hConsole, FALSE, /* set font for current window size */
            &cfix);
  }
  else
  {
    // There is no supported function on these older versions,
    // so we have to call the undocumented one.
    typedef BOOL(WINAPI * pfSetConsoleFont)(HANDLE, DWORD);
    HMODULE hMod = GetModuleHandle(TEXT("kernel32.dll"));
    pfSetConsoleFont pfSCF = (pfSetConsoleFont)GetProcAddress(hMod, "SetConsoleFont");
    pfSCF(hConsole, 12);
  }
}
#endif

bool NotOnlyAscii7(const std::string &s)
{
  return std::any_of(s.begin(), s.end(), [](char c) { return static_cast<unsigned char>(c) > 127; });
}

std::string rep_bs(std::string &s)
{
  std::string bs;
  bs = (char)92;
  replace_all(s, bs, "##BACKSLASH_CODE##");
  s = rep_crlf(s);
  replace_all(s, "##BACKSLASH_CODE##", bs + bs);
  return s;
}

// Filesystem api exposed to javascript
std::string pwd()
{
  return std::filesystem::current_path().string();
}

std::string cwd(std::string new_dir)
{
  std::filesystem::current_path(new_dir);
  return pwd();
}

// Return a string that represents the number in octal form, useful for a command like chmod
std::string to_js_oct(unsigned int number)
{
  std::ostringstream str;
  str << std::oct << number;
  return "0" + str.str();
}

template <typename TP> std::time_t to_time_t(TP tp)
{
  namespace ch = std::chrono;
  auto sctp = ch::time_point_cast<ch::system_clock::duration>(tp - TP::clock::now() + ch::system_clock::now());
  return ch::system_clock::to_time_t(sctp);
}

#ifdef _WIN32
#define my_gmtime(a, b) gmtime_s(b, a)
#define my_localtime(a, b) localtime_s(b, a)
#else
#define my_gmtime(a, b) gmtime_r(a, b)
#define my_localtime(a, b) localtime_r(a, b)
#endif
// Convert a file time to a string, default format is ISO8601 and default time
// zone is local
// std::string file_time_to_string(std::filesystem::file_time_type file_time, std::string fmt = "%Y-%m-%d %H:%M:%S", bool gm = false);
std::string file_time_to_string(std::filesystem::file_time_type file_time, std::string fmt, bool gm)
{
  std::time_t tt = to_time_t(file_time);
  std::tm tim;
  if (gm)
    my_gmtime(&tt, &tim);
  else
    my_localtime(&tt, &tim);
  std::stringstream buffer;
  buffer << std::put_time(&tim, fmt.c_str());
  std::string fmtime = buffer.str();

  return fmtime;
}

std::string ISO8601()
{
  auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::tm tim;
  my_localtime(&tt, &tim);
#ifdef _WIN32
  // Avec put_time %z donne des resultats aleatoire sous windows/msys2
#define TIME_FORMAT "%Y-%m-%dT%H:%M:%S"
#else
#define TIME_FORMAT "%Y-%m-%dT%H:%M:%S%z"
#endif
  std::stringstream buffer;
  buffer << std::put_time(&tim, TIME_FORMAT);
  return buffer.str();
}

std::string lastwrite(std::filesystem::path p)
{
  return file_time_to_string(std::filesystem::last_write_time(p));
}

bool is_number(const std::string &s)
{
  static const std::regex number_regex(R"(^[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?$)");
  return std::regex_match(s, number_regex);
}

bool is_base64(const std::string &s)
{
  if (s.length() % 4 == 0)
    return std::regex_match(s, std::regex("^[A-Za-z0-9+/]*={0,2}$"));
  return false;
}

std::ostream &operator<<(std::ostream &os, const std::locale &loc)
{
  if (loc.name().length() <= 80)
    os << loc.name();
  else
    for (const auto c : loc.name())
      os << c << (c == ';' ? "\n  " : "");

  return os << std::endl;
}

bool is_utf8()
{
  std::locale l;
  std::string s = l.name();
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  return (s.find("utf8") != std::string::npos || s.find("utf-8") != std::string::npos);
}

void print_locale()
{
  std::locale l;
  std::cout << "Current locale: " << l.name() << std::endl;

  if (is_utf8())
    std::cout << "Locale is utf8" << std::endl;
  else
    std::cout << "Locale is NOT utf8" << std::endl;

  //  std::cout << "Classic locale: " << std::locale::classic().name() << std::endl;
}

void force_utf8()
{
  std::locale l;
  std::string s = l.name();
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
  if (s.find("utf8") == std::string::npos && s.find("utf-8") == std::string::npos)
  {
    std::cout << "Forcing to UTF-8" << std::endl;
    std::locale::global(std::locale("en_US.UTF-8"));
  }
}
