
#ifndef UTIL_H
#define UTIL_H

#include <bitset>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

bool starts_with(std::string str, std::string prefix);

std::string CP1252_to_UTF8(const std::string &byte_array);
std::string encodeURIComponent(std::string decoded);
std::string encodeURIComponent(std::wstring decoded);
std::string decodeURIComponent(std::string encoded);

std::string my_getenv(const std::string var, bool msg_if_empty = false);
bool my_setenv(const std::string var, std::string val);
bool my_insenv(const std::string var, std::string val);
bool my_addenv(const std::string var, std::string val);
bool my_delenv(const std::string var);

// Return true if found ndl in hstck, ignoring case
bool ifind(const std::string &hstck, const std::string &ndl);

// Convert wstring to string
std::string ws2s(std::wstring ws);
// Convert string to wstring
std::wstring s2ws(std::string s);

// Convert non ascii characters of a wstring to html entities in the following decimal form &#[dec_value];
std::string to_htent(const std::wstring ws, bool dec_base = true);
// Same as previous for string
std::string to_htent(const std::string s, bool dec_base = true);

// Convert the html entities in hexa or decimal form contained in a string to their wchar_t value, return the obtained
// wstring
std::wstring from_htent_w(const std::string, std::wstring &);
std::wstring from_htent_w(const std::string);
// Same as previous but return a string
std::string from_htent_a(const std::string, std::string &);
std::string from_htent_a(const std::string);

#ifdef WIN32
#define from_htent from_htent_w
#else
#define from_htent from_htent_a
#endif

// Split to a vector of strings the string 's' which contains substrings separated by the delimiter 'delim'
std::vector<std::string> split(const std::string &str, const char delim);
// Split to a vector of strings the string 's' which contains substrings separated by the delimiter 'delim' itself not preceded by the escape character 'esc'.
std::vector<std::string> split_esc(const std::string &s, const char delim = ';', const char esc = '\\');
std::vector<std::string> split(std::string str, std::string delimiters = " ");
void split_1st(std::string &r1, std::string &r2, std::string s, char c = ' ');

bool str2bool(std::string s);

std::string file2s(std::filesystem::path p);
std::wstring file2ws(std::filesystem::path p);
std::filesystem::path file2p(std::filesystem::path p);

bool file2bin(std::filesystem::path, std::vector<std::uint8_t> &);
bool bin2file(std::filesystem::path, std::vector<std::uint8_t> &);

std::string rtrim(std::string &s, const std::string ws = " \t\n\r\f\v");
std::string ltrim(std::string &s, const std::string ws = " \t\n\r\f\v");
std::string trim(std::string &s, const std::string ws = " \t\n\r\f\v");
std::wstring trim(std::wstring &s, const std::wstring wss = L" \t\n\r\f\v");
std::string ctrim(std::string s, const std::string ws = " \t\n\r\f\v");
std::wstring ctrim(std::wstring s, const std::wstring wss = L" \t\n\r\f\v");
std::string to_upper(const std::string);

std::string ISO8601();

bool any_of_ctype(const std::string, std::function<int(int)>);
bool all_of_ctype(const std::string, std::function<int(int)>);

std::string temppath();
std::string tempfile(std::string tpath = "", std::string pfx = "", unsigned int uniq = 0);

std::string exec_cmd(std::string cmd);
std::string wexec_cmd(std::wstring cmd);
bool shell_cmd(std::string cmd, std::string &output, bool show_window = false);
bool wshell_cmd(std::wstring cmd, std::string &output, bool show_window = false);
std::string pipe2s(const std::string command);
std::string wpipe2s(const std::wstring command);

std::string join(const std::vector<std::string> &, const std::string = ",");

void replace_all(std::string &s, const std::string srch, const std::string repl);
void replace_all(std::wstring &s, const std::wstring srch, const std::wstring repl);

std::string creplace_all(const std::string s, const std::string srch, const std::string repl);
std::wstring creplace_all(const std::wstring s, const std::wstring srch, const std::wstring repl);

std::string rep_crlf(std::string s);
std::string rep_bs(std::string &s);

void split_cmd_and_params(const std::string s, std::string &prog, std::string &params);
void split_cmd_and_params(const std::wstring s, std::wstring &prog, std::wstring &params);

// Filesystem api exposed to javascript
std::string pwd();
std::string cwd(std::string new_dir = "");

std::string to_js_oct(unsigned int number);

// Convert a file time to a string, default format is ISO8601 and default time zone is local
std::string file_time_to_string(std::filesystem::file_time_type file_time, std::string fmt = "%Y-%m-%d %H:%M:%S", bool gm = false);
std::string lastwrite(std::filesystem::path p);

bool is_number(const std::string &s);

bool is_base64(const std::string &s);

#endif /* UTIL_H */
