export module rtree:adjusttree;
import :aabb;
import :common;
import :db;
import :quadsplit;
import silog;

namespace rtree {
auto at3(db::nnid n) {
  auto node = db::current()->read(n);
  auto p = node.parent;
  if (p) {
    auto en = find_n_in_parent(n, p);
    auto eni = calculate_enclosing_rect(node);
    db::current()->adjust_eni(p, en, eni);
  }
  return p;
}

void reparent(db::nnid n, db::nnid p) {
  auto node = db::current()->read(n);
  auto area = calculate_enclosing_rect(node);
  db::current()->create_enni(p, n, area);
}

db::nnid adjust_tree(db::nnid n, db::nnid nn) {
  db::nnid p = at3(n);
  if (!p) {
    if (!nn)
      return n;

    auto root = db::current()->create_node(db::nnid{}, false);
    reparent(n, root);
    reparent(nn, root);
    db::current()->set_parent(n, root);
    db::current()->set_parent(nn, root);
    return root;
  }

  db::nnid pp{};
  if (nn) {
    reparent(nn, p);

    auto pnode = db::current()->read(p);
    if (pnode.size == db::node_limit)
      pp = split_node(p);
  }

  return adjust_tree(p, pp);
}
} // namespace rtree
