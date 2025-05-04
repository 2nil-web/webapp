
#include <iostream>
#include <string>
#include <vector>

bool markdown = false;
bool size_sortable = false;

std::string format_sz(std::string t, size_t sz)
{
  std::string s = {};

  if (size_sortable)
  {
    if (sz < 10)
      s += ' ';
    s += std::to_string(sz);
    s += ' ' + t;
  }
  else
  {
    s = t + std::to_string(sz);
    size_t l = s.size();
    //  s=std::to_string(l)+", "+s;
    s = t + std::string(26 - l, '_') + std::to_string(sz);
  }
  return s;
}

std::string out(std::string s)
{
  if (markdown)
    return "<pre>" + s + "</pre>\n";
  else
    return s + '\n';
}

#define str(s) #s
#define format_sizeof(TYPE) format_sz(str(TYPE), sizeof(TYPE))
#define output_type(TYPE) s += out(format_sizeof(TYPE))

std::string type_hierarchy(bool _markdown, bool _size_sortable)
{
  markdown = _markdown;
  size_sortable = _size_sortable;
  std::string s = {};

  if (!size_sortable)
  {
    s += "# Size of known C++ types (in octets)\n";
#ifdef _WIN32
    s += "For win32api\n";
#else
    s += "For Linux\n";
#endif
    s += "\n";
    s += "*The hierarchy order is based on 'Type Classification'*\n";
    s += "*As made by Howard Hinnant in Nov. 8, 2007*\n";
    s += "*see: http://howardhinnant.github.io/TypeHiearchy.pdf*\n";
    s += "\n";
    if (markdown)
      s += "## ";
    else
      s += "1) ";
    s += "fundamental\n"; //  # arithmetic and nullptr_t are also scalar
  }

#ifdef AVOID_GCC_WARNING // _MSC_VER
  output_type(void);     // Although not compiling with msvc, it only triggers a warning with gcc
#endif

  if (!size_sortable)
  {
    if (markdown)
      s += "### ";
    else
      s += "2) ";
    s += "arithmetic";
    if (markdown)
      s += " *";
    else
      s += ' ';
    s += "(is also scalar)";
    if (markdown)
      s += '*';
    s += "\n";
  }

  if (!size_sortable)
  {
    if (markdown)
      s += "#### ";
    else
      s += "3) ";
    s += "floating_point\n";
  }

  output_type(float);
  output_type(double);
  output_type(long double);

  if (!size_sortable)
  {
    if (markdown)
      s += "##### ";
    else
      s += "4) ";
    s += "integral\n";
  }

  //  output_type(extended unsigned integer);
  //  output_type(extended signed integer);
  output_type(bool);
  output_type(char16_t);
  output_type(char);
  output_type(unsigned char);
  output_type(unsigned int);
  output_type(int);
  output_type(char32_t);
  output_type(signed char);
  output_type(unsigned long long);
  output_type(unsigned short);
  output_type(wchar_t);
  output_type(short);
  output_type(long long);
  output_type(unsigned long);
  output_type(long);

  if (markdown)
    s += "<pre>";
  s += format_sizeof(nullptr);
  if (!markdown)
    s += " ";
  if (!size_sortable)
    s += "(is also scalar)";
  if (markdown)
    s += "</pre>";
  s += "\n";

  //  output_type(nullptr_t); // is also scalar
  // #  object
  // ##  scalar # arithmetic and nullptr_t are also fundamentals
  // ### member pointer
  //  output_type(member object pointer);
  //  output_type(member function pointer);
  // ### pointer # is also compound
  //  output_type(enum); // # is also compound
  // ### array # is also compound
  //  output_type(union); // # is also compound
  //  output_type(class); // # is also compound
  // #  compound
  // ## reference
  // ### lvalue reference
  // ### rvalue reference
  // ## function

  return s;
}

#ifdef ALONE
int main(int argc, char **argv)
{
  std::vector<std::string> args(argv, argv + argc);
  bool md = false, sz_s = false;
  if (args.size() > 1)
  {
    if (args[1] == "m" || args[1] == "markdown")
      md = true;
    if (args[1] == "s" || args[1] == "sortable")
      sz_s = true;
  }

  if (args.size() > 2)
  {
    if (args[2] == "m" || args[2] == "markdown")
      md = true;
    if (args[2] == "s" || args[2] == "sortable")
      sz_s = true;
  }

  std::cout << type_hierarchy(md, sz_s);
}
#endif
