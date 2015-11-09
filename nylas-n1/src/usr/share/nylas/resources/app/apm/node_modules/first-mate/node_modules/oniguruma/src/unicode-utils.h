#ifndef SRC_UNICODE_UTILS_H_
#define SRC_UNICODE_UTILS_H_

#include <string>

class UnicodeUtils {
 public:
  static int characters_in_bytes(const char* string, int bytes);
#ifdef _WIN32
  static int bytes_in_characters(const wchar_t* string, int bytes);
#else
  static int bytes_in_characters(const char* string, int bytes);
#endif
};

#endif  // SRC_UNICODE_UTILS_H_
