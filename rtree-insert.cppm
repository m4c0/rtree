export module rtree:insert;
import :aabb;
import :db;
import :adjusttree;
import :chooseleaf;

namespace rtree {
db::nnid insert(db::storage *dbs, db::nnid root, db::nnid id, aabb area,
                int max_level = -1) {
  auto l = choose_leaf(dbs, root, area, max_level);
  dbs->create_enni(l, id, area);

  auto &lnode = dbs->read(l);
  if (!lnode.leaf) { // Reinsert from CT6
    dbs->set_parent(id, l);
  }

  db::nnid ll{};
  if (lnode.size == db::node_limit)
    ll = split_node(dbs, l);

  return adjust_tree(dbs, l, ll);
}

} // namespace rtree
