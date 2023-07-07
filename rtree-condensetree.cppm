export module rtree:condensetree;
import :aabb;
import :common;
import :db;
import :insert;

namespace rtree {
db::nnid condense_tree(db::nnid n) {
  auto &node = db::current()->read(n);
  auto p = node.parent;
  if (!p)
    return n;

  auto idx = find_n_in_parent(n, p);
  if (node.size >= db::node_lower_limit) {
    auto area = calculate_enclosing_rect(node);
    db::current()->adjust_eni(p, idx, area);
    return condense_tree(p);
  }

  db::current()->remove_eni(p, idx);
  auto root = condense_tree(p);
  // ct6 ("q" is actually the call stack)
  for (auto i = 0U; i < node.size; i++) {
    auto &[ei, earea] = node.children[i];
    // insert(root, ei, earea);
  }

  return root;
}
} // namespace rtree
