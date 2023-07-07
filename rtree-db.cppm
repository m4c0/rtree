export module rtree:db;
import :aabb;
import hai;

namespace rtree::db {
constexpr const auto node_limit = 16;

class nnid {
  unsigned m_id{};

public:
  constexpr explicit nnid() = default;
  constexpr explicit nnid(unsigned v) : m_id{v + 1} {}

  constexpr operator bool() const noexcept { return m_id > 0; }
  constexpr unsigned index() const noexcept { return m_id - 1; }
};
struct link {
  nnid id{};
  aabb area{};
};
struct node {
  nnid parent{};
  unsigned size{};
  bool leaf{};
  link children[node_limit]{};
};

class storage {
  static constexpr const auto initial_cap = 128;

  hai::varray<node> m_nodes{initial_cap};

public:
  [[nodiscard]] const node &read(nnid id) {
    unsigned idx = id.index();
    if (idx > m_nodes.capacity())
      m_nodes.set_capacity(idx + initial_cap);

    return m_nodes[idx];
  }
};

db::storage *&current() noexcept {
  static db::storage *i{};
  return i;
}
} // namespace rtree::db
