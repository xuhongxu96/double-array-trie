#include "hashtrie.h"
#include <algorithm>
#include <boost/ut.hpp>
#include <vector>

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;

  "test hashtrie node transition iterator"_test = [] {
    HashTrie trie;
    trie.add("hello", 1);
    trie.add("hi", 1);

    auto h_res = trie.traverse("h");
    std::string h_trans;

    for (auto it = h_res.node->trans_begin(); it != h_res.node->trans_end();
         ++it) {
      h_trans.push_back(it.key());
    }

    std::sort(h_trans.begin(), h_trans.end());
    expect(h_trans == "ei");
  };

  return 0;
}