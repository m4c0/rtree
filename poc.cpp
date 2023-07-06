#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2)
    return 1;

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
