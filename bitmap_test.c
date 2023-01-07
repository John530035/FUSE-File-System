#include <stdio.h>

#include "bitmap.h"

#define SIZE 256

/*
int main(int argc, char **argv) {

  long bm[SIZE / sizeof(long)] = {0};
  printf("Initial: \n");
  bitmap_print(bm, SIZE);

  printf("\nSetting bit 2: \n");
  bitmap_put(bm, 2, 1);
  bitmap_print(bm, SIZE);

  printf("\nSetting bit 65: \n");
  bitmap_put(bm, 65, 1);
  bitmap_print(bm, SIZE);

  printf("\nSetting bit 0: \n");
  bitmap_put(bm, 0, 1);
  bitmap_print(bm, SIZE);

  printf("\nSetting bit 255: \n");
  bitmap_put(bm, 255, 1);
  bitmap_print(bm, SIZE);

  return 0;
}
*/
