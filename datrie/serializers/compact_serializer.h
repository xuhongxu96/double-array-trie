#ifndef DATRIE_COMPACT_SERIALIZER
#define DATRIE_COMPACT_SERIALIZER

#include <cstdint>
#include <vector>

namespace xtrie {

//! @brief Compact serializer will save the values into the base array (offset
//! by 0)
//!
//!     base value must be less than 2^22 so that the base and check can be
//!     stored in a single int32.
//!
//!     22 bit for base, 8 bit for check, 2 bit for value flag.
//!
//!     value_flag: 1 means has value in bases[current base], 2 means current
//!     base is value, 0 means not a terminal node (no value).
//!
struct CompactSerializer {
  template <typename T>
  size_t get_size(const std::vector<int64_t> &base,
                  const std::vector<int64_t> &, const std::vector<T> &,
                  T) const {
    return sizeof(uint32_t) * base.size();
  }

  template <typename OStream, typename T>
  void operator()(OStream &os, const std::vector<int64_t> &base,
                  const std::vector<int64_t> &check,
                  const std::vector<T> &value, T default_value) const {
    static_assert(sizeof(T) <= sizeof(uint32_t));

    union {
      CompactUnit unit;
      uint32_t uint32;
    };

    for (size_t i = 0; i < base.size(); ++i) {
      assert(base[i] < (1 << 22) && check[i] < (1 << 8));

      unit.base = static_cast<uint32_t>(base[i]);
      unit.check = static_cast<uint8_t>(check[i]);
      unit.value_flag = value[i];

      os.write(reinterpret_cast<char *>(&uint32), sizeof(uint32_t));
    }
  }

private:
  struct CompactUnit {
    unsigned value_flag : 2;
    unsigned check : 8;
    unsigned base : 22;
  };

  static_assert(sizeof(CompactUnit) == sizeof(uint32_t));
};

} // namespace xtrie

#endif // DATRIE_COMPACT_SERIALIZER
