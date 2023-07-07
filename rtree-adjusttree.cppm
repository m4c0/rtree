export module rtree:adjusttree;
import :aabb;
import :db;
import :quadsplit;
import silog;

namespace rtree {
unsigned find_n_in_parent(db::nnid n, db::nnid p) {
  auto pnode = db::current()->read(p);
  for (auto idx = 0U; idx < pnode.size; idx++) {
    if (pnode.children[idx].id == n)
      return idx;
  }
  // Unreachable, unless DB is inconsistent
  silog::log(silog::error, "attempt of reading orphaned node");
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
