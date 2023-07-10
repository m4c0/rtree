export module rtree:condensetree;
import :aabb;
import :common;
import :db;
import :insert;
import hai;

namespace rtree {
db::nnid condense_tree(db::storage *dbs, db::nnid l) {
  struct qd {
    db::nnid id{};
    unsigned depth{};
  };
  db::nnid root;
  hai::varray<qd> q{1024};
  auto depth = 0U;
  auto n = l;
  while (true) {
    auto &node = dbs->read(n);
    auto p = node.parent;
    if (!p) {
      root = n;
      break;
    }

    auto idx = find_n_in_parent(dbs, n, p);
    if (node.size < db::node_lower_limit) {
      // CT3
      dbs->remove_eni(p, idx);
      q.push_back(qd{n, depth});
    } else {
      // CT4
      auto area = calculate_enclosing_rect(node);
      dbs->adjust_eni(p, idx, area);
    }
    n = p;
    depth++;
  }

  // CT6
  for (auto &[n, d] : q) {
    auto &node = dbs->read(n);
    for (auto i = node.size; i > 0; i--) {
      auto &[eid, earea] = node.children[i - 1];
      root = insert(dbs, root, eid, earea, depth - d);
    }
  }

  return root;
}
} // namespace rtree
