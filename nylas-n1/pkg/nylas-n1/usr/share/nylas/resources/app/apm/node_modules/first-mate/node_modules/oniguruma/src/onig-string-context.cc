#include "onig-string-context.h"
#include <node.h>  // To see NODE_MINOR_VERSION

OnigStringContext::OnigStringContext(Handle<String> str)
// The v8::Persistent API changed recently and now it requires v8::Isolate
// to the constructor argument.
#if (0 == NODE_MAJOR_VERSION && 11 <= NODE_MINOR_VERSION) || (1 <= NODE_MAJOR_VERSION)
  : v8String(Isolate::GetCurrent(), str),
#else
  : v8String(Persistent<String>::New(str)),
#endif
    utf8Value(str),
#ifdef _WIN32
    utf16Value(str),
#endif
    hasMultibyteCharacters(str->Length() != str->Utf8Length()) {
}

bool OnigStringContext::IsSame(Handle<String> other) const {
#if (0 == NODE_MAJOR_VERSION && 11 <= NODE_MINOR_VERSION) || (1 <= NODE_MAJOR_VERSION)
  return other->StrictEquals(v8::Local<String>::New(Isolate::GetCurrent(), v8String));
#else
  return other->StrictEquals(v8String);
#endif
}

OnigStringContext::~OnigStringContext() {
#if (0 == NODE_MAJOR_VERSION && 10 == NODE_MINOR_VERSION)
  v8String.Dispose();
#else
  v8String.Reset();
#endif
}
