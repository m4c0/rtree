export module rtree:aabb;

export namespace rtree {
struct point {
  float x;
  float y;
};
struct aabb {
  point a;
  point b;
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
} // namespace rtree

static_assert(rtree::intersect(rtree::aabb{1, 1, 2, 2},
                               rtree::aabb{1, 1, 2, 2}));
