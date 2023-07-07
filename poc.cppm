module;
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

export module poc;

import rtree;

import hai;
import silog;
import sitime;
import traits;

using namespace rtree;

void read_file(FILE *in, auto &&fn) {
  fseek(in, SEEK_SET, 0);

  float qkm{};
  float lat{};
  float lng{};
  while (fscanf(in, "%f,%f,%f\n", &qkm, &lat, &lng) == 3) {
    constexpr const auto qscale = 100.f;
    point a{lat, lng};
    point b{lat + qscale * qkm, lng + qscale * qkm};

    fn(aabb{a, b});
  }
}

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
aabb find_minmax(FILE *in) {
  aabb minmax{
      .a = {9e15, 9e15},
      .b = {-9e15, -9e15},
  };

  read_file(in, [&](aabb area) {
    minmax.a.x = min(minmax.a.x, area.a.x, area.b.x);
    minmax.a.y = min(minmax.a.y, area.a.y, area.b.y);
    minmax.b.x = max(minmax.b.x, area.a.x, area.b.x);
    minmax.b.y = max(minmax.b.y, area.a.y, area.b.y);
  });

  return minmax;
}

point normie(point p, aabb minmax) {
  float w = minmax.b.x - minmax.a.x;
  float h = minmax.b.y - minmax.a.y;
  return {800.f * (p.x - minmax.a.x) / w, 800.f * (p.y - minmax.a.y) / h};
}

void rect(FILE *out, int id, aabb area, const char *colour, unsigned ind) {
  point a = area.a;
  point b = area.b;
  fprintf(out,
          "%*s<rect class='r-%d' x='%f' y='%f' width='%f' height='%f' "
          "style='fill:none;stroke:%s'/>\n",
          ind, "", id, a.x, a.y, b.x - a.x, b.y - a.y, colour);
}

void dump_node(FILE *out, db::nnid id, unsigned ind) {
  silog::log(silog::debug, "dump node %d", id.index());

  const auto &node = db::current()->read(id);
  const auto colour = node.leaf ? "blue" : "red";
  for (auto i = 0U; i < node.size; i++) {
    auto &[cid, area] = node.children[i];

    rect(out, cid.index(), area, colour, ind);
    if (!node.leaf)
      dump_node(out, cid, ind + 1);
  }
}
void dump_tree(const char *fn, const tree &t) {
  sitime::stopwatch w{};

  FILE *out = fopen(fn, "w");
  if (!out)
    throw 0;

  fprintf(out, "<?xml version='1.0' standalone='no'?>\n");
  fprintf(out, "<svg width='800' height='800' version='1.1' "
               "xmlns='http://www.w3.org/2000/svg'>\n");

  dump_node(out, t.root(), 0);

  fprintf(out, "</svg>\n");
  fclose(out);

  silog::log(silog::info, "Tree dump in %dms", w.millis());
}

tree build_tree(FILE *in) {
  sitime::stopwatch w{};
  tree t{};

  const aabb minmax = find_minmax(in);
  unsigned i = 100;
  read_file(in, [&](aabb area) {
    // silog::log(silog::debug, "Read %d", i);
    area.a = normie(area.a, minmax);
    area.b = normie(area.b, minmax);

    // We could use the PLZ, but meh... forgot about it in the cleanup...
    t.insert(db::nnid{i++ * 10000}, area);
  });

  silog::log(silog::info, "Tree build in %dms", w.millis());
  return t;
}

void test_tree(FILE *in, const tree &t) {
  sitime::stopwatch w{};

  const aabb minmax = find_minmax(in);
  unsigned i = 100;
  bool failed = false;
  read_file(in, [&](aabb area) {
    area.a = normie(area.a, minmax);
    area.b = normie(area.b, minmax);

    bool found = false;
    t.for_each_in(area, [&](auto ti, auto ta) {
      if (i == ti)
        found = true;
    });
    if (!found) {
      failed = true;
      silog::log(silog::error, "Missing element %d @(%fx%f - %fx%f)", i,
                 area.a.x, area.a.y, area.b.x, area.b.y);
    }

    i++;
  });
  if (failed)
    throw 0;

  silog::log(silog::info, "All elements found in %dms", w.millis());
}

void run_poc(FILE *in, const char *out) {
  db::storage s{};
  db::current() = &s;

  tree t = build_tree(in);
  dump_tree(out, t);
  test_tree(in, t);
}

extern "C" int main(int argc, char **argv) {
  if (argc != 2)
    return 1;

  FILE *in = fopen("data.csv", "r");
  if (!in)
    return 2;

  run_poc(in, argv[1]);
}
