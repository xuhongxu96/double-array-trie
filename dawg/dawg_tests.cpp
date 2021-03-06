#include "dawg.h"
#include <boost/ut.hpp>
#include <iostream>
#include <testcases.h>

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  "test bug 1"_test = [] {
    // Bug: two terminals in a suffix "ds" or two branches "d" and "s"
    // may be serialized into the same id string before
    DAWG dawg;

    std::vector<std::string> words{"abattised", "abattises", "abfarad",
                                   "abfarads"};
    std::sort(words.begin(), words.end());

    for (auto &w : words) {
      dawg.add(w, 0);
    }
    dawg.end_build();
    expect(dawg.traverse("abfarads").matched());
  };

  "test 'e' node is shared btw 'he' and 'me'"_test = [] {
    DAWG dawg;

    std::vector<std::string> words{"hi", "hello", "mello"};
    std::sort(words.begin(), words.end());

    for (auto &w : words) {
      dawg.add(w, 0);
    }
    dawg.end_build();

    auto node_he = dawg.traverse("he").state();
    auto node_me = dawg.traverse("me").state();
    expect(node_he == node_me);
  };

  add_common_tests<DAWG<>>();
  add_common_tests<DAWG<>>(true);

  return 0;
}
