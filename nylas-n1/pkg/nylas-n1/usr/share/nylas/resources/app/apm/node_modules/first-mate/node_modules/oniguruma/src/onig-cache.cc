#include "onig-cache.h"
#include "onig-string-context.h"
#include "unicode-utils.h"

void OnigCache::Clear() {
  maxCachedIndex = -1;
  results.clear();
  results.resize(maxSize);
}

void OnigCache::Init(shared_ptr<OnigStringContext> stringToSearch, int byteOffset) {
  useCache = (stringToSearch == lastMatchedString && byteOffset >= lastStartLocation);
  lastStartLocation = byteOffset;

  if (!useCache) {
    Clear();
    lastMatchedString = stringToSearch;
  }
}

void OnigCache::Reset(const OnigCache& cache) {
  lastMatchedString = cache.lastMatchedString;
  maxCachedIndex = cache.maxCachedIndex;
  lastStartLocation = cache.lastStartLocation;
  results = cache.results;
}

shared_ptr<OnigResult> OnigCache::Search(OnigRegExp *regExp, shared_ptr<OnigStringContext> searchString, int byteOffset) {
  shared_ptr<OnigResult> result;
  int index = regExp->Index();
  bool useCachedResult = false;

  if (useCache && index <= maxCachedIndex) {
    result = results[index];
    useCachedResult = result == NULL || result->LocationAt(0) >= byteOffset;
  }

  if (!useCachedResult) {
    result = regExp->Search(searchString->utf8_value(), byteOffset, searchString->utf8_length());
    results[index] = result;
    if (index > maxCachedIndex) {
      maxCachedIndex = index;
    }
  }

  return result;
}
