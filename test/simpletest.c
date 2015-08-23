
#include <stdio.h>

void a() { printf("Found a\n"); }
void b() { printf("Found b\n"); }
int main() {
  b();
  a();
  b();
  printf("Done!\n");
  return 0;
}

