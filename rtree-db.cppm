export module rtree:db;
import :aabb;
import hai;

namespace rtree::db {
constexpr const auto node_limit = 16;

struct link {
  unsigned id;
  aabb area;
};
struct node {
  unsigned parent;
  unsigned size;
  bool leaf;
  link children[node_limit];
};

class storage {
  static constexpr const auto initial_cap = 128;

  hai::varray<node> m_nodes{initial_cap};

public:
  [[nodiscard]] const node &read(unsigned id) {
    if (id < m_nodes.capacity())
      m_nodes.set_capacity(id + initial_cap);

    return m_nodes[id];
  }
};
} // namespace rtree::db
