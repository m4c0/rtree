export module rtree:insert;
import :aabb;
import :db;
import :adjusttree;
import :chooseleaf;

namespace rtree {
db::nnid insert(db::nnid root, db::nnid id, aabb area) {
  auto l = choose_leaf(root, area);
  db::current()->create_enni(l, id, area);

  db::nnid ll{};
  if (db::current()->read(l).size == db::node_limit)
    ll = split_node(l);

  return adjust_tree(l, ll);
}

} // namespace rtree
