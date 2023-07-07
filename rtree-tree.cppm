export module rtree:tree;
import :aabb;
import :db;
import :condensetree;
import :findleaf;
import :insert;
import :quadsplit;
import hai;
import traits;

export namespace rtree {
class tree {
  db::nnid m_root{};

  void for_each_in(db::nnid n, aabb area, auto &fn) const noexcept {
    auto &node = db::current()->read(n);
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

    m_root = rtree::insert(m_root, id, area);
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

    auto idx = find_n_in_parent(id, l);
    db::current()->remove_eni(l, idx);

    condense_tree(l);

    return true;
  }

  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }
};
} // namespace rtree
