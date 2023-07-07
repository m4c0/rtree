export module rtree:tree;
import :aabb;
import :db;
import :adjusttree;
import :chooseleaf;
import :quadsplit;
import hai;
import traits;

export namespace rtree {
class tree {
  db::nnid m_root{};

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

  void for_each_in(aabb area, auto &&fn) const noexcept {}

  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }
};
} // namespace rtree
