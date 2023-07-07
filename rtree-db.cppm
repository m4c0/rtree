export module rtree:db;
import :aabb;
import hai;

namespace rtree::db {
constexpr const auto node_limit = 16;

class inconsistency_error {};

class nnid {
  unsigned m_id{};

public:
  constexpr explicit nnid() = default;
  constexpr explicit nnid(unsigned v) : m_id{v + 1} {}

  explicit constexpr operator bool() const noexcept { return m_id > 0; }
  constexpr unsigned index() const noexcept { return m_id - 1; }

  constexpr bool operator==(const nnid &o) const noexcept {
    return m_id == o.m_id;
  }
};
struct link {
  nnid id{};
  aabb area{};
};
struct node {
  nnid parent{};
  unsigned size{};
  bool leaf{};
  bool in_use{};
  link children[node_limit]{};
};

// Simple data facade. Eventually, it can redirect its primitives to a file
// storage
class storage {
  static constexpr const auto initial_cap = 128;
  static constexpr const auto resize_cap = 128;

  hai::array<node> m_nodes{initial_cap};

  [[nodiscard]] node &get(nnid id) {
    unsigned idx = id.index();
    if (idx > m_nodes.size())
      m_nodes.set_capacity(idx + resize_cap);

    return m_nodes[idx];
  }

public:
  [[nodiscard]] const node &read(nnid id) { return get(id); }
  [[nodiscard]] nnid create_node() {
    for (auto i = 0U; i < m_nodes.size(); i++) {
      auto &n = m_nodes[i];
      if (!n.in_use) {
        n.in_use = true;
        return nnid{i};
      }
    }
    auto i = m_nodes.size();
    m_nodes.add_capacity(resize_cap);
    return nnid{i};
  }
  [[nodiscard]] nnid create_node(nnid p, bool leaf) {
    auto res = create_node();
    auto &n = get(res);
    n.parent = p;
    n.leaf = leaf;
    return res;
  }

  void create_enni(nnid p, nnid nn, aabb area) {
    auto pnode = get(p);
    pnode.children[pnode.size++] = {nn, area};
  }
  void adjust_eni(nnid p, unsigned en, aabb area) {
    get(p).children[en].area = area;
  }
  void remove_eni(nnid p, unsigned en) {
    auto pnode = get(p);
    pnode.children[en] = pnode.children[--pnode.size];
  }
};

db::storage *&current() noexcept {
  static db::storage *i{};
  return i;
}
} // namespace rtree::db
