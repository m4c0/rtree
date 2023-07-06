#include "../ecow/ecow.hpp"

class pocify : public ecow::unit {
  void build_self() const override {
    std::cerr << "running poc" << std::endl;

    const auto out = ecow::impl::current_target()->build_path() / "poc.html";
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

  auto poc = unit::create<tool>("poc");
  poc->add_unit("poc");

  auto all = unit::create<seq>("all");
  all->add_ref(poc);
  all->add_unit<pocify>(poc->executable().string());
  return run_main(all, argc, argv);
}
