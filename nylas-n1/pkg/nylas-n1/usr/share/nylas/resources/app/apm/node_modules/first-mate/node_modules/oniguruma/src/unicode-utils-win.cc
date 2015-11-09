#include "unicode-utils.h"

#include <windows.h>

int UnicodeUtils::characters_in_bytes(const char* string, int bytes) {
  if (bytes > 0)
    return MultiByteToWideChar(CP_UTF8, 0, string, bytes, NULL, 0);
  else
    return 0;
}

int UnicodeUtils::bytes_in_characters(const wchar_t* string, int characters) {
  if (characters > 0)
    return WideCharToMultiByte(CP_UTF8, 0, string, characters, NULL, 0, NULL, NULL);
  else
    return 0;
}
