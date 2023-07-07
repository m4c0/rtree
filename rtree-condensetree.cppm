export module rtree:condensetree;
import :aabb;
import :common;
import :db;
import :insert;

namespace rtree {
struct ctres {
  db::nnid root;
  unsigned level;
};
ctres condense_tree(db::nnid n) {
  auto &node = db::current()->read(n);
  auto p = node.parent;
  if (!p)
    return {n, 1};

  auto idx = find_n_in_parent(n, p);
  if (node.size >= db::node_lower_limit) {
    auto area = calculate_enclosing_rect(node);
    db::current()->adjust_eni(p, idx, area);
    auto [root, lvl] = condense_tree(p);
    return {root, lvl + 1};
  }

  db::current()->remove_eni(p, idx);
  auto [root, lvl] = condense_tree(p);

  // ct6 ("q" is embedded in the call stack)
  for (auto i = 0U; i < node.size; i++) {
    auto &[ei, earea] = node.children[i];
    root = insert(root, ei, earea, lvl);
  }
  db::current()->delete_node(n);

  return {root, lvl + 1};
}
} // namespace rtree
