#include "../ecow/ecow.hpp"
#include "../hai/build.hpp"
#include "../silog/build.hpp"
#include "../traits/build.hpp"

auto rtree() {
  using namespace ecow;

  auto m = unit::create<mod>("rtree");
  m->add_wsdep("hai", hai());
  m->add_wsdep("silog", silog());
  m->add_wsdep("traits", traits());
  m->add_part("aabb");
  m->add_part("db");
  m->add_part("common");
  m->add_part("chooseleaf");
  m->add_part("quadsplit");
  m->add_part("adjusttree");
  m->add_part("insert");
  m->add_part("findleaf");
  m->add_part("condensetree");
  m->add_part("tree");
  return m;
}
