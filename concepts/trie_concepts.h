#ifndef TRIE_H
#define TRIE_H

#include <concepts>
#include <iostream>

namespace xtrie {

template <typename T, typename TValue>
concept IsTraverseResult = requires(const std::remove_cvref_t<T> &res) {
  { res.matched() } -> std::convertible_to<bool>;
  { res.matched_length() } -> std::convertible_to<uint32_t>;
  {res.state()};
};

template <typename T>
concept IsTrie = requires(std::remove_cvref_t<T> &trie) {
  typename T::value_type;

  requires std::same_as<const typename T::value_type,
                        decltype(T::DEFAULT_VALUE)>;

  { trie.add("", T::DEFAULT_VALUE) } -> std::same_as<void>;
  {
    trie.value_at(trie.traverse("").state())
    } -> std::same_as<typename T::value_type &>;
}
&&requires(const std::remove_cvref_t<T> &trie) {
  { trie.traverse("") } -> IsTraverseResult<typename T::value_type>;
  {
    trie.traverse("", trie.traverse("").state())
    } -> IsTraverseResult<typename T::value_type>;
  {
    trie.value_at(trie.traverse("").state())
    } -> std::same_as<const typename T::value_type &>;
  { trie.has_value_at(trie.traverse("").state()) } -> std::convertible_to<bool>;
};

template <typename T>
concept IsStaticTrie = IsTrie<T> && requires(std::remove_cvref_t<T> &trie) {
  { trie.end_build() } -> std::same_as<void>;
};

template <typename T>
concept IsSerializableTrie = IsTrie<T> &&
    requires(std::remove_cvref_t<T> &trie, std::ostream &os) {
  {
    trie.save(os, [](std::ostream &os, int64_t, int64_t, int, int) { os << 0; })
    } -> std::same_as<void>;
};

} // namespace xtrie

#endif TRIE_H