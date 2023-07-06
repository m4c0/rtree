#include "../ecow/ecow.hpp"

auto rtree() {
  using namespace ecow;

  auto m = unit::create<mod>("rtree");
  m->add_part("aabb");
  return m;
}
