#include "compact_datrie.h"
#include <boost/ut.hpp>
#include <fstream>
#include <iostream>
#include <profile.h>

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  "test load"_test = [] {
    CompactDoubleArrayTrie trie;
    auto mem = get_mem_info();
    {
      std::ifstream ifs(DATA_DIR "en_466k.txt.bin", std::ios::binary);
      trie.load(ifs);
    }
    auto d = get_mem_delta(mem, get_mem_info());
    std::cout << d << std::endl;
  };
}