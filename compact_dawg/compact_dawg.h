#ifndef COMPACT_DAWG_H
#define COMPACT_DAWG_H

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

template <typename T = int, T DefaultValue = -1> class CompactDAWG {
public:
  using value_type = T;
  static constexpr value_type DEFAULT_VALUE = DefaultValue;

  class Node {
    friend class CompactDAWG;

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
      node_pointer_type target() const { return iter_->second.get(); }
      node_smart_pointer_type target_shared_ptr() const {
        return iter_->second;
      }

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

    std::string prefix_;

  public:
    ConstTransitionIterator trans_begin() const { return {trans_.cbegin()}; }
    ConstTransitionIterator trans_end() const { return {trans_.cend()}; }
    size_t trans_size() const { return trans_.size(); }
    ConstTransitionIterator trans_by(char key) const {
      return {trans_.find(key)};
    }

    bool has_value() const { return value_ != DEFAULT_VALUE; }
    const value_type &value() const { return value_; }

    const std::string &prefix() const { return prefix_; }

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

    void set_prefix(std::string prefix) { prefix_ = std::move(prefix); }
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
    friend class CompactDAWG;

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

  struct Metrics {
    size_t state_size = 0;
    std::unordered_map<size_t, size_t> single_branch_length_to_count;
    std::unordered_map<std::string, size_t> strings;
  };

public:
  CompactDAWG() : build_(std::make_unique<BuildInfo>()) {}

  TraverseResult traverse(std::string_view prefix, const Node *start) const {
    const Node *p = start;
    uint32_t i = 0;
    for (; i < prefix.size(); ++i) {
      if (!p->prefix().empty()) {
        for (auto p_ch : p->prefix()) {
          if (i >= prefix.size() || p_ch != prefix[i++]) {
            return {p, false, i};
          }
        }
        if (i >= prefix.size())
          break;
      }

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
    compact();
  }

  value_type &value_at(const Node *state) {
    return const_cast<Node *>(state)->value();
  }

  Metrics collect_metrics() const {
    Metrics res;
    collect_metrics(&root_, res, 0);
    return res;
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
      res += ',';
    }
    build_->node_id_map_.insert(it, {node, std::move(res)});
    return build_->node_id_map_[node];
  }

  void collect_metrics(const Node *node, Metrics &meta,
                       size_t single_trans_size) const {
    ++meta.state_size;
    ++meta.strings[node->prefix()];
    if (node->trans_size() == 1) {
      ++single_trans_size;
    } else {
      if (single_trans_size > 0)
        ++meta.single_branch_length_to_count[single_trans_size];

      single_trans_size = 0;
    }

    for (auto it = node->trans_begin(); it != node->trans_end(); ++it) {
      collect_metrics(it.target(), meta, single_trans_size);
    }
  }

  void compact() {
    std::unordered_map<Node *, size_t> referenced_nodes;
    {
      std::queue<Node *> q;
      q.push(&root_);
      while (!q.empty()) {
        Node *node = q.front();
        q.pop();

        for (auto it = node->trans_begin(); it != node->trans_end(); ++it) {
          ++referenced_nodes[it.target()];
          q.push(it.target());
        }
      }
    }

    std::unordered_map<Node *, std::shared_ptr<Node>> old_to_new;
    std::queue<std::tuple<std::shared_ptr<Node>, Node *, char>> q;

    auto push_trans = [&q](Node *node) {
      for (auto it = node->trans_begin(); it != node->trans_end(); ++it) {
        q.push({it.target_shared_ptr(), node, it.key()});
      }
    };

    auto is_refed_twice = [&referenced_nodes](Node *node) {
      if (auto it = referenced_nodes.find(node); it != referenced_nodes.end()) {
        return it->second > 1;
      }

      return false;
    };

    push_trans(&root_);

    while (!q.empty()) {
      auto [node, parent, key] = q.front();
      q.pop();

      if (auto new_it = old_to_new.find(node.get());
          new_it != old_to_new.end()) {
        parent->insert_trans(key, new_it->second);
        continue;
      }

      if (node->trans_size() != 1 || node->has_value() ||
          !node->prefix().empty() || is_refed_twice(node.get())) {
        push_trans(node.get());
        continue;
      }

      std::shared_ptr<Node> end = node;
      std::string trans_str;
      do {
        auto next = end->trans_begin().target_shared_ptr();
        if (is_refed_twice(next.get()))
          break;

        trans_str += end->trans_begin().key();
        end = next;
      } while (end->trans_size() == 1 && !end->has_value());

      if (trans_str.size() < 4) {
        // unworthy compacting
        push_trans(node.get());
        continue;
      }

      end->set_prefix(trans_str);
      parent->insert_trans(key, end);
      old_to_new[node.get()] = end;

      push_trans(end.get());
    }
  }
};

#ifdef ASSERT_CONCEPT
static_assert(IsKVTrie<CompactDAWG<>>);
static_assert(IsStaticTrieBuilder<CompactDAWG<>>);
#endif

} // namespace xtrie

#endif // COMPACT_DAWG_H
