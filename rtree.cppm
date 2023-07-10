export module rtree;
export import :aabb;
export import :db;
export import :tree;

module :private;

static_assert([] {
  // Test if rtree is constexpr-able
  rtree::db::storage s{};
  rtree::tree t{&s};
  t.insert(rtree::db::nnid{10}, {});
  t.for_each_in({}, [](auto, auto) {});
  return t.remove(rtree::db::nnid{10}, {});
}());
