#ifndef DAWG_H
#define DAWG_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#ifdef ASSERT_CONCEPT
#include <trie_concepts.h>
#endif

namespace xtrie {

template <typename T = int, T DefaultValue = -1> class DAWG {
public:
  using value_type = T;
  static constexpr value_type DEFAULT_VALUE = DefaultValue;

  class Node {
    friend class DAWG;

  public:
    using trans_type = std::unordered_map<char, std::shared_ptr<Node>>;

  private:
    template <bool Const> class TransitionIterator_ {
      friend class Node;

    protected:
      using inner_iterator_type =
          std::conditional_t<Const, typename trans_type::const_iterator,
                             typename trans_type::iterator>;
      using node_pointer_type = std::conditional_t<Const, const Node *, Node *>;
      using node_smart_pointer_type =
          std::conditional_t<Const, std::shared_ptr<const Node>,
                             std::shared_ptr<Node>>;

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
      node_smart_pointer_type target_shared_ptr() { return iter_->second; }

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
                      std::shared_ptr<Node> node) {
      trans_.insert_or_assign(hint.iter_, key, node);
    }

    void insert_trans(char ch, std::shared_ptr<Node> node) {
      trans_.insert_or_assign(ch, node);
    }
  };

private:
  struct UncheckedNode {
    Node *parent;
    char trans;
  };

  struct BuildInfo {
    std::string current_prefix_;
    std::stack<UncheckedNode> unchecked_nodes_;
    std::unordered_map<std::string_view, std::shared_ptr<Node>>
        minimized_nodes_;
    std::unordered_map<Node *, std::string> node_id_map_;
  };

public:
  class TraverseResult {
    friend class DAWG;

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
  DAWG() : build_(std::make_unique<BuildInfo>()) {}

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
    size_t current_prefix_size = build_->current_prefix_.size();

    size_t common_size = 0;
    for (; common_size < std::min(sv.size(), current_prefix_size);
         ++common_size) {
      if (sv[common_size] != build_->current_prefix_[common_size]) {
        break;
      }
    }

    minimize(common_size);

    Node *p = &root_;
    if (!build_->unchecked_nodes_.empty()) {
      auto top_unchecked_node = build_->unchecked_nodes_.top();
      p = top_unchecked_node.parent->trans_by(top_unchecked_node.trans)
              .target();
    }

    build_->current_prefix_ = sv;
    sv = sv.substr(common_size);

    for (char ch : sv) {
      assert(p->trans_by(ch) == p->trans_end());

      auto new_node = std::make_shared<Node>();
      auto temp = new_node.get();
      p->insert_trans(ch, std::move(new_node));
      build_->unchecked_nodes_.push({p, ch});
      p = temp;
    }

    p->value() = value;
  }

  void end_build() {
    minimize(0);
    build_.reset();
  }

  value_type &value_at(const Node *state) {
    return const_cast<Node *>(state)->value();
  }

private:
  Node root_;

  std::unique_ptr<BuildInfo> build_;

  void minimize(size_t common_size) {
    for (size_t i = build_->unchecked_nodes_.size(); i > common_size; --i) {
      UncheckedNode unchecked_node = build_->unchecked_nodes_.top();
      build_->unchecked_nodes_.pop();

      auto child = unchecked_node.parent->trans_by(unchecked_node.trans)
                       .target_shared_ptr();

      auto id = calc_id(child.get());
      auto it = build_->minimized_nodes_.find(id);
      if (it == build_->minimized_nodes_.end()) {
        build_->minimized_nodes_.insert(it, {id, child});
      } else {
        unchecked_node.parent->insert_trans(unchecked_node.trans, it->second);
        build_->node_id_map_.erase(child.get());
      }
    }
  }

  std::string_view calc_id(Node *node) {
    auto it = build_->node_id_map_.find(node);
    if (it != build_->node_id_map_.end())
      return it->second;

    std::string res;
    std::vector<char> keys;
    for (auto &it : node->trans_) {
      keys.push_back(it.first);
    }
    std::sort(keys.begin(), keys.end());

    res += std::to_string(node->value_);

    for (char key : keys) {
      Node *p = node->trans_.find(key)->second.get();
      res += '|';
      res += key;
      res += calc_id(p);
    }
    build_->node_id_map_.insert(it, {node, std::move(res)});
    return build_->node_id_map_[node];
  }
};

#ifdef ASSERT_CONCEPT
static_assert(IsStaticTrie<DAWG<>>);
#endif

} // namespace xtrie

#endif // DAWG_H
