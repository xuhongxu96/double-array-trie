#ifndef COMPACT_SERIALIZER
#define COMPACT_SERIALIZER

#include <cstdint>
#include <vector>

namespace xtrie {

namespace details {
struct CompactValue {
  unsigned terminal : 1;
  unsigned check : 8;
  unsigned base : 23;
};

static_assert(sizeof(CompactValue) == sizeof(uint32_t));

} // namespace details

//! @brief Compact serializer will not save the values, but save a flag
//! indicating whether the state is a terminal
//!
//!     base value must be less than 2^23 so that the base and check can be
//!     stored in a single int32.
//!
//!     23 bit for base, 8 bit for check, 1 bit for terminal flag.
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
    assert(base < (1 << 23) && check < (1 << 8));

    union {
      details::CompactValue val;
      uint32_t uint32;
    };

    for (size_t i = 0; i < base.size(); ++i) {
      val.base = static_cast<uint32_t>(base[i]);
      val.check = static_cast<uint8_t>(check[i]);
      val.terminal = value[i] != default_value;

      os.write(reinterpret_cast<char *>(&uint32), sizeof(uint32_t));
    }
  }
};

} // namespace xtrie

#endif // DATRIE_COMPACT_SERIALIZER