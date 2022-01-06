#include "compact_datrie.h"
#include "datrie_builder.h"
#include "default_datrie.h"
#include "no_value_datrie.h"
#include "serializers/compact_serializer.h"
#include "serializers/default_serializer.h"
#include "serializers/no_value_serializer.h"
#include <boost/ut.hpp>
#include <fstream>
#include <iostream>
#include <profile.h>
#include <testcases.h>

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  //add_common_serializable_trie_tests<
  //    NoValueDoubleArrayTrie<>, DoubleArrayTrieBuilder<>, NoValueSerializer>();

  add_common_serializable_trie_tests<
      DefaultDoubleArrayTrie<>, DoubleArrayTrieBuilder<>, DefaultSerializer>(
      true);

  //add_common_serializable_trie_tests<CompactDoubleArrayTrie<>,
  //                                   DoubleArrayTrieBuilder<uint32_t, 0, true>,
  //                                   CompactSerializer>(true);
}