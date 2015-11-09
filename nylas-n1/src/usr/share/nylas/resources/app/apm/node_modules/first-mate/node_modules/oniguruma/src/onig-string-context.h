#ifndef SRC_ONIG_STRING_CONTEXT_H_
#define SRC_ONIG_STRING_CONTEXT_H_

#include <string>
#include "nan.h"

using ::v8::Handle;
using ::v8::Isolate;
using ::v8::Persistent;
using ::v8::String;
using ::v8::Value;

using ::std::string;

class OnigStringContext {
 public:
  explicit OnigStringContext(Handle<String> v8String);
  bool IsSame(Handle<String> other) const;
  bool HasMultibyteCharacters() const;
  const char* utf8_value() const { return *utf8Value; }
  size_t utf8_length() const { return utf8Value.length(); }
  ~OnigStringContext();

#ifdef _WIN32
  const wchar_t* utf16_value() const { return reinterpret_cast<const wchar_t*>(*utf16Value); }
  size_t utf16_length() const { return utf16Value.length(); }
#endif
  bool has_multibyte_characters() const { return hasMultibyteCharacters; }

 private:
  Persistent<String> v8String;
  String::Utf8Value utf8Value;
#ifdef _WIN32
  String::Value utf16Value;
#endif
  bool hasMultibyteCharacters;
};
#endif  // SRC_ONIG_STRING_CONTEXT_H_
