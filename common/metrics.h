#ifndef METATRIE_H
#define METATRIE_H

#include <queue>
#include <string_view>
#include <unordered_set>

namespace xtrie {

template <typename, typename = void> struct has_end_build : std::false_type {};

template <typename T>
struct has_end_build<T, std::void_t<decltype(&T::end_build)>> : std::true_type {
};

template <class TrieClass> class MetaTrie {
public:
  using value_type = typename TrieClass::value_type;
  static constexpr value_type DEFAULT_VALUE = TrieClass::DEFAULT_VALUE;
  static constexpr bool HAS_END_BUILD = has_end_build<TrieClass>::value;

  using node_type = typename TrieClass::Node;

  struct Metrics {
    size_t node_size = 0;
    size_t state_size = 0;

    friend std::ostream &operator<<(std::ostream &os, const Metrics &metrics) {
      os << "node_size: " << metrics.node_size << std::endl;
      os << "state_size: " << metrics.state_size << std::endl;
      os << "compression_rate: "
         << (float)metrics.node_size / metrics.state_size << std::endl;
      return os;
    }
  };

public:
  MetaTrie() = default;
  MetaTrie(TrieClass &&o) noexcept : trie_{std::move(o)} {}

  Metrics collect_metrics() const {
    Metrics res;

    std::queue<const node_type *> q;
    std::unordered_set<const node_type *> visited;
    q.push(&trie_.root_);

    while (!q.empty()) {
      const node_type *p = q.front();
      q.pop();

      if (auto it = visited.find(p); it == visited.end()) {
        ++res.node_size;
        visited.insert(it, p);
      }

      ++res.state_size;

      for (auto it = p->trans_begin(); it != p->trans_end(); ++it) {
        q.push(it.target());
      }
    }

    return res;
  }

  auto traverse(std::string_view prefix) const {
    return trie_.traverse(prefix);
  }

  template <class TVec>
  void load_words(TVec &words, bool already_sorted = false) {
    if (!already_sorted)
      std::sort(words.begin(), words.end());

    for (auto &w : words) {
      add(w, 0);
    }

    end_build();
  }

  void add(std::string_view sv, value_type value) { trie_.add(sv, value); }

  void end_build() {
    if constexpr (HAS_END_BUILD) {
      trie_.end_build();
    }
  }

  auto traverse(std::string_view prefix) { return trie_.traverse(prefix); }

private:
  TrieClass trie_;
};

} // namespace xtrie

#endif // METATRIE_H