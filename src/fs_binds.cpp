
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

// clang-format off
#ifdef _WIN32
#include <winerror.h>
#include <winsock.h>
#include <accctrl.h>
#include <sddl.h>
#include <aclapi.h>
#include <shlwapi.h>
#include <pathcch.h>
#include "winapi.h"
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#endif
#include "util.h"
#include "log.h"
#include "options.h"
#include "base64.h"
#include "wrap.h"
#include "path_entity.h"
#include "fs_binds.h"
// clang-format on

#define trc std::cout << __LINE__ << std::endl

std::string iso_8859_1_to_utf8(std::string &str)
{
  std::string strOut;
    for (std::string::iterator it = str.begin(); it != str.end(); ++it)
    {
        uint8_t ch = *it;
        if (ch < 0x80) {
            strOut.push_back(ch);
        }
        else {
            strOut.push_back(0xc0 | ch >> 6);
            strOut.push_back(0x80 | (ch & 0x3f));
        }
    }
    return strOut;
}

bool set_fs_error(webview_wrapper &w, std::filesystem::path pcaller, const std::error_code ec)
{
  //logDebug("Error number: ", ec.value());
  std::string caller;

  if (ec.value() != 0)
  {
    std::filesystem::path perrm(s2ws(ec.message()));
    std::string errm; //(ec.message());

#ifdef _MSC_VER
    errm = path_to_htent(perrm);
    caller = path_to_htent(pcaller);
#else
    errm = perrm.string();
    caller = pcaller.string();
#endif

    std::filesystem::path last_error("Last error with " + caller + " is: " + errm);
    logError(last_error);
    w.setvar("fs", "last_error", last_error);
    return true;
  }

  //logDebug("No error with " + caller);
  w.setvar("fs", "last_error", "No error with " + caller);
  return false;
}

void fwrite(webview_wrapper &w, const std::string &seq, const std::string &req, std::ios_base::openmode omod = std::ios::trunc)
{
  std::thread([&, seq, req, omod] {
    std::string fn, s;
    fn = json_parse(req, "", 0);
    std::ofstream f(fn, omod);

    if (f)
    {
      int n = 1;

      for (;;)
      {
        s = json_parse(req, "", n++);
        if (s.empty())
          break;
        try
        {
          f << s;
        }
        catch (std::ofstream::failure &e)
        {
          logE(e.what());
        }
      }

      f.close();
      w.resolve(seq, 0, "true");
    }
    else
      w.resolve(seq, 0, "false");
  }).detach();
}

// As file_type have unspecified values in the C++ standard
// and as I notice a difference between G++ and MSVC
// let define a forced order values of my own choice
int file_type_index(std::filesystem::file_type ft)
{
  switch (ft)
  {
  case std::filesystem::file_type::not_found:
    return 0;
  case std::filesystem::file_type::none:
    return 1;
  case std::filesystem::file_type::regular:
    return 2;
  case std::filesystem::file_type::directory:
    return 3;
  case std::filesystem::file_type::symlink:
    return 4;
  case std::filesystem::file_type::block:
    return 5;
  case std::filesystem::file_type::character:
    return 6;
  case std::filesystem::file_type::fifo:
    return 7;
  case std::filesystem::file_type::socket:
    return 8;
  case std::filesystem::file_type::unknown:
  default:
    return 9;
  }
}

std::vector<std::string> file_type_string_vector = {"not_found", "none", "regular", "directory", "symlink", "block", "character", "fifo", "socket", "unknown"};

/* From https://www.bogotobogo.com/Linux/linux_File_Types.php:
Regular files ('-')
Directory files ('d')
Special files (Actually, this one has 5 types in it.)
  Block file('b')
  Character device file('c')
  Named pipe file or just a pipe file('p')
  Symbolic link file('l')
  Socket file('s')
I have added
  not_found('n')
  none('o')
  unknown('u')
*/
std::vector<char> file_type_char_vector = {'n', 'o', '-', 'd', 'l', 'b', 'c', 'p', 's', 'u'};

char file_type_char(std::filesystem::file_type ft)
{
  return file_type_char_vector[file_type_index(ft)];
}

std::string file_type_string(std::filesystem::file_type ft)
{
  return file_type_string_vector[file_type_index(ft)];
}

std::string padint(size_t n, int w)
{
  std::string rs = std::to_string(n);
  auto ls = w - rs.size();
  std::string res = "";
  if (ls > 0)
    res = std::string(ls, ' ');
  return res + rs;
}

std::string padstr(std::string s, size_t w)
{
  if (s.size() > w)
    return s;
  auto ls = w - s.size();
  std::string res = "";
  if (ls > 0)
    res = std::string(ls, ' ');
  return res + s;
}

// Format a number expressed in bytes to Bytes, KiloBytes, MegaBytes, GigaBytes, TeraBytes, PetaBytes, ExaBytes
// According to its size (starting from kilobytes multiple of 1000 (or 1024) implies unit upgrade)
// Size in bytes are unchanged until 10000
std::vector<std::string> byte_mult = {"K", "M", "G", "T", "P", "E", "Z", "Y", "R", "Q"};
std::string fmt_bytes(size_t n)
{
  if (n < 10000)
    return std::to_string(n) + "B";

  const size_t mult = 1000;
  size_t coeff1, coeff2, val;

  coeff1 = mult;
  coeff2 = coeff1 * mult * 10;

  for (auto bm : byte_mult)
  {
    if (n < coeff2)
    {
      val = n / coeff1;
      return std::to_string(val) + bm;
    }
    coeff1 *= mult;
    coeff2 *= mult;
  }

  return "Too big value: " + std::to_string(n);
}

#ifdef _WIN32
std::string StrError3(std::string sf, int sfn, HANDLE hFile, std::filesystem::path p, std::string s)
{
  DWORD LastError = GetLastError();
  std::string ps = ws2s(p.wstring());
  // logTrace('[', sf, ", ", sfn, "] - ", s, " on file ", ps, " - Error ", LastError, ": ", TxtError(LastError));
  // std::cerr << '[' << sf << " << " << sfn << "] - " << s << " on file " << p.string() << " - Error " << LastError << ": " << TxtError(LastError) << std::endl;

  CloseHandle(hFile);
  return SimpleError(LastError);
}
#define StrError2(a, b, c) StrError3(__FILE__, __LINE__, a, b, c)

// Corriger access denied sur répertoire
std::string WinGetOwner(std::filesystem::path p)
{
  PSID pSidOwner = NULL;
  BOOL bRtnBool = TRUE;
  LPTSTR AcctName = NULL, DomainName = NULL;
  DWORD dwAcctName = 0, dwDomainName = 0;
  SID_NAME_USE eUse = SidTypeUnknown;
  HANDLE hFile;
  PSECURITY_DESCRIPTOR pSD = NULL;

  DWORD dwFlagsAndAttributes;
  auto ft = std::filesystem::status(p).type();
  if (ft == std::filesystem::file_type::directory)
  {
    dwFlagsAndAttributes = FILE_FLAG_BACKUP_SEMANTICS;
  }
  else
  {
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  }

  p.make_preferred();
  PCWSTR ppszServer;

  char hostname[MAX_COMPUTERNAME_LENGTH + 1];

  if (PathIsUNCEx(p.wstring().c_str(), &ppszServer))
  {
    std::filesystem::path srv = ppszServer;
    std::string s = srv.string();
    s = s.substr(0, s.find("\\"));
    strncpy_s(hostname, MAX_COMPUTERNAME_LENGTH, s.c_str(), _TRUNCATE);
  }
  else
  {
    DWORD sz = MAX_COMPUTERNAME_LENGTH;
    if (!GetComputerName(hostname, &sz))
      logError(StrError("GetComputerName"));
  }

  //  logTrace("host name: ", hostname );

  hFile = CreateFileW(p.wstring().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);

  // Get the handle of the file object.
  if (hFile == INVALID_HANDLE_VALUE)
    return StrError2(hFile, p, "CreateFile");
  // Get the owner SID of the file.
  if (GetSecurityInfo(hFile, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &pSidOwner, NULL, NULL, NULL, &pSD) != ERROR_SUCCESS)
  {
    return StrError2(hFile, p, "GetSecurityInfo");
  }

  // First call to LookupAccountSid to get the buffer sizes.
  bRtnBool = LookupAccountSid(hostname, pSidOwner, AcctName, (LPDWORD)&dwAcctName, DomainName, (LPDWORD)&dwDomainName, &eUse);
  DWORD LastError = GetLastError();
  if (LastError == ERROR_INSUFFICIENT_BUFFER)
  {
    // Reallocate memory for the buffers.
    AcctName = (LPTSTR)GlobalAlloc(GMEM_FIXED, dwAcctName * sizeof(wchar_t));
    // Check LastError for GlobalAlloc error condition.
    if (AcctName == NULL)
      return StrError2(hFile, p, "GlobalAlloc");
    DomainName = (LPTSTR)GlobalAlloc(GMEM_FIXED, dwDomainName * sizeof(wchar_t));
    // Check LastError for GlobalAlloc error condition.
    if (DomainName == NULL)
      return StrError2(hFile, p, "GlobalAlloc");

    // Second call to LookupAccountSid to get the account name.
    bRtnBool = LookupAccountSid(hostname, pSidOwner, AcctName, (LPDWORD)&dwAcctName, DomainName, (LPDWORD)&dwDomainName, &eUse);
  }

  // Check LastError for LookupAccountSid error condition.
  if (bRtnBool == FALSE)
  {
    if (LastError == ERROR_NONE_MAPPED)
    {
      // logTrace("pSidOwner: ", pSidOwner);
      LPSTR StringSid;
      if (ConvertSidToStringSid(pSidOwner, &StringSid))
      {
        std::string ret(StringSid);
        LocalFree(StringSid);
        CloseHandle(hFile);
        return ret;
      }
      else
        return StrError2(hFile, p, "ConvertSidToStringSid");
    }
    else
      return StrError2(hFile, p, "LookupAccountSid");
  }
  else if (bRtnBool == TRUE)
  {
    std::string ret = DomainName + std::string("\\") + AcctName;
    CloseHandle(hFile);
    return ret;
  }

  CloseHandle(hFile);
  return "nothing";
}
#endif

#define trc_return(str) return str;
// #define trc_return(str) { logTrace(__LINE__, ", ", str, std::endl; return str); }
std::string getowner(std::filesystem::path p)
{
#ifdef _WIN32
  return WinGetOwner(p);
#else // Linux
  struct stat buf;
  if (stat(p.string().c_str(), &buf) == 0)
  {
    struct passwd *pass = getpwuid(buf.st_uid);
    return pass->pw_name;
  }

  return "";
#endif
}

std::string getgroup(std::filesystem::path p)
{
  std::string ret = "";
#ifdef _WIN32
  // Il n'y a pas la notion de groupe sous Windows, seulement sous AD.
  // Et les groupes AD sont uniquement attachées aux utilisateurs, pas aux fichiers.
  ret = "NOGROUP";
#else
  struct stat buf;
  if (stat(p.c_str(), &buf) == 0)
  {
    struct group *grp = getgrgid(buf.st_gid);
    return grp->gr_name;
  }

#endif
  return ret;
}

#define sstr(op, perm) sp += (perms::none == (perm & p) ? '-' : op);
std::string perm_str(std::filesystem::perms p)
{
  using std::filesystem::perms;
  std::string sp = "";
  sstr('r', perms::owner_read);
  sstr('w', perms::owner_write);
  sstr('x', perms::owner_exec);
  sstr('r', perms::group_read);
  sstr('w', perms::group_write);
  sstr('x', perms::group_exec);
  sstr('r', perms::others_read);
  sstr('w', perms::others_write);
  sstr('x', perms::others_exec);

  return sp;
}
std::string do_stat(webview_wrapper &w, std::filesystem::path p, bool json = false)
{
  std::error_code ec;
  if (!std::filesystem::exists(p))
  {
    w.setvar("fs", "last_error", "Cannot stat non existing file " + p.string());
    return "false";
  }

  auto fs = std::filesystem::status(p, ec);
  if (set_fs_error(w, std::filesystem::path("stat error on existing file '" + p.string() + "'"), ec))
    return "false";

  auto ft = fs.type();

  std::uintmax_t sz;
  if (ft == std::filesystem::file_type::regular)
    sz = std::filesystem::file_size(p);
  else
    sz = static_cast<std::uintmax_t>(-1);

  std::string lastwr;
  if (ft != std::filesystem::file_type::not_found)
    lastwr = lastwrite(p);
  else
    lastwr = "YYYY-mm-DD HH:MM:SS";

  unsigned int perms = (unsigned)fs.permissions();
  std::string res = "";

  std::string owner, group;
#ifdef _WIN32
  std::filesystem::path po = s2ws(getowner(p));
  //  std::filesystem::path pg = s2ws(getgroup(p));
  owner = po.string();
//  group=pg.string();
#else
  owner = getowner(p);
  group = getgroup(p);
#endif

#ifdef _WIN32
  replace_all(owner, "\\", "/");
#endif
#ifndef _WIN32
  // replace_all(group, "\\", "/");
#endif

  if (json)
  {
    std::string ssz = "";
    if (sz == static_cast<std::uintmax_t>(-1))
      ssz = "N/A";
    else
      ssz = std::to_string(sz);
    res += "{\"path\":\"" + p.string() + "\",";
    res += "\"type\":\"" + file_type_string(ft) + "\",";
    res += "\"perms\":\"" + to_js_oct(perms) + "\",";
    if (!owner.empty())
      res += "\"owner\":\"" + owner + "\",";
#ifndef _WIN32
    if (!group.empty())
      res += "\"group\":\"" + group + "\",";
#endif
    res += "\"size\":\"" + ssz + "\",";
    res += "\"last_write\":\"" + lastwr;
    res += "\"}";
  }
  else
  {                                    // drwxrwxrwx;owner[;group];last write;human readable size;filename
    res += file_type_char(ft);         // File type
    res += perm_str(fs.permissions()); // right
    res += ';';
    res += owner;

#ifndef _WIN32
    res += ';';
    res += group;
#endif
    res += ';';
    if (sz != static_cast<std::uintmax_t>(-1))
    {
      // res += fmt_bytes((size_t)sz); // Size
      res += std::to_string((size_t)sz); // Size
    }
    res += ';';
    res += lastwr; // Last write date and time
    res += ';';
    res += p.string(); // File name
  }

  // logTrace(res);
  return res;
}

std::string pstat(webview_wrapper &w, std::filesystem::path p, bool detail, bool json = false)
{
  std::string s;
  s = p.string();

  if (s.size() > 1 && s[0] == '.' && (s[1] == '/' || s[1] == '\\'))
    s = s.substr(2);

  std::string res;

  if (detail)
  {
    res = do_stat(w, p, json);
  }
  else
  {
    res = s;
  }

  if (json && !detail)
  {
    res = "{ \"path\":\"" + res + "\"}";
    res += ',';
    return res;
  } /*
   else
   {
     res += "\n";
   }*/

  return res;
}

// Transforme la liste détaillée au format csv en une table texte ajustée avec des espaces
std::string csv_to_formatted_text(std::string &s, std::string col_sep = ";", std::string row_sep = "\n")
{
  std::string grp = {};
#ifndef _WIN32
  // Pas de groupe pour les fichiers sous Windows, la notion de groupe est rattachée aux utilisateurs avec AD.
  grp = "group";
#endif
  s = "rights;owner;" + grp + "size;last write;filename\n" + s;

  auto rows = split(s, '\n');
  std::string res = {};
  std::vector<size_t> max_widths;

  // First pass evaluate the max width of each column
  for (auto row : rows)
  {
    auto cols = split_esc(row);
    if (max_widths.size() < cols.size())
      max_widths.resize(cols.size());
    for (size_t i = 0; i < cols.size(); i++)
    {
      if (cols[i].size() > max_widths[i])
        max_widths[i] = cols[i].size();
    }
  }

  // Second pass adjust each col with spaces (on the left for numbers and on right for 'not numbers')
  for (size_t r = 0; r < rows.size(); r++)
  {
    std::string adjusted_line = {};
    auto cols = split_esc(rows[r]);
    for (size_t c = 0; c < cols.size(); c++)
    {
      size_t adj_wi = 1 + max_widths[c] - cols[c].size();
      //logDebug("row number: ", r, ", col number: ", c, ", col: ", cols[c], ", col size: ", cols[c].size(), ", max col size: ", max_widths[c], ", adj-width: ", adj_wi);

      if (r == 0) // Center the first line's columns (header line)
      {
        if (c < cols.size() - 1)
        {
          size_t cen = adj_wi / 2;
          adjusted_line += std::string(cen, ' ') + cols[c];
          if (2 * cen < adj_wi)
            cen++;
          adjusted_line += std::string(cen, ' ');
        }
        else
        {
          adjusted_line += "   " + cols[c];
        }
      }
      else
      {
        if (is_number(cols[c]))
        {
          adjusted_line += std::string(adj_wi - 1, ' ') + cols[c] + ' ';
        }
        else
        {
          adjusted_line += cols[c] + std::string(adj_wi, ' ');
        }
      }
    }

    res += rtrim(adjusted_line) + '\n';
  }

  s = res;
  return res;
}

// Transforme la liste détaillée au format csv en html table
std::string csv_dir_to_html_table(std::string &s, std::string col_sep = ";", std::string line_sep = "\n")
{
  std::string grp, chld;
#ifdef _WIN32
  // Pas de groupe pour les fichiers sous Windows, la notion de groupe est rattachée aux utilisateurs avec AD.
  chld = "3";
  grp = "";
#else
  chld = "4";
  grp = "<th>group</th>";
#endif
  /* top | right | bottom | left */
  std::string tstyle = "<style>#listdir>tbody>tr>:nth-child(" + chld += ") { text-align: end; } table, th, td { border: 0; margin:0; border-spacing:0; padding:1px 4px 0 0; } </style>";
  std::string head = "<thead><tr><th>rights</th><th>owner</th>" + grp + "<th>size</th><th>last write</th><th>filename</th></tr></thead>";

  //  if (s[s.length() - 2] == '\\' && s.back() == 'n') { s.pop_back(); s.pop_back(); }

  replace_all(s, line_sep, "</td></tr>\n<tr><td>");
  replace_all(s, col_sep, "</td><td>");
  s = tstyle + "<table id='listdir'>\n" + head + "\n<tr><td>" + s + "</td></tr>\n</table>";

  return s;
}

// Return the directory list of the first parameter (default to current), recursive or not (default not) and in json or text (default text)
// The result of the list is URI encoded
std::string lsdir(webview_wrapper &w, std::filesystem::path path, std::string fmt = {}, bool detail = false, bool recursive = false)
{
  std::string res = "";

  bool json = (fmt == "json");

  if (path.empty())
    path = ".";

  if (std::filesystem::is_directory(path))
  {
    // Attention au récursif, dangereux, ça peut planter le PC ...
    if (recursive)
    {
      //logDebug("Recursive listing");
      for (const auto &e : std::filesystem::recursive_directory_iterator(path))
      {
        res += pstat(w, e, detail, json) + '\n';
      }
    }
    else
    {
      for (const auto &e : std::filesystem::directory_iterator(path))
      {
        res += pstat(w, e, detail, json) + '\n';
      }
    }
  }
  else
  {
    if (std::filesystem::is_regular_file(path))
    {
      res += pstat(w, path.filename(), detail, json) + '\n';
    }
    else
    {
      res += "not a directory or file '" + path.filename().string() + "'";
    }
  }

  // Remove last character if needed
  while (res.back() == ',' || res.back() == '\r' || res.back() == '\n' || res.back() == '\\')
    res.pop_back();

  if (json)
  {
    res = "[" + res + "]";
  }
  else if (detail)
  {
    if (!starts_with(res, "not a directory or file"))
    {

      if (fmt == "html")
        csv_dir_to_html_table(res);
      else
        csv_to_formatted_text(res);
    }

    if (res.empty())
      res = "No file to list for the path '" + path.string() + "'";
    else if (std::filesystem::is_directory(path))
    {
      std::string title = "In folder '" + std::filesystem::absolute(path).string() + "'\n";
      if (fmt == "html")
        title = "<b>" + title + "</b>";
      res = title + res;
    }
  }

  if (res.empty())
  {
    res = "No file found for '" + path.string() + "'";
  }

  return res;
}

// Change to directory provided by 'p' if it is not empty
// Then return the value of the new current path if OK or the error message
std::filesystem::path curr_dir(webview_wrapper &w, std::filesystem::path p = {})
{
  std::error_code ec;

  if (p.empty())
  {
    p = std::filesystem::current_path(ec);
  }
  else
  {
    p = std::filesystem::absolute(p);
    std::filesystem::current_path(p, ec);
  }

  if (set_fs_error(w, std::filesystem::path("current_path on '" + p.string() + "'"), ec))
    return "false";

  if (!p.empty())
    w.setvar("fs", "cwd", p);

  // logDebug(p);
  return p;
}

void create_fs_binds(webview_wrapper &w)
{
  w.bind_doc(                                                     //
      "fs_exists",                                                //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::filesystem::path p = json_parse(req, "", 0);
          std::error_code ec;

          if (std::filesystem::exists(p, ec))
            w.resolve(seq, 0, "true");
          else
          {
            set_fs_error(w, std::filesystem::path("exists on '" + p.string() + "'"), ec);
            w.resolve(seq, 0, "false");
          }
        }).detach();
      },
      "return true if file exists else false and set 'fs.last_error' variable.", //
      1);

  w.bind_doc(            //
      "fs_current_path", //
      [&](const std::string &req) -> std::string {
        std::filesystem::path p = json_parse(req, "", 0);
        std::filesystem::path res = curr_dir(w, p);
        if (res == "false")
          return "false";
        return w.json_escape(res.string());
      },
      "change the current path to the one provided, if any, and return it, if ok, else return false and set the variable 'fs.last_error'.", //
      -1);

  w.bind_doc(                                                     //
      "fs_absolute",                                              //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::filesystem::path p = json_parse(req, "", 0);
          if (p.empty())
            p = ".";
          std::error_code ec;
          auto res = std::filesystem::absolute(p, ec).string();
          if (set_fs_error(w, std::filesystem::path("absolute on '" + p.string() + "'"), ec))
            w.resolve(seq, 0, "false");
          w.resolve(seq, 0, w.json_escape(res));
        }).detach();
      },
      "return the corresponding absolute path of a the parameter or false if error.", //
      1);

  w.bind_doc(                                                     //
      "fs_mkdir",                                                 //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::filesystem::path p = json_parse(req, "", 0);
          std::string res;
          std::error_code ec;

          if (std::filesystem::create_directory(p, ec))
          {
            res = "true";
          }
          else
          {
            set_fs_error(w, std::filesystem::path("mkdir on '" + p.string() + "'"), ec);
            res = "false";
          }

          w.resolve(seq, 0, res);
        }).detach();
      },
      "create a directory with the provided path if it does not already exists and return true if ok else false and set the variable 'fs.last_error'.", //
      1);
  w.bind_doc(                                    //
      "fs_stat",                                 //
      [&](const std::string &req) -> std::string //
      {
        std::filesystem::path path = json_parse(req, "", 0);
        auto json = str2bool(json_parse(req, "", 1));
        std::string res = pstat(w, path, true, json);

        if (res == "false")
        {
          return "false";
        }
        else
        {
          if (res.back() == ',')
            res.pop_back();
          return w.json_escape(res);
        }
      },
      "gives information details on the provided path in json format if second parameter is true (else in text).", //
      -2);

  // Ne fonctionne pas
  w.bind_doc(  //
      "fs_ls", //
      [&](const std::string &seq, const std::string &req, void *) {
        std::thread([&, seq, req] {
          //logDebug("SEQ: ", seq, ", REQ: ", req);
          // std::filesystem::path path;
          std::string path, fmt, sdet, srec;
          js_params(req, path, sdet, fmt, srec);
          //logDebug("fs.ls(", path, ", ", fmt, ", ", sdet, ", ", srec, ")");
          std::string res = lsdir(w, path, fmt, str2bool(sdet), str2bool(srec));
          w.resolve(seq, 0, w.json_escape(res));
        }).detach();
      },
      "list path (default is current directory), in a simple (default) or detailled list if second parameter is true, in one of the following format: text(default), html or json as defined by the third parameter and" //
      "recursively if fourth parameter is true (default is not).",                                                                                                                                                       //
      -4);

  w.bind_doc(                                                     //
      "fs_copy",                                                  //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          namespace fs = std::filesystem;
          fs::path from = json_parse(req, "", 0);
          fs::path to = json_parse(req, "", 1);
          std::error_code ec;
          fs::copy(from, to, fs::copy_options::update_existing | fs::copy_options::recursive, ec);
          if (set_fs_error(w, std::filesystem::path("copy from '" + from.string() + "' to '" + to.string() + "'"), ec))
            w.resolve(seq, 0, "false");
          else
            w.resolve(seq, 0, "true");
        }).detach();
      },
      "copy a file or directory (recursively) from 'path1' to 'path2'. Already existing files are updated if necessary. Return true if ok else false and set the variable 'fs.last_error'.", //
      2);

  w.bind_doc(                                                     //
      "fs_rename",                                                //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          namespace fs = std::filesystem;
          fs::path from = json_parse(req, "", 0);
          fs::path to = json_parse(req, "", 1);
          std::error_code ec;
          //logDebug(from, "==>", to);
          fs::rename(from, to, ec);
          if (set_fs_error(w, std::filesystem::path("rename from '" + from.string() + "' to '" + to.string() + "'"), ec))
            w.resolve(seq, 0, "false");
          else
            w.resolve(seq, 0, "true");
        }).detach();
      },
      "Moves or renames the filesystem object identified by the first parameter to the one provided by the second parameter as if by the POSIX rename. Return true if ok else false and set the variable 'fs.last_error'.", //
      2);

  w.bind_doc(                                                     //
      "fs_remove",                                                //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          namespace fs = std::filesystem;
          fs::path p = json_parse(req, "", 0);
          std::error_code ec;
          if (std::filesystem::remove(p, ec))
          {
            set_fs_error(w, std::filesystem::path("remove '" + p.string() + "'"), ec);
            w.resolve(seq, 0, "false");
          }
          else
            w.resolve(seq, 0, "true");
        }).detach();
      },
      "remove a file or an empty directory, return true if ok else false and set the variable 'fs.last_error'.", //
      1);

  w.bind_doc(                                                     //
      "fs_remove_all",                                            //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          namespace fs = std::filesystem;
          fs::path p = json_parse(req, "", 0);
          std::error_code ec;
          std::uintmax_t n{std::filesystem::remove_all(p, ec)};
          if (set_fs_error(w, std::filesystem::path("remove_all '" + p.string() + "'"), ec))
            w.resolve(seq, 0, "false");
          else
            w.resolve(seq, 0, w.json_escape(std::to_string(n) + " files or directories have been deleted with the path " + p.string()));
        }).detach();
      },
      "remove a file or a directory RECURSIVELY (BEWARE this erase everything underneath the directory !!)", //
      1);

  w.bind_doc(                                                     //
      "fs_read",                                                  //
      [&](const std::string &seq, const std::string &req, void *) //
      {trc;
        std::thread([&, seq, req] {trc;
          std::filesystem::path p = json_parse(req, "", 0);
          //logDebug("apres resolve");
          w.resolve(seq, 0, w.json_escape(file2s(p)));
        }).detach();
      },
      "read a file with the provided file name and return its content, if possible.", //
      1);

  w.bind_doc(                                                     //
      "fs_read_to_utf",                                                  //
      [&](const std::string &seq, const std::string &req, void *) //
      {trc;
        std::thread([&, seq, req] {trc;
          std::filesystem::path p = json_parse(req, "", 0);
          auto f2s=file2s(p);
          auto utf=iso_8859_1_to_utf8(f2s);
          auto wjs=w.json_escape(utf);
          w.resolve(seq, 0, wjs);
          //logDebug("apres resolve");
          //w.resolve(seq, 0, w.json_escape(file2s(p)));
        }).detach();
      },
      "read a file with the provided file name and return its content, converted to utf, if possible.", //
      1);

  w.bind_doc(                                                     //
      "fs_read_to_base64",                                        //
      [&](const std::string &seq, const std::string &req, void *) //
      {
        std::thread([&, seq, req] {
          std::filesystem::path p = json_parse(req, "", 0);
          std::vector<std::uint8_t> data;
          std::string s = {};

          if (file2bin(p, data))
          {
            s = to_base64(data);
            //logDebug("fs_read_to_base64: file2bin OK");
            // logDebug("fs_read_to_base64: "+s);
          }
          else
            logDebug("fs_read_to_base64: file2bin KO");

          w.resolve(seq, 0, w.json_escape(s));
        }).detach();
      },
      "read a binary file with the provided file name and return its content in base64 encoded text, if possible.", //
      1);

  w.bind_doc(     //
      "fs_write", //
      [&](const std::string &seq, const std::string &req, void *) { fwrite(w, seq, req); },
      "truncate and write to the file who's name is provided as the first parameter, the content of all the following parameters, return true if the operation was OK, else false.", //
      INT_MAX);
  w.bind_doc(      //
      "fs_append", //
      [&](const std::string &seq, const std::string &req, void *) { fwrite(w, seq, req, std::ios::app); },
      "append to the file who's name is provided as the first parameter, the content of all the following parameters, return true if the operation was OK, else false.", //
      INT_MAX);                                                                                                                                                          //

  w.decvar("fs", "last_error", "may contains the last fs object error", "No error with fs object");
  w.decvar("fs", "cwd", "current working directory", curr_dir(w));
}
