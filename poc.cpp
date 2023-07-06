#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

import hai;
import silog;
import sitime;
import traits;

struct point {
  float x;
  float y;
};
struct aabb {
  point a;
  point b;
};
struct leaf_data {
  unsigned id;
  aabb area;
};

[[nodiscard]] constexpr auto intersect(aabb n, aabb o) noexcept {
  if (n.b.x < o.a.x)
    return false;
  if (o.b.x < n.a.x)
    return false;
  if (n.b.y < o.a.y)
    return false;
  if (o.b.y < n.a.y)
    return false;
  return true;
}
static_assert(intersect(aabb{1, 1, 2, 2}, aabb{1, 1, 2, 2}));

constexpr float area_of(const aabb &a) {
  float w = a.b.x - a.a.x;
  float h = a.b.y - a.a.y;
  return w * h;
}
constexpr float min(float a, float b) { return a < b ? a : b; }
constexpr float max(float a, float b) { return a > b ? a : b; }
constexpr aabb merge(const aabb &p, const aabb &q) {
  point a{
      .x = min(p.a.x, q.a.x),
      .y = min(p.a.y, q.a.y),
  };
  point b{
      .x = max(p.b.x, q.b.x),
      .y = max(p.b.y, q.b.y),
  };
  return {.a = a, .b = b};
}
constexpr float enlargement(const aabb &orig, const aabb &ext) {
  auto new_area = area_of(merge(orig, ext));
  auto old_area = area_of(orig);
  return new_area - old_area;
}

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
  [[nodiscard]] bool is_leaf() const noexcept { return false; }
};
class leaf : public node, public hai::varray<leaf_data> {
public:
  explicit leaf(non_leaf *p) : node{p}, varray{node_size} {}
  [[nodiscard]] bool is_leaf() const noexcept { return true; }
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

class tree {
  hai::uptr<node> m_root{new leaf{nullptr}};

  leaf *choose_leaf(const aabb &area) {
    auto *n = &*m_root;
    while (!n->is_leaf())
      n = cl3_choose_subtree(static_cast<non_leaf *>(n), area);
    return static_cast<leaf *>(n);
  }

  node *cl3_choose_subtree(non_leaf *n, const aabb &area) {
    node *f{};
    float min_enl{};
    float rect_area{};
    for (auto &fn : *n) {
      auto enl = enlargement(fn->area(), area);
      auto ra = area_of(fn->area());
      if ((f == nullptr) || (enl < min_enl) ||
          (enl == min_enl && ra < rect_area)) {
        f = &*fn;
        min_enl = enl;
        rect_area = ra;
        continue;
      }
    }
    return f;
  }

  template <typename Tp> void quad_split(Tp *n, Tp *ll) {
    Tp l{n->parent()};

    // QS1
    auto [s1, s2] = pick_seeds(n);
    if (s1 > s2) {
      l.push_back(take(n, s1));
      ll->push_back(take(n, s2));
    } else {
      l.push_back(take(n, s2));
      ll->push_back(take(n, s1));
    }
    while (n->size() > 0) {
      if (l.size() + n->size() == node::minimum) {
        l.push_back(take(n, 0));
        continue;
      }
      if (ll->size() + n->size() == node::minimum) {
        ll->push_back(take(n, 0));
        continue;
      }

      // QS3
      auto next = take(n, pick_next(n, &l, ll));

      auto a_1 = area_of(l.area());
      auto a_2 = area_of(ll->area());
      auto en_1 = area_of(merge(l.area(), aabb_of(next))) - a_1;
      auto en_2 = area_of(merge(ll->area(), aabb_of(next))) - a_2;
      if (en_1 > en_2) {
        l.push_back(next);
        continue;
      }
      if (en_2 > en_1) {
        ll->push_back(next);
        continue;
      }
      if (a_1 < a_2) {
        l.push_back(next);
        continue;
      }
      if (a_1 > a_2) {
        ll->push_back(next);
        continue;
      }
      if (l.size() < ll->size()) {
        l.push_back(next);
        continue;
      }
      if (l.size() > ll->size()) {
        ll->push_back(next);
        continue;
      }
      l.push_back(next);
    }

    *n = traits::move(l);
  }
  template <typename Tp> auto pick_seeds(const Tp *n) {
    struct pair {
      unsigned s1;
      unsigned s2;
    } res;
    auto worst_d = 0.f;
    for (auto e1 = 0U; e1 < n->size(); e1++) {
      for (auto e2 = e1 + 1; e2 < n->size(); e2++) {
        auto e1i = aabb_of((*n)[e1]);
        auto e2i = aabb_of((*n)[e2]);
        auto j = merge(e1i, e2i);
        auto d = area_of(j) - area_of(e1i) - area_of(e2i);
        if (d > worst_d) {
          worst_d = d;
          res.s1 = e1;
          res.s2 = e2;
        }
      }
    }
    return res;
  }
  template <typename Tp>
  unsigned pick_next(const Tp *n, const Tp *l, const Tp *ll) {
    auto max_d = 0U;
    auto res = 0U;

    for (auto e = 0U; e < n->size(); e++) {
      auto &ei = (*n)[e];
      auto d1 = area_of(merge(l->area(), aabb_of(ei))) - area_of(l->area());
      auto d2 = area_of(merge(ll->area(), aabb_of(ei))) - area_of(ll->area());
      auto d = d1 > d2 ? d1 - d2 : d2 - d1;
      if (d > max_d) {
        max_d = d;
        res = e;
      }
    }

    return res;
  }

  template <typename Tp> void adjust_tree(Tp *n, hai::uptr<node> &nn) {
    if (n == &*m_root) {
      if (!nn)
        return;

      auto old = traits::move(m_root);
      auto *new_nl = new non_leaf(nullptr);
      old->set_parent(new_nl);
      nn->set_parent(new_nl);
      new_nl->push_back(traits::move(old));
      new_nl->push_back(traits::move(nn));
      m_root = hai::uptr<node>(new_nl);
      return;
    }

    non_leaf *p = n->parent();
    at3_adjust(n);

    if (nn) {
      at3_adjust(static_cast<Tp *>(&*nn));
      p->push_back(traits::move(nn));
      if (!p->has_capacity()) {
        hai::uptr<node> pp{new non_leaf{p->parent()}};
        quad_split(p, static_cast<non_leaf *>(&*pp));
        return adjust_tree(p, pp);
      }
    }

    hai::uptr<node> pp{};
    return adjust_tree(p, pp);
  }

  template <typename Tp> void at3_adjust(Tp *n) {
    aabb res = aabb_of((*n)[0]);
    for (auto i = 1U; i < n->size(); i++) {
      res = merge(res, aabb_of((*n)[i]));
    }
    n->set_area(res);
  }

  void for_each_in(const leaf *n, aabb area, auto &fn) const noexcept {
    for (auto &e : *n) {
      if (intersect(e.area, area))
        fn(e.id, e.area);
    }
  }
  void for_each_in(const non_leaf *n, aabb area, auto &fn) const noexcept {
    for (auto &e : *n) {
      if (!intersect(e->area(), area))
        continue;
      if (e->is_leaf()) {
        for_each_in(static_cast<const leaf *>(&*e), area, fn);
      } else {
        for_each_in(static_cast<const non_leaf *>(&*e), area, fn);
      }
    }
  }

public:
  void insert(unsigned id, aabb area) {
    auto l = choose_leaf(area);
    l->push_back(leaf_data{id, area});

    if (l->has_capacity()) {
      hai::uptr<node> ll{};
      adjust_tree(l, ll);
      return;
    }

    hai::uptr<node> ll{new leaf{l->parent()}};
    quad_split(&*l, static_cast<leaf *>(&*ll));
    adjust_tree(l, ll);
  }

  void for_each_in(aabb area, auto &&fn) const noexcept {
    if (m_root->is_leaf()) {
      for_each_in(static_cast<const leaf *>(&*m_root), area, fn);
    } else {
      for_each_in(static_cast<const non_leaf *>(&*m_root), area, fn);
    }
  }

  [[nodiscard]] constexpr const auto &root() const noexcept { return m_root; }
};

void read_file(FILE *in, auto &&fn) {
  fseek(in, SEEK_SET, 0);

  float qkm{};
  float lat{};
  float lng{};
  while (fscanf(in, "%f,%f,%f\n", &qkm, &lat, &lng) == 3) {
    constexpr const auto qscale = 100.f;
    point a{lat, lng};
    point b{lat + qscale * qkm, lng + qscale * qkm};

    fn(aabb{a, b});
  }
}

float min(float a, float b, float c) {
  if (a < b && a < c)
    return a;
  return b < c ? b : c;
}
float max(float a, float b, float c) {
  if (a > b && a > c)
    return a;
  return b > c ? b : c;
}
aabb find_minmax(FILE *in) {
  aabb minmax{
      .a = {9e15, 9e15},
      .b = {-9e15, -9e15},
  };

  read_file(in, [&](aabb area) {
    minmax.a.x = min(minmax.a.x, area.a.x, area.b.x);
    minmax.a.y = min(minmax.a.y, area.a.y, area.b.y);
    minmax.b.x = max(minmax.b.x, area.a.x, area.b.x);
    minmax.b.y = max(minmax.b.y, area.a.y, area.b.y);
  });

  return minmax;
}

point normie(point p, aabb minmax) {
  float w = minmax.b.x - minmax.a.x;
  float h = minmax.b.y - minmax.a.y;
  return {800.f * (p.x - minmax.a.x) / w, 800.f * (p.y - minmax.a.y) / h};
}

void rect(FILE *out, aabb area, const char *colour, unsigned ind) {
  point a = area.a;
  point b = area.b;
  fprintf(out,
          "%*s<rect x='%f' y='%f' width='%f' height='%f' "
          "style='fill:none;stroke:%s'/>\n",
          ind, "", a.x, a.y, b.x - a.x, b.y - a.y, colour);
}

void dump_tree(FILE *out, const leaf *n, unsigned ind) {
  for (const auto &d : *n) {
    rect(out, d.area, "green", ind);
  }
}
void dump_tree(FILE *out, const non_leaf *n, unsigned ind) {
  for (const auto &d : *n) {
    if (d->is_leaf()) {
      rect(out, d->area(), "blue", ind);
      dump_tree(out, static_cast<const leaf *>(&*d), ind + 1);
    } else {
      rect(out, d->area(), "red", ind);
      dump_tree(out, static_cast<const non_leaf *>(&*d), ind + 1);
    }
  }
}
void dump_tree(FILE *out, const tree &t) {
  sitime::stopwatch w{};

  if (t.root()->is_leaf()) {
    dump_tree(out, static_cast<const leaf *>(&*(t.root())), 0);
  } else {
    dump_tree(out, static_cast<const non_leaf *>(&*(t.root())), 0);
  }

  silog::log(silog::info, "Tree dump in %dms", w.millis());
}

tree build_tree(FILE *in) {
  sitime::stopwatch w{};
  tree t{};

  const aabb minmax = find_minmax(in);
  unsigned i = 100;
  read_file(in, [&](aabb area) {
    area.a = normie(area.a, minmax);
    area.b = normie(area.b, minmax);

    // We could use the PLZ, but meh... forgot about it in the cleanup...
    t.insert(i++, area);
  });

  silog::log(silog::info, "Tree build in %dms", w.millis());
  return t;
}

void test_tree(FILE *in, const tree &t) {
  sitime::stopwatch w{};

  const aabb minmax = find_minmax(in);
  unsigned i = 100;
  read_file(in, [&](aabb area) {
    area.a = normie(area.a, minmax);
    area.b = normie(area.b, minmax);

    bool found = false;
    t.for_each_in(area, [&](auto ti, auto ta) {
      if (i == ti)
        found = true;
    });
    if (!found) {
      silog::log(silog::error, "Missing element %d @(%fx%f - %fx%f)", i,
                 area.a.x, area.a.y, area.b.x, area.b.y);
      throw 0;
    }

    i++;
  });

  silog::log(silog::info, "All elements found in %dms", w.millis());
}

void run_poc(FILE *in, FILE *out) {
  tree t = build_tree(in);
  dump_tree(out, t);
  test_tree(in, t);
}

int main(int argc, char **argv) {
  if (argc != 2)
    return 1;

  FILE *in = fopen("data.csv", "r");
  if (!in)
    return 2;

  FILE *out = fopen(argv[1], "w");
  if (!out)
    return 2;

  fprintf(out, "<?xml version='1.0' standalone='no'?>\n");
  fprintf(out, "<svg width='800' height='800' version='1.1' "
               "xmlns='http://www.w3.org/2000/svg'>\n");
  run_poc(in, out);
  fprintf(out, "</svg>\n");
  fclose(out);
}
