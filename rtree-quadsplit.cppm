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

void split_node(db::nnid n) {
  // QS1
  auto &node = db::current()->read(n);
  auto [ei1, ei2] = pick_seeds(node);
  auto g1 = db::current()->create_node(node.parent, node.leaf);
  move_to_group(g1, n, ei1);
  auto g2 = db::current()->create_node(node.parent, node.leaf);
  move_to_group(g2, n, ei2);

  while (true) {
    // QS2
    auto n_size = db::current()->read(n).size;
    if (n_size == 0) {
      db::current()->delete_node(n);
      break;
    }
    auto g1_size = db::current()->read(g1).size;
    if (g1_size + n_size <= db::node_lower_limit) {
      move_to_group(g1, n, 0);
      continue;
    }
    auto g2_size = db::current()->read(g1).size;
    if (g2_size + n_size <= db::node_lower_limit) {
      move_to_group(g2, n, 0);
      continue;
    }
  }
}
} // namespace rtree
