export module rtree:quadsplit;
import :aabb;
import :db;

namespace rtree {
struct seeds {
  db::link e1;
  db::link e2;
};

seeds pick_seeds(const db::node &n) {
  seeds res{};
  float max_d = 0.f;
  for (auto i1 = 0; i1 < n.size; i1++) {
    auto &e1 = n.children[i1];
    for (auto i2 = i1 + 1; i2 < n.size; i2++) {
      auto &e2 = n.children[i2];
      auto j = merge(e1.area, e2.area);
      float d = area_of(j) - area_of(e1.area) - area_of(e2.area);
      if (d > max_d) {
        res = {e1, e2};
      }
    }
  }
  return res;
}

} // namespace rtree
