#include "hashtrie.h"
#include <boost/ut.hpp>
#include <testcases.h>
#include <vector>

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  add_common_tests<HashTrie<>>();

  return 0;
}