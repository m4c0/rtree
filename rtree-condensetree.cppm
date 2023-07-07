export module rtree:condensetree;
import :aabb;
import :common;
import :db;

namespace rtree {
void condense_tree(db::nnid n) {
  auto &node = db::current()->read(n);
  auto p = node.parent;
  if (p) {
    auto idx = find_n_in_parent(n, p);
    if (node.size < db::node_lower_limit) {
      db::current()->remove_eni(p, idx);
      condense_tree(p);
      // ct6 ("q" is actually the call stack)
      for (auto i = 0U; i < node.size; i++) {
        auto &[ei, earea] = node.children[i];
        // insert
      }
    } else {
      auto area = calculate_enclosing_rect(node);
      db::current()->adjust_eni(p, idx, area);
      condense_tree(p);
    }
  }
}
} // namespace rtree
