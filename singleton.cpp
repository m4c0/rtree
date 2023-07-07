module rtree;
import :db;

rtree::db::storage *&rtree::db::current() noexcept {
  static rtree::db::storage *i{};
  return i;
}
