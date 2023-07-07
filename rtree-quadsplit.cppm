export module rtree:quadsplit;
import :aabb;
import :db;

namespace rtree {
struct seeds {
  unsigned ei1;
  unsigned ei2;
};
struct split {
  db::nnid g1;
  db::nnid g2;
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
unsigned pick_next(const db::node &n, aabb g1a, aabb g2a) {
  unsigned res{};
  float max_d = 0;
  for (auto i = 0U; i < n.size; i++) {
    auto e = n.children[i];
    auto d1 = area_of(merge(e.area, g1a));
    auto d2 = area_of(merge(e.area, g2a));
    auto d = (d1 - d2) * (d1 - d2); // Poor'man ABS
    if (d > max_d) {
      res = i;
      max_d = d;
    }
  }
  return res;
}

inline aabb move_to_group(db::nnid g, db::nnid n, unsigned idx) {
  auto e = db::current()->read(n).children[idx];
  db::current()->create_enni(g, e.id, e.area);
  db::current()->remove_eni(n, idx);
  return e.area;
}

inline bool should_move_to_g1(aabb g1area, aabb g2area, aabb iarea,
                              const db::node &g1node, const db::node &g2node) {
  auto ig1area = merge(g1area, iarea);
  auto ig2area = merge(g2area, iarea);

  if (area_of(ig1area) < area_of(ig2area)) {
    return true;
  } else if (area_of(ig1area) > area_of(ig2area)) {
    return false;
  }

  if (area_of(g1area) < area_of(g2area)) {
    return true;
  } else if (area_of(g1area) > area_of(g2area)) {
    return false;
  }

  if (g1node.size < g2node.size) {
    return true;
  } else if (g1node.size < g2node.size) {
    return false;
  }
  return true;
}

split split_node(db::nnid n) {
  // QS1
  auto &node = db::current()->read(n);
  auto [ei1, ei2] = pick_seeds(node);
  auto g1 = db::current()->create_node(node.parent, node.leaf);
  auto g1area = move_to_group(g1, n, ei1);
  auto g2 = db::current()->create_node(node.parent, node.leaf);
  auto g2area = move_to_group(g2, n, ei2);

  while (true) {
    auto &node = db::current()->read(n);
    auto &g1node = db::current()->read(g1);
    auto &g2node = db::current()->read(g2);

    // QS2
    if (node.size == 0) {
      db::current()->delete_node(n);
      break;
    }
    if (g1node.size + node.size <= db::node_lower_limit) {
      move_to_group(g1, n, 0);
      continue;
    }
    if (g2node.size + node.size <= db::node_lower_limit) {
      move_to_group(g2, n, 0);
      continue;
    }

    // QS3
    auto idx = pick_next(node, g1area, g2area);
    auto iarea = node.children[idx].area;

    if (should_move_to_g1(g1area, g2area, iarea, g1node, g2node)) {
      move_to_group(g1, n, idx);
      g1area = merge(g1area, iarea);
    } else {
      move_to_group(g2, n, idx);
      g2area = merge(g2area, iarea);
    }
  }

  return split{g1, g2};
}
} // namespace rtree
