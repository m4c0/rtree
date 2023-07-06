export module rtree:tree;
import :aabb;
import :node;
import hai;
import traits;

export namespace rtree {
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
    at3_adjust(n);
    if (nn)
      at3_adjust(static_cast<Tp *>(&*nn));

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
    if (nn) {
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
} // namespace rtree
