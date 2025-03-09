#include <algorithm>
#include <cctype>
#include <iterator>
#include <string>

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

namespace Compiler::Utils {
// trim from start (in place)
inline void ltrim(std::string& str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string& str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(),
        str.end());
}

// trim from both ends (not in place)
inline std::string trim(std::string str)
{
    rtrim(str);
    ltrim(str);

    return str;
}

inline std::string to_uppercase(const std::string& str)
{
    std::string result;
    for (char ch : str) {
        result.push_back(std::toupper(ch));
    }
    return result;
}

/*
inline std::string identify_str(std::iterator  *it) {
     std::string str;
     while (*it != '"') {
          str.push_back(*it);
          std::next(it);
     }
     return str;
}
inline double identify_number(auto *it) {
     std::string num;
     while (std::isdigit(*it)) { // OR is . if I implement floating point numbers
          num.push_back(*it);
          std::next(it);
     }
     return strtod(num.c_str(), nullptr);
}
*/

} // namespace Compiler::Utils

#endif // !STRING_UTILS_H
