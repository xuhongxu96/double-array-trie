#ifndef COMPACT_DATRIE_H
#define COMPACT_DATRIE_H

#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>

namespace xtrie {

template <typename T = int, T DefaultValue = -1> class CompactDoubleArrayTrie {
public:
  using value_type = T;
  static constexpr value_type DEFAULT_VALUE = DefaultValue;

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
    values_.resize(size_sum / sizeof(uint32_t));

    is.read(reinterpret_cast<char *>(charmap_), charmap_size);

    for (size_t i = 0; i < values_.size(); ++i) {
      uint32_t res;
      is.read(reinterpret_cast<char *>(&res), sizeof(uint32_t));
      values_[i].val = res;
    }
  }

private:
  union CompactValue {
    struct {
      unsigned terminal : 1;
      unsigned check : 8;
      unsigned base : 23;
    };

    uint32_t val;
  };

  static_assert(sizeof(CompactValue) == sizeof(uint32_t));

  uint8_t charmap_[MAX_CHAR_VAL + 1];
  std::vector<CompactValue> values_;
};

} // namespace xtrie

#endif // COMPACT_DATRIE_H