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
  db::storage *m_storage{};
  db::nnid m_root{};

  constexpr void for_each_in(db::nnid n, aabb area, auto &fn) const noexcept {
    auto &node = m_storage->read(n);
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
  explicit constexpr tree(db::storage *s) : m_storage{s} {}

  constexpr void insert(db::nnid id, aabb area) {
    if (!m_root) {
      m_root = m_storage->create_node(db::nnid{}, true);
    }

    m_root = rtree::insert(m_storage, m_root, id, area);
  }

  constexpr void for_each_in(aabb area, auto &&fn) const noexcept {
    if (!m_root)
      return;

    for_each_in(m_root, area, fn);
  }

  [[nodiscard]] constexpr bool remove(db::nnid id, aabb area) {
    auto l = find_leaf(m_storage, m_root, id, area);
    if (!l)
      return false;

    auto idx = find_n_in_parent(m_storage, id, l);
    m_storage->remove_eni(l, idx);

    m_root = condense_tree(m_storage, l);

    auto root = m_storage->read(m_root);
    if (!root.leaf && root.size == 1) {
      auto new_root = root.children[0].id;
      m_storage->delete_node(m_root);
      m_storage->set_parent(new_root, db::nnid{});
      m_root = new_root;
    }

    return true;
  }

  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }
  constexpr void set_root(db::nnid r) noexcept { m_root = r; }
};
} // namespace rtree
