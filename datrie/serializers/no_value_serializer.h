#ifndef DATRIE_NO_VALUE_SERIALIZER
#define DATRIE_NO_VALUE_SERIALIZER

#include <cstdint>
#include <vector>

namespace xtrie {

//! @brief No-value serializer will not save the values, but save a flag
//! indicating whether the state is a terminal
//!
//!     base value must be less than 2^23 so that the base and check can be
//!     stored in a single int32.
//!
//!     23 bit for base, 8 bit for check, 1 bit for terminal flag.
//!
struct NoValueSerializer {
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

    union {
      CompactUnit unit;
      uint32_t uint32;
    };

    for (size_t i = 0; i < base.size(); ++i) {
      assert(base[i] < (1 << 23) && check[i] < (1 << 8));

      unit.base = static_cast<uint32_t>(base[i]);
      unit.check = static_cast<uint8_t>(check[i]);
      unit.terminal = value[i] != default_value;

      os.write(reinterpret_cast<char *>(&uint32), sizeof(uint32_t));
    }
  }

private:
  struct CompactUnit {
    unsigned terminal : 1;
    unsigned check : 8;
    unsigned base : 23;
  };

  static_assert(sizeof(CompactUnit) == sizeof(uint32_t));
};

} // namespace xtrie

#endif // DATRIE_NO_VALUE_SERIALIZER