export module rtree:adjusttree;
import :aabb;
import :common;
import :db;
import :quadsplit;
import silog;

namespace rtree {
auto at3(db::storage *dbs, db::nnid n) {
  auto node = dbs->read(n);
  auto p = node.parent;
  if (p) {
    auto en = find_n_in_parent(dbs, n, p);
    auto eni = calculate_enclosing_rect(node);
    dbs->adjust_eni(p, en, eni);
  }
  return p;
}

void reparent(db::storage *dbs, db::nnid n, db::nnid p) {
  auto node = dbs->read(n);
  auto area = calculate_enclosing_rect(node);
  dbs->create_enni(p, n, area);
}

db::nnid adjust_tree(db::storage *dbs, db::nnid n, db::nnid nn) {
  db::nnid p = at3(dbs, n);
  if (!p) {
    if (!nn)
      return n;

    auto root = dbs->create_node(db::nnid{}, false);
    reparent(dbs, n, root);
    reparent(dbs, nn, root);
    dbs->set_parent(n, root);
    dbs->set_parent(nn, root);
    return root;
  }

  db::nnid pp{};
  if (nn) {
    reparent(dbs, nn, p);

    auto pnode = dbs->read(p);
    if (pnode.size == db::node_limit)
      pp = split_node(dbs, p);
  }

  return adjust_tree(dbs, p, pp);
}
} // namespace rtree
