#ifndef SRC_ONIG_REG_EXP_H_
#define SRC_ONIG_REG_EXP_H_

#include <memory>
#include <string>

#include "nan.h"
#include "oniguruma.h"

using ::std::shared_ptr;
using ::std::string;

class OnigResult;

class OnigRegExp {
 public:
  explicit OnigRegExp(const string& source, int indexInScanner);
  ~OnigRegExp();

  bool Contains(const string& value);
  int LocationAt(int index);
  int Index() { return indexInScanner; }
  shared_ptr<OnigResult> Search(const string &searchString, size_t position);
  shared_ptr<OnigResult> Search(const char* data, size_t position, size_t end);

 private:
  OnigRegExp(const OnigRegExp&);  // Disallow copying
  OnigRegExp &operator=(const OnigRegExp&);  // Disallow copying

  string source_;
  regex_t* regex_;
  int indexInScanner;
};

#endif  // SRC_ONIG_REG_EXP_H_
