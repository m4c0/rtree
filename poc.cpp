#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

import hai;
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

class node {
  aabb m_area{};

public:
  virtual ~node() {}

  [[nodiscard]] constexpr aabb area() const noexcept { return m_area; }

  [[nodiscard]] virtual bool is_leaf() const noexcept = 0;
};
class non_leaf : public node, public hai::varray<hai::uptr<node>> {
public:
  [[nodiscard]] bool is_leaf() const noexcept { return false; }
};
class leaf : public node, hai::varray<leaf_data> {
public:
  [[nodiscard]] bool is_leaf() const noexcept { return true; }

  [[nodiscard]] bool add(unsigned id, aabb area) noexcept {
    if (this->size() == this->capacity()) {
      return false;
    }
    this->push_back(leaf_data{id, area});
    return true;
  }

  using varray::operator[];
  using varray::size;
};

class tree {
  hai::uptr<node> m_root{new leaf{}};

  leaf *choose_leaf(const aabb &area) {
    auto *n = &*m_root;
    while (!n->is_leaf())
      n = cl3_choose_subtree(static_cast<non_leaf *>(n), area);
    return static_cast<leaf *>(n);
  }

  node *cl3_choose_subtree(non_leaf *n, const aabb &area) {
    node *f;
    float min_enl;
    float rect_area;
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

  template <typename Tp> void quad_split(Tp *n, Tp *l, Tp *ll) {
    unsigned s1;
    unsigned s2;
    pick_seeds(n, s1, s2);
    (*l)[0] = traits::move((*n)[s1]);
    (*ll)[0] = traits::move((*n)[s2]);
  }
  template <typename Tp>
  void pick_seeds(const Tp *n, unsigned &s1, unsigned &s2) {
    auto worst_d = 0.f;
    for (auto e1 = 0U; e1 < n->size(); e1++) {
      for (auto e2 = e1 + 1; e2 < n->size(); e2++) {
        auto e1i = (*n)[e1].area;
        auto e2i = (*n)[e2].area;
        auto j = merge(e1i, e2i);
        auto d = area_of(j) - area_of(e1i) - area_of(e2i);
        if (d > worst_d) {
          worst_d = d;
          s1 = e1;
          s2 = e2;
        }
      }
    }
  }

public:
  void insert(unsigned id, aabb area) {
    auto l = choose_leaf(area);
    if (!l->add(id, area)) {
      hai::uptr<node> new_l{new leaf{}};
      hai::uptr<node> ll{new leaf{}};
      quad_split(&*l, static_cast<leaf *>(&*new_l), static_cast<leaf *>(&*ll));
    }
  }
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
aabb find_minmax(FILE *in, FILE *out) {
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
  fprintf(out, "<!-- %f %f %f %f -->\n", minmax.a.x, minmax.a.y, minmax.b.x,
          minmax.b.y);

  return minmax;
}

point normie(point p, aabb minmax) {
  float w = minmax.b.x - minmax.a.x;
  float h = minmax.b.y - minmax.a.y;
  return {800.f * (p.x - minmax.a.x) / w, 800.f * (p.y - minmax.a.y) / h};
}

void run_poc(FILE *in, FILE *out) {
  tree t{};

  const aabb minmax = find_minmax(in, out);
  read_file(in, [&](aabb area) {
    point a = normie(area.a, minmax);
    point b = normie(area.b, minmax);

    fprintf(out,
            "<rect x='%f' y='%f' width='%f' height='%f' "
            "style='fill:none;stroke:black'/>\n",
            a.x, a.y, b.x - a.x, b.y - a.y);

    // We could use the PLZ, but meh... forgot about it in the cleanup...
    // t.insert(i, area);
  });
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

  fprintf(out, "<!DOCTYPE html>\n");
  fprintf(out, "<html><body><svg width=800 height=800>\n");
  run_poc(in, out);
  fprintf(out, "</svg></body></html>\n");
  fclose(out);
}
