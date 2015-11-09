#ifndef SRC_ONIG_CACHE_H_
#define SRC_ONIG_CACHE_H_

#include "onig-reg-exp.h"
#include "onig-result.h"
#include <memory>
#include <string>
#include <vector>

using ::std::shared_ptr;
using ::std::string;
using ::std::vector;

class OnigStringContext;

class OnigCache {
 public:
  explicit OnigCache(int maxSize) :
    maxSize(maxSize),
    maxCachedIndex(-1),
    lastStartLocation(-1),
    useCache(false) {
    results.resize(maxSize);
  }

  ~OnigCache() {}

  void Clear();
  void Init(shared_ptr<OnigStringContext> searchString, int byteOffset);
  void Reset(const OnigCache& cache);
  shared_ptr<OnigResult> Search(OnigRegExp *regExp, shared_ptr<OnigStringContext> searchString, int byteOffset);

 private:
  vector<shared_ptr<OnigResult>> results;
  int maxSize;
  shared_ptr<OnigStringContext> lastMatchedString;
  int maxCachedIndex;
  int lastStartLocation;
  bool useCache;
};

#endif  // SRC_ONIG_CACHE_H_
