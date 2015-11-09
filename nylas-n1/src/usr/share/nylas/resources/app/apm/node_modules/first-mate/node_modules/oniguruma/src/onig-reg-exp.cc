#include "onig-reg-exp.h"
#include "onig-result.h"

using ::v8::Exception;
using ::v8::String;

OnigRegExp::OnigRegExp(const string& source, int indexInScanner)
    : source_(source),
      regex_(NULL),
      indexInScanner(indexInScanner) {
  OnigErrorInfo error;
  const UChar* sourceData = (const UChar*)source.data();
  int status = onig_new(&regex_, sourceData, sourceData + source.length(),
                        ONIG_OPTION_CAPTURE_GROUP, ONIG_ENCODING_UTF8,
                        ONIG_SYNTAX_DEFAULT, &error);

  if (status != ONIG_NORMAL) {
    UChar errorString[ONIG_MAX_ERROR_MESSAGE_LEN];
    onig_error_code_to_str(errorString, status, &error);
    NanThrowError(Exception::Error(NanNew<String>(reinterpret_cast<char*>(errorString))));
  }
}

OnigRegExp::~OnigRegExp() {
  if (regex_) onig_free(regex_);
}

bool OnigRegExp::Contains(const string& value) {
  return source_.find(value) != string::npos;
}

shared_ptr<OnigResult> OnigRegExp::Search(const string& searchString,
                                          size_t position) {
  return Search(searchString.data(), position, searchString.size());
}

shared_ptr<OnigResult> OnigRegExp::Search(const char* data,
                                          size_t position, size_t end) {
  if (!regex_) {
    NanThrowError(Exception::Error(NanNew<String>("RegExp is not valid")));
    return shared_ptr<OnigResult>();
  }

  const UChar* searchData = reinterpret_cast<const UChar*>(data);
  OnigRegion* region = onig_region_new();
  int status = onig_search(regex_, searchData, searchData + end,
                           searchData + position, searchData + end, region,
                           ONIG_OPTION_NONE);

  if (status != ONIG_MISMATCH) {
    return shared_ptr<OnigResult>(new OnigResult(region, indexInScanner));
  } else {
    onig_region_free(region, 1);
    return shared_ptr<OnigResult>();
  }
}
