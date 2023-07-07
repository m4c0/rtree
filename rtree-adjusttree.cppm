export module rtree:adjusttree;
import :aabb;
import :db;
import :quadsplit;

namespace rtree {
unsigned find_n_in_parent(db::nnid n, db::nnid p) {
  auto pnode = db::current()->read(p);
  for (auto idx = 0U; idx < pnode.size; idx++) {
    if (pnode.children[idx].id == n)
      return idx;
  }
  // Unreachable, unless DB is inconsistent
  throw db::inconsistency_error{};
}
aabb calculate_enclosing_rect(const db::node &node) {
  aabb eni{
      .a = {9e10f, 9e10f},
      .b = {-9e10f, -9e10f},
  };
  for (auto i = 0U; i < node.size; i++) {
    auto area = node.children[i].area;
    eni.a.x = min(area.a.x, eni.a.x);
    eni.a.y = min(area.a.y, eni.a.y);
    eni.b.x = max(area.b.x, eni.b.x);
    eni.b.y = max(area.b.y, eni.b.y);
  }
  return eni;
}

auto at3(db::nnid n) {
  auto node = db::current()->read(n);
  auto p = node.parent;
  auto en = find_n_in_parent(n, p);
  auto eni = calculate_enclosing_rect(node);
  db::current()->adjust_eni(p, en, eni);

  return p;
}

void adjust_tree(db::nnid root, db::nnid n, db::nnid nn) {
  if (n == root)
    return;

  db::nnid p = at3(n);

  db::nnid pp{};
  if (nn) {
    auto nnnode = db::current()->read(nn);
    auto enni = calculate_enclosing_rect(nnnode);
    db::current()->create_enni(p, nn, enni);

    auto pnode = db::current()->read(p);
    if (pnode.size == db::node_limit) {
      auto [g1, g2] = split_node(p);
      p = g1;
      pp = g2;
    }
  }

  adjust_tree(root, p, pp);
}
} // namespace rtree
