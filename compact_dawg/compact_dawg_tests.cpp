#include "compact_dawg.h"
#include <boost/ut.hpp>
#include <iostream>
#include <testcases.h>

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;
  //"test"_test = [] {
  //  CompactDAWG dawg;

  //  std::vector<std::string> words{"1-point", "2-point", "abcdef"};
  //  std::sort(words.begin(), words.end());

  //  for (auto &w : words) {
  //    dawg.add(w, 0);
  //  }
  //  dawg.end_build();

  //  for (auto &w : words)
  //    expect(dawg.value_at(dawg.traverse(w).state()) == 0);
  //};

  add_common_tests<CompactDAWG<>>();

  //"test 'e' node is shared btw 'he' and 'me'"_test = [] {
  //  CompactDAWG dawg;

  //  std::vector<std::string> words{"hi", "hello", "mello"};
  //  std::sort(words.begin(), words.end());

  //  for (auto &w : words) {
  //    dawg.add(w, 0);
  //  }
  //  dawg.end_build();

  //  auto node_he = dawg.traverse("he").state();
  //  auto node_me = dawg.traverse("me").state();
  //  expect(node_he == node_me);
  //};

  return 0;
}
