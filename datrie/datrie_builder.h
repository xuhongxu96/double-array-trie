#ifndef DATRIE_BUILDER_H
#define DATRIE_BUILDER_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <dawg.h>
#include <hashtrie.h>
#include <limits>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <vector>

#ifdef _WINDOWS
#include <intrin.h>

#pragma intrinsic(_BitScanForward64)
#pragma intrinsic(_BitScanReverse64)
#endif

#ifdef ASSERT_CONCEPT
#include <trie_concepts.h>
#endif

namespace xtrie {

namespace details {
template <typename, typename = void> struct has_end_build : std::false_type {};

template <typename T>
struct has_end_build<T, std::void_t<decltype(&T::end_build)>> : std::true_type {
};

static inline unsigned char _bit_scan_forward(unsigned long *index,
                                              uint64_t data) {
  if (data == 0)
    return 0;

  unsigned long res = 0;

  do {
    if (data & 1) {
      *index = res;
      return 1;
    }

    data >>= 1;
    ++res;
  } while (data > 0);

  assert(false);
  return 1;
}

static inline unsigned char bit_scan_forward(unsigned long *index,
                                             uint64_t data) {
#ifdef _WINDOWS
  return _BitScanForward64(index, data);
#else
  return _bit_scan_forward(index, data);
#endif
}

static inline unsigned char _bit_scan_reverse(unsigned long *index,
                                              uint64_t data) {
  if (data == 0)
    return 0;

  unsigned long res = 0;

  do {
    if (data & (1ULL << 63)) {
      *index = 63 - res;
      return 1;
    }

    data <<= 1;
    ++res;
  } while (data > 0);

  assert(false);
  return 1;
}

static inline unsigned char bit_scan_reverse(unsigned long *index,
                                             uint64_t data) {
#ifdef _WINDOWS
  return _BitScanReverse64(index, data);
#else
  return _bit_scan_reverse(index, data);
#endif
}

} // namespace details

class TransSet {
public:
  class Iterator {
  public:
    Iterator(const uint64_t *data)
        : iter_data_{data[0], data[1], data[2], data[3]} {}

    bool end() {
      return iter_data_[0] == 0 && iter_data_[1] == 0 && iter_data_[2] == 0 &&
             iter_data_[3] == 0;
    }

    Iterator &operator++() {
      using namespace details;

      unsigned long index;

      if (bit_scan_forward(&index, iter_data_[0])) {
        iter_data_[0] &= ~(1ULL << index);
      } else if (bit_scan_forward(&index, iter_data_[1])) {
        iter_data_[1] &= ~(1ULL << index);
      } else if (bit_scan_forward(&index, iter_data_[2])) {
        iter_data_[2] &= ~(1ULL << index);
      } else if (bit_scan_forward(&index, iter_data_[3])) {
        iter_data_[3] &= ~(1ULL << index);
      }

      return *this;
    }

    unsigned long trans() const {
      using namespace details;

      unsigned long index;

      if (bit_scan_forward(&index, iter_data_[0])) {
        return index;
      } else if (bit_scan_forward(&index, iter_data_[1])) {
        return index + 64;
      } else if (bit_scan_forward(&index, iter_data_[2])) {
        return index + 64 * 2;
      } else if (bit_scan_forward(&index, iter_data_[3])) {
        return index + 64 * 3;
      }

      return 0; // invalid, no trans
    }

  private:
    uint64_t iter_data_[4];
  };

public:
  TransSet() : data_{0, 0, 0, 0} {}

  void add(uint8_t ch) {
    if (ch < 64) {
      data_[0] |= (1ULL << ch);
    } else if (ch < 64 * 2) {
      data_[1] |= (1ULL << (ch - 64));
    } else if (ch < 64 * 3) {
      data_[2] |= (1ULL << (ch - 64 * 2));
    } else {
      data_[3] |= (1ULL << (ch - 64 * 3));
    }
  }

  const uint64_t *data() const { return data_; }

  bool empty() const {
    return data_[0] == 0 && data_[1] == 0 && data_[2] == 0 && data_[3] == 0;
  }

  bool has(uint8_t ch) const {
    if (ch < 64) {
      return data_[0] & (1ULL << ch);
    } else if (ch < 64 * 2) {
      return data_[1] & (1ULL << (ch - 64));
    } else if (ch < 64 * 3) {
      return data_[2] & (1ULL << (ch - 64 * 2));
    } else {
      return data_[3] & (1ULL << (ch - 64 * 3));
    }
  }

  Iterator begin() const { return {data_}; }

  unsigned long front() const {
    using namespace details;

    unsigned long index;

    if (bit_scan_forward(&index, data_[0])) {
      return index;
    } else if (bit_scan_forward(&index, data_[1])) {
      return index + 64;
    } else if (bit_scan_forward(&index, data_[2])) {
      return index + 64 * 2;
    } else if (bit_scan_forward(&index, data_[3])) {
      return index + 64 * 3;
    }

    return 0; // invalid, no trans
  }

  unsigned long back() const {
    using namespace details;

    unsigned long index;

    if (bit_scan_reverse(&index, data_[3])) {
      return index + 64 * 3;
    } else if (bit_scan_reverse(&index, data_[2])) {
      return index + 64 * 2;
    } else if (bit_scan_reverse(&index, data_[1])) {
      return index + 64;
    } else if (bit_scan_reverse(&index, data_[0])) {
      return index;
    }

    return 0; // invalid, no trans
  }

private:
  uint64_t data_[4];
};

//! @brief Builder for DoubleArrayTrie
//!
//!     We don't consider the size of the array element in builder.
//!     In other words, we will construct the double array trie as simple as
//!     possible. In the same time, we will calculate some meta info, e.g. the
//!     number of states etc. After construction, we will select the most
//!     suitable data type for array elements per the state size etc.
//!
//!     We will also map the most frequent character to the least index offset
//!     when being added to the base offset. Hope it can make the array more
//!     compact. We should be enable to calculate the compression rate it
//!     brings, so, we need a switch to disable this charmap feature.
//!
//! @tparam T value type
template <typename T = int, T DefaultValue = -1> class DoubleArrayTrieBuilder {
public:
  using internal_trie_type = DAWG<T, DefaultValue>;
  using value_type = typename internal_trie_type::value_type;
  static constexpr value_type DEFAULT_VALUE = internal_trie_type::DEFAULT_VALUE;

public:
  class TraverseResult {
    friend class DoubleArrayTrieBuilder;

  public:
    int64_t state() const { return state_index_; }
    bool matched() const { return matched_; }
    uint32_t matched_length() const { return matched_length_; }

  private:
    int64_t state_index_;
    bool matched_;
    uint32_t matched_length_;

    TraverseResult(int64_t state_index, bool matched, uint32_t matched_length)
        : state_index_(state_index), matched_(matched),
          matched_length_(matched_length) {}
  };

public:
  DoubleArrayTrieBuilder() : build_(std::make_unique<BuildInfo>()) {}

  TraverseResult traverse(std::string_view prefix, int64_t state_index) const {
    int64_t p = state_index;

    uint32_t i = 0;
    for (; i < prefix.size(); ++i) {
      uint8_t mapped_ch = charmap_[static_cast<uint8_t>(prefix[i])];
      assert(base_[p] > 0);
      int64_t new_base = base_[p] + mapped_ch;
      if (static_cast<size_t>(new_base) < check_.size() && check_[new_base] == mapped_ch) {
        p = new_base;
      } else {
        return {p, false, i};
      }
    }
    return {p, true, i};
  }

  TraverseResult traverse(std::string_view prefix) const {
    return traverse(prefix, 0);
  }

  const value_type &value_at(int64_t state_index) const {
    return value_[state_index];
  }

  bool has_value_at(int64_t state_index) const {
    return value_at(state_index) != DEFAULT_VALUE;
  }

  void add(std::string_view sv, T value) {
    assert(base_.empty());

    build_->trie_.add(sv, value);

    for (char c : sv) {
      ++build_->char_freq_[c];
    }
  }

  void end_build() {
    assert(base_.empty());

    if constexpr (details::has_end_build<internal_trie_type>::value) {
      build_->trie_.end_build();
    }

    build_charmap();
    build_states();
    build_post_meta_data();

    build_.reset(nullptr);
  }

  value_type &value_at(int64_t state_index) { return value_[state_index]; }

  template <typename OStream, typename F>
  size_t save(OStream &os, F &&serialize_base_check_value) const {
    assert(base_.size() == check_.size() && base_.size() == value_.size());

    constexpr uint32_t charmap_size = sizeof(uint8_t) * (MAX_CHAR_VAL + 1);

    uint32_t size_sum = charmap_size;

    size_sum += static_cast<uint32_t>(serialize_base_check_value.get_size(
        base_, check_, value_, DEFAULT_VALUE));

    os.write(reinterpret_cast<char *>(&size_sum), sizeof(uint32_t));
    os.write(reinterpret_cast<const char *>(charmap_), charmap_size);
    serialize_base_check_value(os, base_, check_, value_, DEFAULT_VALUE);

    return size_sum;
  }

private:
  static constexpr uint32_t MAX_CHAR_VAL = std::numeric_limits<uint8_t>::max();

  struct BuildInfo {
    // internal trie
    internal_trie_type trie_;

    // meta info calculated from input words
    std::unordered_map<char, size_t> char_freq_;

    char rev_charmap_[MAX_CHAR_VAL + 1];
  };

  struct PostMetaData {
    int64_t max_base;
  };

private:
  std::unique_ptr<BuildInfo> build_;

  // constructed things
  uint8_t charmap_[MAX_CHAR_VAL + 1];

  std::vector<int64_t> base_;
  std::vector<int64_t> check_;
  std::vector<T> value_;

  PostMetaData post_;

  void build_post_meta_data() {
    post_.max_base = *std::max_element(base_.begin(), base_.end());
  }

  void build_charmap() {
    std::fill(charmap_, charmap_ + MAX_CHAR_VAL + 1, 0);
    std::fill(build_->rev_charmap_, build_->rev_charmap_ + MAX_CHAR_VAL + 1, 0);

    std::vector<std::pair<size_t, char>> sorted_char_freq;
    for (auto &[ch, n] : build_->char_freq_) {
      sorted_char_freq.push_back({n, ch});
    }

    std::sort(sorted_char_freq.begin(), sorted_char_freq.end(),
              std::greater<std::pair<size_t, char>>());

    static_assert(std::numeric_limits<unsigned char>::max() <=
                  std::numeric_limits<uint8_t>::max());
    assert(sorted_char_freq.size() < MAX_CHAR_VAL); // 0 will never appear

    for (uint8_t i = 0; i < static_cast<uint8_t>(sorted_char_freq.size());
         ++i) {
      assert(sorted_char_freq[i].second != 0);
      charmap_[static_cast<uint8_t>(sorted_char_freq[i].second)] =
          i + 1; // keep 0 as null char
      build_->rev_charmap_[i + 1] = sorted_char_freq[i].second;
    }
  }

  bool overflow(size_t i) const { return i >= check_.size(); }
  bool free(size_t i) const {
    assert(!overflow(i));
    return check_[i] <= 0;
  }
  void resize(size_t n) {
    base_.resize(n + 1, 0);
    check_.resize(n + 1, 0);
    value_.resize(n + 1, DefaultValue);
  }

  bool fit_trans(uint32_t base, const TransSet &trans_set) const {
    // 0 is null char
    assert(overflow(base) || free(base)); // free to place front

    if (overflow(base))
      return true;

    // get front and skip it (it must be free to place front)
    auto it = trans_set.begin();
    auto front = it.trans();
    ++it;

    for (; !it.end(); ++it) {
      uint32_t next = base + it.trans() - front;

      if (overflow(next))
        return true;

      if (!free(next)) // free
        return false;
    }

    return true;
  }

  uint32_t next_free_base(uint32_t base) const {
    assert(overflow(base) || check_[base] <= 0);
    if (overflow(base) || check_[base] == 0)
      return base + 1;
    return static_cast<uint32_t>(-check_[base]);
  }

  uint32_t last_free_base(uint32_t base) const {
    assert(free(base) && base_[base] <= 0);
    if (base_[base] == 0)
      return base - 1;
    return static_cast<uint32_t>(-base_[base]);
  }

  void set_last_free_index(uint32_t for_base, uint32_t last_free_index) {
    base_[for_base] = -static_cast<int64_t>(last_free_index);
  }

  void set_next_free_index(uint32_t for_base, uint32_t next_free_index) {
    check_[for_base] = -static_cast<int64_t>(next_free_index);
  }

  uint32_t find_or_allocate_free_base(const TransSet &trans_set) {
    uint32_t base = next_free_base(0);

    auto front = trans_set.front();
    while (base <= front)
      base = next_free_base(base);

    while (!fit_trans(base, trans_set))
      base = next_free_base(base);

    uint32_t max_next = base + trans_set.back();
    if (overflow(max_next)) {
      resize(max_next);
    }

    return base;
  }

  void build_states() {
    resize(1);

    std::queue<std::pair<const typename internal_trie_type::Node *, uint32_t>>
        q; // node and base
    q.push({build_->trie_.traverse("").state(), 0});

    while (!q.empty()) {
      auto [node, node_base] = q.front();
      q.pop();

      // Construct trans set
      TransSet trans_set;
      for (auto it = node->trans_begin(); it != node->trans_end(); ++it) {
        auto ch = static_cast<uint8_t>(it.key());
        assert(ch > 0);

        ch = charmap_[ch];
        trans_set.add(ch);
      }

      if (trans_set.empty()) {
        // leaf node
        base_[node_base] = 0;
        continue;
      }

      uint32_t start_base = find_or_allocate_free_base(trans_set);

      // assign
      for (auto it = trans_set.begin(); !it.end(); ++it) {
        auto current_base = start_base - trans_set.front() + it.trans();

        // update free pointers
        auto last_free_index = last_free_base(current_base);
        auto next_free_index = next_free_base(current_base);
        set_last_free_index(next_free_index, last_free_index);
        set_next_free_index(last_free_index, next_free_index);

        // assign trans
        check_[current_base] = it.trans();

        // get next state node and store value
        assert(it.trans() < 256);
        auto next_node =
            node->trans_by(build_->rev_charmap_[it.trans()]).target();
        value_[current_base] = next_node->value();

        q.push({next_node, current_base});
      }

      // update base of the "from" state node
      base_[node_base] = start_base - trans_set.front();
    }

    size_t last_unused = base_.size() - 1;
    while (last_unused >= 0 && free(last_unused))
      --last_unused;

    resize(last_unused);
  }
};

#ifdef ASSERT_CONCEPT
static_assert(IsKVTrie<DoubleArrayTrieBuilder<>>);
static_assert(IsStaticTrieBuilder<DoubleArrayTrieBuilder<>>);
static_assert(IsSerializableTrieBuilder<DoubleArrayTrieBuilder<>>);
#endif

} // namespace xtrie

#endif // DATRIE_BUILDER_H
