export module rtree:quadsplit;
import :aabb;
import :db;

namespace rtree {
struct seeds {
  unsigned ei1;
  unsigned ei2;
};

seeds pick_seeds(const db::node &n) {
  seeds res{};
  float max_d = 0.f;
  for (auto i1 = 0U; i1 < n.size; i1++) {
    auto &e1 = n.children[i1];
    for (auto i2 = i1 + 1U; i2 < n.size; i2++) {
      auto &e2 = n.children[i2];
      auto j = merge(e1.area, e2.area);
      float d = area_of(j) - area_of(e1.area) - area_of(e2.area);
      if (d > max_d) {
        res = {i1, i2};
      }
    }
  }
  return res;
}

inline void move_to_group(db::nnid g, db::nnid n, unsigned idx) {
  auto e = db::current()->read(n).children[idx];
  db::current()->create_enni(g, e.id, e.area);
  db::current()->remove_eni(n, idx);
}

db::nnid split_node(db::nnid n) {
  auto &node = db::current()->read(n);

  auto [ei1, ei2] = pick_seeds(node);

  auto g1 = db::current()->create_node(node.parent, node.leaf);
  move_to_group(g1, n, ei1);

  auto g2 = db::current()->create_node(node.parent, node.leaf);
  move_to_group(g2, n, ei2);

  return db::nnid{};
}
} // namespace rtree
