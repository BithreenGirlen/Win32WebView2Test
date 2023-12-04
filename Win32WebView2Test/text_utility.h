#ifndef TEXT_UTILITY_H_
#define TEXT_UTILITY_H_

#include <string>

std::wstring WidenUtf8(const std::string &str);
std::string NarrowUtf8(const std::wstring &wstr);

#endif //TEXT_UTILITY_H_
