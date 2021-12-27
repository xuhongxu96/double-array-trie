#include <boost/ut.hpp>
#include <darts/darts.h>
#include <iostream>
#include <memory>
#include <string_view>
#include <testcases.h>
#include <trie_concepts.h>
#include <vector>

namespace xtrie {

class DartsWrapper {
public:
  using value_type = int;
  static constexpr value_type DEFAULT_VALUE = -1;
  using state_type = std::pair<size_t, int>;

  class TraverseResult {
  public:
    state_type state() const { return {node_, res_}; }
    bool matched() const { return res_ != -2; }
    uint32_t matched_length() const { return matched_length_; }
    int res() const { return res_; }

  private:
    size_t node_;
    int res_;
    uint32_t matched_length_;

    TraverseResult(size_t node, int res, uint32_t matched_length)
        : node_(node), res_(res), matched_length_(matched_length) {}

    friend class DartsWrapper;
  };

  DartsWrapper() : build_(std::make_unique<BuildInfo>()) {}

  void add(std::string_view sv, value_type) {
    build_->words.emplace_back(sv.begin(), sv.end());
  }

  void end_build() {
    std::vector<const char *> keys;
    keys.reserve(build_->words.size());

    for (auto &w : build_->words) {
      keys.push_back(w.c_str());
    }

    da_.build(keys.size(), keys.data());

    build_.reset(nullptr);
  }

  TraverseResult traverse(std::string_view prefix,
                          state_type start = {0, 0}) const {
    size_t key_pos = 0;
    size_t node_pos = start.first;
    int res = da_.traverse(prefix.data(), node_pos, key_pos, prefix.size());
    return {node_pos, res, static_cast<uint32_t>(key_pos)};
  }

  bool has_value_at(state_type state) const { return state.second >= 0; }

private:
  struct BuildInfo {
    std::vector<std::string> words;
  };

  Darts::DoubleArray da_;
  std::unique_ptr<BuildInfo> build_;
};

#ifdef ASSERT_CONCEPT
static_assert(IsStaticTrieBuilder<DartsWrapper>);
static_assert(IsTrie<DartsWrapper>);
#endif

class KVDartsWrapper {
public:
  using value_type = int;
  static constexpr value_type DEFAULT_VALUE = -1;
  using state_type = std::pair<size_t, int>;

  class TraverseResult {
  public:
    state_type state() const { return {node_, res_}; }
    bool matched() const { return res_ != -2; }
    uint32_t matched_length() const { return matched_length_; }
    int res() const { return res_; }

  private:
    size_t node_;
    int res_;
    uint32_t matched_length_;

    TraverseResult(size_t node, int res, uint32_t matched_length)
        : node_(node), res_(res), matched_length_(matched_length) {}

    friend class KVDartsWrapper;
  };

  KVDartsWrapper() : build_(std::make_unique<BuildInfo>()) {}

  void add(std::string_view sv, value_type v) {
    build_->words.emplace_back(sv.begin(), sv.end());
    build_->values.push_back(v);
  }

  void end_build() {
    std::vector<const char *> keys;
    keys.reserve(build_->words.size());

    for (auto &w : build_->words) {
      keys.push_back(w.c_str());
    }

    da_.build(keys.size(), keys.data(), nullptr, build_->values.data());

    build_.reset(nullptr);
  }

  TraverseResult traverse(std::string_view prefix,
                          state_type start = {0, 0}) const {
    size_t key_pos = 0;
    size_t node_pos = start.first;
    int res = da_.traverse(prefix.data(), node_pos, key_pos, prefix.size());
    return {node_pos, res, static_cast<uint32_t>(key_pos)};
  }

  bool has_value_at(state_type state) const { return state.second >= 0; }
  const value_type &value_at(state_type state) const { return state.second; }

private:
  struct BuildInfo {
    std::vector<std::string> words;
    std::vector<int> values;
  };

  Darts::DoubleArray da_;
  std::unique_ptr<BuildInfo> build_;
};

#ifdef ASSERT_CONCEPT
static_assert(IsStaticTrieBuilder<KVDartsWrapper>);
static_assert(IsKVTrie<KVDartsWrapper>);
#endif

} // namespace xtrie

int main() {
  using namespace boost::ut;
  using namespace boost::ut::literals;
  using namespace boost::ut::operators::terse;
  using namespace xtrie;

//  add_common_tests<DartsWrapper>();
  add_common_tests<KVDartsWrapper>();

  return 0;
}