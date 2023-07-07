export module rtree:chooseleaf;
import :aabb;
import :db;

namespace rtree {
[[nodiscard]] db::nnid choose_leaf(db::nnid n, aabb ei) {
  auto node = db::current()->read(n);
  if (node.leaf)
    return n;

  db::nnid f{};
  float min_enl{};
  float rect_area{};
  for (auto i = 0U; i < node.size; i++) {
    auto &[id, area] = node.children[i];
    auto enl = enlargement(area, ei);
    auto ra = area_of(area);
    if (!f || (enl < min_enl) || (enl == min_enl && ra < rect_area)) {
      f = id;
      min_enl = enl;
      rect_area = ra;
      continue;
    }
  }
  return choose_leaf(f, ei);
}
} // namespace rtree
