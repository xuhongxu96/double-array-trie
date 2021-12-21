#ifndef HASHTRIE_H
#define HASHTRIE_H

#include <iostream>
#include <memory>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#ifdef ASSERT_CONCEPT
#include <trie_concepts.h>
#endif

namespace xtrie {

template <typename T = int, T DefaultValue = -1> class HashTrie {
public:
  using value_type = T;
  static constexpr value_type DEFAULT_VALUE = DefaultValue;

  class Node {
  public:
    using value_type = value_type;
    using trans_type = std::unordered_map<char, std::unique_ptr<Node>>;

  private:
    template <bool Const> class TransitionIterator_ {
      friend class Node;

    public:
      using inner_iterator_type =
          std::conditional_t<Const, typename trans_type::const_iterator,
                             typename trans_type::iterator>;
      using node_pointer_type = std::conditional_t<Const, const Node *, Node *>;

      TransitionIterator_(inner_iterator_type iter) : iter_(iter) {}

    public:
      TransitionIterator_ &operator++() {
        ++iter_;
        return *this;
      }

      TransitionIterator_ operator++(int) { return {iter_++}; }

      bool operator==(TransitionIterator_ b) const { return iter_ == b.iter_; }

      char key() const { return iter_->first; }
      node_pointer_type target() { return iter_->second.get(); }

    protected:
      inner_iterator_type iter_;
    };

  public:
    class TransitionIterator : public TransitionIterator_<false> {};
    class ConstTransitionIterator : public TransitionIterator_<true> {
    protected:
      using TransitionIterator_<true>::TransitionIterator_;

    public:
      ConstTransitionIterator(TransitionIterator iter)
          : TransitionIterator_<true>(iter.iter_) {}
    };

  private:
    value_type value_ = DEFAULT_VALUE;
    trans_type trans_;

  public:
    ConstTransitionIterator trans_begin() const { return {trans_.cbegin()}; }
    ConstTransitionIterator trans_end() const { return {trans_.cend()}; }
    ConstTransitionIterator trans_by(char key) const {
      return {trans_.find(key)};
    }

    bool has_value() const { return value_ != DEFAULT_VALUE; }
    const value_type &value() const { return value_; }

    TransitionIterator trans_begin() { return {trans_.begin()}; }
    TransitionIterator trans_end() { return {trans_.end()}; }
    TransitionIterator trans_by(char key) { return {trans_.find(key)}; }

    value_type &value() { return value_; }

    void insert_trans(TransitionIterator hint, char key,
                      std::unique_ptr<Node> node) {
      trans_.insert_or_assign(hint.iter_, key, std::move(node));
    }

    void insert_trans(char ch, std::unique_ptr<Node> node) {
      trans_.insert_or_assign(ch, std::move(node));
    }
  };

public:
  class TraverseResult {
    friend class HashTrie;

  public:
    const Node *state() const { return node_; }
    bool matched() const { return matched_; }
    uint32_t matched_length() const { return matched_length_; }

  private:
    const Node *node_;
    bool matched_;
    uint32_t matched_length_;

    TraverseResult(const Node *node, bool matched, uint32_t matched_length)
        : node_(node), matched_(matched), matched_length_(matched_length) {}
  };

public:
  TraverseResult traverse(std::string_view prefix, const Node *start) const {
    const Node *p = start;
    uint32_t i = 0;
    for (; i < prefix.size(); ++i) {
      auto it = p->trans_by(prefix[i]);
      if (it == p->trans_end()) {
        return {p, false, i};
      }

      p = it.target();
    }
    return {p, true, i};
  }

  TraverseResult traverse(std::string_view prefix) const {
    return traverse(prefix, &root_);
  }

  bool has_value_at(const Node *state) const {
    return value_at(state) != DEFAULT_VALUE;
  }

  const value_type &value_at(const Node *state) const { return state->value(); }

  void add(std::string_view sv, T value) {
    Node *p = &root_;
    for (char ch : sv) {
      auto it = p->trans_by(ch);
      if (it == p->trans_end()) {
        auto new_node = std::make_unique<Node>();
        auto temp = new_node.get();
        p->insert_trans(it, ch, std::move(new_node));
        p = temp;
      } else {
        p = it.target();
      }
    }

    p->value() = value;
  }

  value_type &value_at(const Node *state) {
    return const_cast<Node *>(state)->value();
  }

private:
  Node root_;
};

#ifdef ASSERT_CONCEPT
static_assert(IsTrie<HashTrie<>>);
static_assert(!IsStaticTrie<HashTrie<>>);
#endif

} // namespace xtrie

#endif // HASHTRIE_H