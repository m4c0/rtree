export module rtree:tree;
import :aabb;
import :db;
import :adjusttree;
import :chooseleaf;
import :findleaf;
import :quadsplit;
import hai;
import traits;

export namespace rtree {
class tree {
  db::nnid m_root{};

  void for_each_in(db::nnid n, aabb area, auto &fn) const noexcept {
    auto node = db::current()->read(n);
    for (auto i = 0U; i < node.size; i++) {
      auto &e = node.children[i];
      if (!intersect(area, e.area))
        continue;
      if (node.leaf) {
        fn(e.id, e.area);
      } else {
        for_each_in(e.id, area, fn);
      }
    }
  }

public:
  void insert(db::nnid id, aabb area) {
    if (!m_root) {
      m_root = db::current()->create_node(db::nnid{}, true);
    }

    auto l = choose_leaf(m_root, area);
    db::current()->create_enni(l, id, area);

    db::nnid ll{};
    if (db::current()->read(l).size == db::node_limit)
      ll = split_node(l);

    m_root = adjust_tree(l, ll);
  }

  void for_each_in(aabb area, auto &&fn) const noexcept {
    if (!m_root)
      return;

    for_each_in(m_root, area, fn);
  }

  [[nodiscard]] bool remove(db::nnid id, aabb area) {
    auto l = find_leaf(m_root, id, area);
    if (!l)
      return false;

    return true;
  }

  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }
};
} // namespace rtree
