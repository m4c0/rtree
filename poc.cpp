#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

struct point {
  float x;
  float y;
};
struct aabb {
  point a;
  point b;
};

class tree {
public:
  void insert(unsigned id, aabb area) {}
};

int main(int argc, char **argv) {
  if (argc != 2)
    return 1;

  tree t{};

  for (auto i = 0; i < 10240; i++) {
    // We could use the PLZ, but meh... forgot about it in the cleanup...
    // t.insert(i, area);
  }

  FILE *out = fopen(argv[1], "w");
  if (!out)
    return 2;

  fprintf(out, "<!DOCTYPE html>\n");
  fprintf(out, "<html><body><svg width=800 height=800>\n");
  fprintf(out, "<rect x=50 y=50 width=100 height=200 "
               "style='stroke:black;fill:transparent'>\n");
  fprintf(out, "</svg></body></html>\n");
  fclose(out);
}
