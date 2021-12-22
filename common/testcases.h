#ifndef TESTCASES_H
#define TESTCASES_H

#include "loader.h"
#include "profile.h"
#include <boost/ut.hpp>
#include <trie_concepts.h>

template <xtrie::IsTrie TrieClass, typename Serializer = void,
          typename T = char>
static TrieClass test_lexicon(const char *path, const T *test_case) {
  using namespace boost::ut;
  using namespace xtrie;

  std::cout << path << std::endl;

  auto words = load_lexicon(path);
  std::sort(words.begin(), words.end());

  TrieClass trie;

  auto mem0 = get_mem_info();

  for (auto &w : words) {
    trie.add(w, 0);
  }

  if constexpr (IsStaticTrie<TrieClass>) {
    trie.end_build();
  }

  printf("Memory usage: %zd bytes\n", get_mem_delta(mem0, get_mem_info()));

  auto res = trie.traverse(reinterpret_cast<const char *>(test_case));
  expect(res.matched());
  expect(trie.value_at(res.state()) == 0);

  if constexpr (IsSerializableTrie<TrieClass>) {
    std::ofstream ofs(std::string(path) + ".bin",
                      std::ios::binary | std::ios::trunc);
    trie.save(ofs, Serializer{});
  }

  return trie;
}

template <xtrie::IsTrie TrieClass, typename Serializer = void>
static void add_common_tests() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  /*
  "test h->i and h->e"_test = [] {
    TrieClass trie;

    std::vector<std::string> words{"hi", "hello", "mello"};
    std::sort(words.begin(), words.end());

    for (auto &w : words) {
      trie.add(w, 0);
    }

    if constexpr (IsStaticTrie<TrieClass>) {
      trie.end_build();
    }

    auto h_res = trie.traverse("h");
    std::string h_trans;

    for (auto it = h_res.node->trans_begin(); it != h_res.node->trans_end();
         ++it) {
      h_trans.push_back(it.key());
    }

    std::sort(h_trans.begin(), h_trans.end());
    expect(h_trans == "ei");
  };
  */

  "test build en_1k.txt"_test = [] {
    test_lexicon<TrieClass, Serializer>(DATA_DIR "en_1k.txt", u8"aborticide");
  };

  "test build en_466k.txt"_test = [] {
    test_lexicon<TrieClass, Serializer>(DATA_DIR "en_466k.txt", u8"scordature");
  };

  "test build zh_cn_406k.txt"_test = [] {
    test_lexicon<TrieClass, Serializer>(DATA_DIR "zh_cn_406k.txt", u8"李祥霆");
  };
}

#endif
