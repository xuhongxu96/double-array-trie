#ifndef COMPACT_SERIALIZER
#define COMPACT_SERIALIZER

#include <cstdint>

namespace xtrie {

namespace details {
struct CompactValue {
  unsigned terminal : 1;
  unsigned check : 8;
  unsigned base : 23;
};
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
  template <typename T> size_t get_size(int64_t, int64_t, T, T) const {
    return sizeof(uint32_t);
  }

  template <typename OStream, typename T>
  void operator()(OStream &os, int64_t base, int64_t check, T value,
                  T default_value) const {
    assert(base < (1 << 23) && check < (1 << 8));

    union {
      details::CompactValue val;
      uint32_t uint32;
    };

    val.base = static_cast<uint32_t>(base);
    val.check = static_cast<uint8_t>(check);
    val.terminal = value != default_value;

    os.write(reinterpret_cast<char *>(&uint32), sizeof(uint32_t));
  }
};

} // namespace xtrie

#endif // DATRIE_COMPACT_SERIALIZER