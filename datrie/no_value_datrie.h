#ifndef NO_VALUE_DATRIE_H
#define NO_VALUE_DATRIE_H

#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>

#ifdef ASSERT_CONCEPT
#include <trie_concepts.h>
#endif

namespace xtrie {

template <typename T = int, T DefaultValue = -1> class NoValueDoubleArrayTrie {
public:
  using value_type = T;
  static constexpr value_type DEFAULT_VALUE = DefaultValue;

  class TraverseResult {
    friend class NoValueDoubleArrayTrie;

  public:
    unsigned state() const { return state_index_; }
    bool matched() const { return matched_; }
    uint32_t matched_length() const { return matched_length_; }

  private:
    unsigned state_index_;
    bool matched_;
    uint32_t matched_length_;

    TraverseResult(unsigned state_index, bool matched, uint32_t matched_length)
        : state_index_(state_index), matched_(matched),
          matched_length_(matched_length) {}
  };

private:
  static constexpr uint32_t MAX_CHAR_VAL = std::numeric_limits<uint8_t>::max();

public:
  template <typename IStream> void load(IStream &is) {

    uint32_t size_sum;
    is.read(reinterpret_cast<char *>(&size_sum), sizeof(uint32_t));

    constexpr uint32_t charmap_size =
        static_cast<uint32_t>(sizeof(uint8_t)) * (MAX_CHAR_VAL + 1);

    assert(size_sum > charmap_size);

    size_sum -= charmap_size;
    bases_.resize(size_sum / sizeof(uint32_t));

    is.read(reinterpret_cast<char *>(charmap_), charmap_size);

    for (size_t i = 0; i < bases_.size(); ++i) {
      uint32_t res;
      is.read(reinterpret_cast<char *>(&res), sizeof(uint32_t));
      bases_[i].unit = res;
    }
  }

  TraverseResult traverse(std::string_view prefix, unsigned state_index) const {
    unsigned p = state_index;

    uint32_t i = 0;
    for (; i < prefix.size(); ++i) {
      uint8_t mapped_ch = charmap_[static_cast<uint8_t>(prefix[i])];
      unsigned new_base = bases_[p].base + mapped_ch;
      if (new_base < bases_.size() && bases_[new_base].check == mapped_ch) {
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

  bool has_value_at(unsigned state_index) const {
    return bases_[state_index].terminal;
  }

private:
  union CompactUnit {
    struct {
      unsigned terminal : 1;
      unsigned check : 8;
      unsigned base : 23;
    };

    uint32_t unit;
  };

  static_assert(sizeof(CompactUnit) == sizeof(uint32_t));

  uint8_t charmap_[MAX_CHAR_VAL + 1];
  std::vector<CompactUnit> bases_;
};

#ifdef ASSERT_CONCEPT
static_assert(IsDeserializableTrie<NoValueDoubleArrayTrie<>>);
#endif

} // namespace xtrie

#endif // NO_VALUE_DATRIE_H