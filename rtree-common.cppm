export module rtree:common;
import :aabb;
import :db;
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
} // namespace rtree
