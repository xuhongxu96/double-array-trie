#ifndef TESTCASES_H
#define TESTCASES_H

#include "loader.h"
#include "profile.h"
#include <boost/ut.hpp>
#include <string>
#include <trie_concepts.h>

template <xtrie::IsTrieBuilder TrieBuilder, typename Serializer = void>
class BuilderCommonTests {
public:
  BuilderCommonTests(std::string filename) : filename_(std::move(filename)) {}

  void build_dict() {
    using namespace xtrie;

    std::cout << filename_ << std::endl;

    auto words = load_lexicon((std::string(DATA_DIR) + filename_).c_str());
    std::sort(words.begin(), words.end());

    auto mem0 = get_mem_info();

    for (auto &w : words) {
      builder_.add(w, 0);
    }

    if constexpr (IsStaticTrieBuilder<TrieBuilder>) {
      builder_.end_build();
    }

    printf("Memory usage by builder: %zd bytes\n",
           get_mem_delta(mem0, get_mem_info()));
  }

  std::string serialize() const {
    using namespace xtrie;

    std::string path = DATA_DIR + filename_ + ".bin";

    if constexpr (IsSerializableTrieBuilder<TrieBuilder>) {
      std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
      builder_.save(ofs, Serializer{});
    }

    return path;
  }

  template <class TChar> bool contains(const TChar *str) const {
    using namespace xtrie;

    if constexpr (IsTrie<TrieBuilder>) {
      auto res = builder_.traverse(reinterpret_cast<const char *>(str));
      return res.matched() && builder_.has_value_at(res.state());
    } else {
      return true;
    }
  }

private:
  TrieBuilder builder_;
  std::string filename_;
};

template <xtrie::IsDeserializableTrie Trie,
          xtrie::IsSerializableTrieBuilder TrieBuilder,
          typename Serializer = void>
class SerializableTrieCommonTests {
public:
  SerializableTrieCommonTests(std::string filename)
      : builder_(std::move(filename)) {}

  void build() {
    builder_.build_dict();
    auto bin_path = builder_.serialize();
    std::ifstream ifs(bin_path, std::ios::binary);

    auto mem0 = get_mem_info();

    trie_.load(ifs);

    printf("Memory usage by trie: %zd bytes\n",
           get_mem_delta(mem0, get_mem_info()));
  }

  template <class TChar> bool contains(const TChar *str) const {
    using namespace xtrie;

    auto res = trie_.traverse(reinterpret_cast<const char *>(str));
    return res.matched() && trie_.has_value_at(res.state());
  }

private:
  BuilderCommonTests<TrieBuilder, Serializer> builder_;
  Trie trie_;
};

template <xtrie::IsTrieBuilder TrieBuilder, typename Serializer = void>
static void add_common_tests() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  using test_type = BuilderCommonTests<TrieBuilder, Serializer>;

  "test en_1k.txt"_test = [] {
    test_type test("en_1k.txt");
    test.build_dict();
    test.serialize();
    expect(test.contains(u8"aborticide"));
  };

  "test en_466k.txt"_test = [] {
    test_type test("en_466k.txt");
    test.build_dict();
    test.serialize();
    expect(test.contains(u8"scordature"));
  };

  "test zh_cn_406k.txt"_test = [] {
    test_type test("zh_cn_406k.txt");
    test.build_dict();
    test.serialize();
    expect(test.contains(u8"李祥霆"));
  };
}

template <xtrie::IsDeserializableTrie Trie,
          xtrie::IsSerializableTrieBuilder TrieBuilder,
          typename Serializer = void>
static void add_common_serializable_trie_tests() {

  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  using test_type = SerializableTrieCommonTests<Trie, TrieBuilder, Serializer>;

  "test en_1k.txt"_test = [] {
    test_type test("en_1k.txt");
    test.build();
    expect(test.contains(u8"aborticide"));
  };

  "test en_466k.txt"_test = [] {
    test_type test("en_466k.txt");
    test.build();
    expect(test.contains(u8"scordature"));
  };

  "test zh_cn_406k.txt"_test = [] {
    test_type test("zh_cn_406k.txt");
    test.build();
    expect(test.contains(u8"李祥霆"));
  };
}

#endif
