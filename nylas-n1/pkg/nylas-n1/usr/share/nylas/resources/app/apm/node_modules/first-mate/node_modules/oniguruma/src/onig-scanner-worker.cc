#include "onig-scanner-worker.h"
#include "onig-string-context.h"
#include "unicode-utils.h"

using ::v8::Array;
using ::v8::Local;
using ::v8::Null;
using ::v8::Number;
using ::v8::Object;
using ::v8::String;
using ::v8::Value;

void OnigScannerWorker::Execute() {
  bestResult = searcher->Search(source, charOffset);
}

void OnigScannerWorker::HandleOKCallback() {
  NanScope();

  // Try to reuse the cached results across async searches
  cache->Reset(searcher->GetCache());

  if (bestResult != NULL) {
    Local<Object> result = NanNew<Object>();
    result->Set(NanNew<String>("index"), NanNew<Number>(bestResult->Index()));

    int resultCount = bestResult->Count();
    Local<Array> captures = NanNew<Array>(resultCount);
    for (int index = 0; index < resultCount; index++) {
      int captureLength = bestResult->LengthAt(index);
      int captureStart = bestResult->LocationAt(index);

      if (source->has_multibyte_characters()) {
        captureLength = UnicodeUtils::characters_in_bytes(source->utf8_value() + captureStart, captureLength);
        captureStart = UnicodeUtils::characters_in_bytes(source->utf8_value(), captureStart);
      }

      Local<Object> capture = NanNew<Object>();
      capture->Set(NanNew<String>("index"), NanNew<Number>(index));
      capture->Set(NanNew<String>("start"), NanNew<Number>(captureStart));
      capture->Set(NanNew<String>("end"), NanNew<Number>(captureStart + captureLength));
      capture->Set(NanNew<String>("length"), NanNew<Number>(captureLength));
      captures->Set(index, capture);
    }
    result->Set(NanNew<String>("captureIndices"), captures);

    Local<Value> argv[] = {
      NanNull(),
      result
    };
    callback->Call(2, argv);
  } else {
    Local<Value> argv[] = {
      NanNull(),
      NanNull()
    };
    callback->Call(2, argv);
  }
}
