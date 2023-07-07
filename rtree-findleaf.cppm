export module rtree:findleaf;
import :aabb;
import :db;

namespace rtree {
db::nnid find_leaf(db::nnid t, db::nnid eid, aabb earea) {
  auto tn = db::current()->read(t);
  if (!tn.leaf) {
    for (auto i = 0U; i < tn.size; i++) {
      auto &[fp, fi] = tn.children[i];
      if (intersect(fi, earea)) {
        auto e = find_leaf(fp, eid, earea);
        if (e)
          return e;
      }
    }
  } else {
    for (auto i = 0U; i < tn.size; i++) {
      auto &[fp, fi] = tn.children[i];
      if (fp != eid)
        continue;
      if (intersect(fi, earea))
        return t;
    }
  }
  return db::nnid{};
}
} // namespace rtree
