export module rtree:node;
import :aabb;
import hai;
import traits;

export namespace rtree {
struct leaf_data {
  unsigned id;
  aabb area;
};

class non_leaf;
class node {
  aabb m_area{};
  non_leaf *m_parent;

protected:
  static constexpr const auto node_size = 16;

  explicit node(non_leaf *p) : m_parent{p} {}

public:
  static constexpr const auto minimum = 4; // "m" in the article

  virtual ~node() {}

  [[nodiscard]] constexpr aabb area() const noexcept { return m_area; }
  [[nodiscard]] constexpr non_leaf *parent() const noexcept { return m_parent; }

  constexpr void set_area(aabb a) noexcept { m_area = a; }
  constexpr void set_parent(non_leaf *l) noexcept { m_parent = l; }

  [[nodiscard]] virtual bool is_leaf() const noexcept = 0;

  void merge_area(aabb area) noexcept { m_area = merge(m_area, area); }
};
class non_leaf : public node, public hai::varray<hai::uptr<node>> {
public:
  explicit non_leaf(non_leaf *p) : node{p}, varray{node_size} {}
  [[nodiscard]] bool is_leaf() const noexcept override { return false; }
};
class leaf : public node, public hai::varray<leaf_data> {
public:
  explicit leaf(non_leaf *p) : node{p}, varray{node_size} {}
  [[nodiscard]] bool is_leaf() const noexcept override { return true; }
};

constexpr float area_of(const hai::uptr<node> &n) noexcept {
  return area_of(n->area());
}
constexpr float area_of(const leaf_data &n) noexcept { return area_of(n.area); }

constexpr auto aabb_of(const leaf_data &n) noexcept { return n.area; }
constexpr auto aabb_of(const hai::uptr<node> &n) noexcept { return n->area(); }

constexpr auto take(auto *n, unsigned i) noexcept {
  auto res = traits::move((*n)[i]);
  (*n)[i] = traits::move(n->pop_back());
  return res;
}
} // namespace rtree
