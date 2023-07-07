#include "../ecow/ecow.hpp"
#include "../silog/build.hpp"
#include "../sitime/build.hpp"
#include "build.hpp"

class pocify : public ecow::unit {
  void build_self() const override {
    std::cerr << "running poc" << std::endl;

    const auto out = ecow::impl::current_target()->build_path() / "poc.svg";
    const auto cmd = name() + " " + out.string();
    if (std::system(cmd.c_str()) != 0)
      throw std::runtime_error("Proof-of-concept failed");
  }

  void create_self_cdb(std::ostream &) const override {}
  [[nodiscard]] pathset self_objects() const override { return {}; }

public:
  using unit::unit;
};

int main(int argc, char **argv) {
  using namespace ecow;

  auto poc = unit::create<mod>("poc");
  poc->add_wsdep("hai", hai());
  poc->add_wsdep("silog", silog());
  poc->add_wsdep("sitime", sitime());
  poc->add_wsdep("traits", traits());

  auto p_tool = unit::create<tool>("poc");
  p_tool->add_ref(rtree());
  p_tool->add_ref(poc);

  auto all = unit::create<seq>("all");
  all->add_ref(p_tool);
  all->add_unit<pocify>(p_tool->executable().string());
  return run_main(all, argc, argv);
}
