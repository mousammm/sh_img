#include <stdio.h>

#define SH_IMAGE_IMPLEMENTATION
#include "./vendor/sh_img.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Usage: %s <input_image>\n", argv[0]);
    return 1;
  }

  SH_IMG* my_img = sh_load_image(argv[1]);
  if (!my_img) return 1;

  int total_pixels = my_img->width * my_img->height;
  for (int i = 0; i < total_pixels; i++) {
    unsigned char* pixel_bytes = (unsigned char*)&my_img->pixels[i];
    pixel_bytes[1] = 0; 
  }

  printf("\nIMG Saved.\n");
  sh_export_image(my_img, "img", SH_FORMAT_PNG);

  sh_free_image(my_img);
  return 0;
}
