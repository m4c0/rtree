export module rtree:chooseleaf;
import :aabb;
import :db;

namespace rtree {
[[nodiscard]] constexpr db::nnid choose_leaf(db::storage *dbs, db::nnid n,
                                             aabb ei, int max_level = -1) {
  auto node = dbs->read(n);
  if (node.leaf || max_level == 0)
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
  return choose_leaf(dbs, f, ei, max_level - 1);
}
} // namespace rtree
