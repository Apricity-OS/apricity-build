#ifndef SRC_ONIG_SEARCHER_H_
#define SRC_ONIG_SEARCHER_H_

#include <string>
#include <vector>
#include "onig-cache.h"
#include "onig-reg-exp.h"
#include "onig-result.h"

using ::std::string;
using ::std::shared_ptr;
using ::std::vector;

class OnigStringContext;

class OnigSearcher {
 public:
  explicit OnigSearcher(vector<shared_ptr<OnigRegExp>> regExps)
    : regExps(regExps),
      cache(regExps.size()) {}

  OnigSearcher(vector<shared_ptr<OnigRegExp>> regExps, OnigCache cache)
    : regExps(regExps),
      cache(cache) {}

  ~OnigSearcher() {}

  const OnigCache& GetCache() { return cache; }
  shared_ptr<OnigResult> Search(shared_ptr<OnigStringContext> source, int charOffset);

 private:
  vector<shared_ptr<OnigRegExp>> regExps;
  OnigCache cache;
};

#endif  // SRC_ONIG_SEARCHER_H_
