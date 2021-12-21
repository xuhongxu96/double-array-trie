#ifndef COMPACT_DATRIE_H
#define COMPACT_DATRIE_H

namespace xtrie {

template <typename T = int, T DefaultValue = -1> class CompactDoubleArrayTrie {
public:
  using value_type = T;
  static constexpr value_type DEFAULT_VALUE = DefaultValue;
};

} // namespace xtrie

#endif // COMPACT_DATRIE_H