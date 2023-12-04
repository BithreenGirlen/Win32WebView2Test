#ifndef JSON_SERIALISATION_H_
#define JSON_SERIALISATION_H_

#include <string>

std::wstring SerialiseJsonString(const std::wstring &src);
std::wstring DeserialiseJsonString(const std::wstring &src);

#endif //JSON_SERIALISATION_H_
