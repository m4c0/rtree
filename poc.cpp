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

float min(float a, float b, float c) {
  if (a < b && a < c)
    return a;
  return b < c ? b : c;
}
float max(float a, float b, float c) {
  if (a > b && a > c)
    return a;
  return b > c ? b : c;
}

void run_poc(FILE *in, FILE *out) {
  tree t{};

  constexpr const aabb minmax{
      .a = {9e9, 9e9},
      .b = {-9e9, -9e9},
  };

  float qkm{};
  float lat{};
  float lng{};
  while (fscanf(in, "%f,%f,%f\n", &qkm, &lat, &lng) == 3) {
    point a{lat, lng};
    point b{lat + qkm, lng + qkm};

    fprintf(out,
            "<rect x='%f' y='%f' width='%f' height='%f' "
            "style='fill:none;stroke:yellow'/>\n",
            a.x, a.y, b.x - a.x, b.y - a.y);

    minmax.a.x = min(minmax.a.x, a.x, b.x);
    minmax.a.y = min(minmax.a.y, a.y, b.y);
    minmax.b.x = max(minmax.a.x, a.x, b.x);
    minmax.b.y = max(minmax.a.y, a.y, b.y);

    // We could use the PLZ, but meh... forgot about it in the cleanup...
    // t.insert(i, area);
  }

  fprintf(out, "<!-- %f %f %f %f -->\n", minmax.a.x, minmax.a.y, minmax.b.x,
          minmax.b.y);
}

int main(int argc, char **argv) {
  if (argc != 2)
    return 1;

  FILE *in = fopen("data.csv", "r");
  if (!in)
    return 2;

  FILE *out = fopen(argv[1], "w");
  if (!out)
    return 2;

  fprintf(out, "<!DOCTYPE html>\n");
  fprintf(out, "<html><body><svg width=800 height=800>\n");
  run_poc(in, out);
  fprintf(out, "</svg></body></html>\n");
  fclose(out);
}
