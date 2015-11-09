#include "onig-scanner.h"
#include "onig-string-context.h"
#include "onig-reg-exp.h"
#include "onig-result.h"
#include "onig-scanner-worker.h"
#include "unicode-utils.h"

using ::v8::Function;
using ::v8::FunctionTemplate;
using ::v8::HandleScope;
using ::v8::Local;
using ::v8::Null;

void OnigScanner::Init(Handle<Object> target) {
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>(OnigScanner::New);
  tpl->SetClassName(NanNew<String>("OnigScanner"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set(NanNew<String>("_findNextMatch"), NanNew<FunctionTemplate>(OnigScanner::FindNextMatch)->GetFunction());
  tpl->PrototypeTemplate()->Set(NanNew<String>("_findNextMatchSync"), NanNew<FunctionTemplate>(OnigScanner::FindNextMatchSync)->GetFunction());

  target->Set(NanNew<String>("OnigScanner"), tpl->GetFunction());
}

NODE_MODULE(onig_scanner, OnigScanner::Init)

NAN_METHOD(OnigScanner::New) {
  NanScope();
  OnigScanner* scanner = new OnigScanner(Local<Array>::Cast(args[0]));
  scanner->Wrap(args.This());
  NanReturnUndefined();
}

NAN_METHOD(OnigScanner::FindNextMatchSync) {
  NanScope();
  OnigScanner* scanner = node::ObjectWrap::Unwrap<OnigScanner>(args.This());
  NanReturnValue(scanner->FindNextMatchSync(Local<String>::Cast(args[0]), Local<Number>::Cast(args[1])));
}

NAN_METHOD(OnigScanner::FindNextMatch) {
  NanScope();
  OnigScanner* scanner = node::ObjectWrap::Unwrap<OnigScanner>(args.This());
  scanner->FindNextMatch(Local<String>::Cast(args[0]), Local<Number>::Cast(args[1]), Local<Function>::Cast(args[2]));
  NanReturnUndefined();
}

OnigScanner::OnigScanner(Handle<Array> sources) {
  int length = sources->Length();
  regExps.resize(length);

  for (int i = 0; i < length; i++) {
    String::Utf8Value utf8Value(sources->Get(i));
    regExps[i] = shared_ptr<OnigRegExp>(new OnigRegExp(string(*utf8Value), i));
  }

  searcher = shared_ptr<OnigSearcher>(new OnigSearcher(regExps));
  asyncCache = shared_ptr<OnigCache>(new OnigCache(length));
}

OnigScanner::~OnigScanner() {}

void OnigScanner::FindNextMatch(Handle<String> v8String, Handle<Number> v8StartLocation, Handle<Function> v8Callback) {
  int charOffset = v8StartLocation->Value();
  NanCallback *callback = new NanCallback(v8Callback);
  shared_ptr<OnigStringContext> source = shared_ptr<OnigStringContext>(new OnigStringContext(v8String));


  OnigScannerWorker *worker = new OnigScannerWorker(callback, regExps, source, charOffset, asyncCache);
  NanAsyncQueueWorker(worker);
}

Handle<Value> OnigScanner::FindNextMatchSync(Handle<String> v8String, Handle<Number> v8StartLocation) {
  if (!lastSource || !lastSource->IsSame(v8String))
    lastSource = shared_ptr<OnigStringContext>(new OnigStringContext(v8String));
  int charOffset = v8StartLocation->Value();

  shared_ptr<OnigResult> bestResult = searcher->Search(lastSource, charOffset);
  if (bestResult != NULL) {
    Local<Object> result = NanNew<Object>();
    result->Set(NanNew<String>("index"), NanNew<Number>(bestResult->Index()));
    result->Set(NanNew<String>("captureIndices"), CaptureIndicesForMatch(bestResult.get(), lastSource));
    return result;
  } else {
    return NanNull();
  }
}

Handle<Value> OnigScanner::CaptureIndicesForMatch(OnigResult* result, shared_ptr<OnigStringContext> source) {
  int resultCount = result->Count();
  Local<Array> captures = NanNew<Array>(resultCount);

  for (int index = 0; index < resultCount; index++) {
    int captureLength = result->LengthAt(index);
    int captureStart = result->LocationAt(index);

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

  return captures;
}
