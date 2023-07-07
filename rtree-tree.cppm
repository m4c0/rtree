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
    if (db::current()->read(l).size == db::node_limit) {
      // TODO: where we add node to parent?
      auto [g1, g2] = split_node(l);
      l = g1;
      ll = g2;
    }

    adjust_tree(l, ll);
    if (db::current()->read(m_root).size == 0) {
      m_root = db::current()->create_node(db::nnid{}, false);

      auto l_area = calculate_enclosing_rect(db::current()->read(l));
      db::current()->create_enni(m_root, l, l_area);
      auto ll_area = calculate_enclosing_rect(db::current()->read(ll));
      db::current()->create_enni(m_root, ll, ll_area);
    }
  }

  void for_each_in(aabb area, auto &&fn) const noexcept {}

  [[nodiscard]] constexpr auto root() const noexcept { return m_root; }
};
} // namespace rtree
