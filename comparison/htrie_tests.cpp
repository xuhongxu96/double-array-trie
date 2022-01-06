#define NOMINMAX

#include <boost/ut.hpp>
#include <iostream>
#include <memory>
#include <string_view>
#include <testcases.h>
#include <trie_concepts.h>
#include <tsl/htrie_map.h>
#include <vector>

namespace xtrie {

class KVHTrieWrapper {
public:
  using value_type = int;
  static constexpr value_type DEFAULT_VALUE = -1;
  using state_type = tsl::htrie_map<char, int>::const_iterator;

  class TraverseResult {
  public:
    state_type state() const { return it_; }
    bool matched() const { return it_ != end_; }
    uint32_t matched_length() const { return 0; }
    int res() const { return it_.value(); }

  private:
    state_type it_;
    state_type end_;

    TraverseResult(state_type it, state_type end) : it_(it), end_(end) {}

    friend class KVHTrieWrapper;
  };

  KVHTrieWrapper() {}

  void add(std::string_view sv, value_type v) { map_.insert(sv, v); }

  TraverseResult traverse(std::string_view prefix,
                          state_type start = {}) const {
    return {map_.find(prefix), map_.end()};
  }

  bool has_value_at(state_type state) const { return state != map_.end(); }
  const value_type &value_at(state_type state) const { return state.value(); }

private:
  tsl::htrie_map<char, int> map_;
};

#ifdef ASSERT_CONCEPT
static_assert(IsTrieBuilder<KVHTrieWrapper>);
static_assert(IsKVTrie<KVHTrieWrapper>);
#endif

} // namespace xtrie

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

  "test"_test = [] {
    KVHTrieWrapper darts;
    darts.add("ab", 1);
    darts.add("abc", 2);

    auto res = darts.traverse("ab");
    expect(darts.value_at(res.state()) == 1);
  };

  add_common_tests<KVHTrieWrapper>(true);

  return 0;
}