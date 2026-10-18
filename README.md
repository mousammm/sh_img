# sh_img.h

Easy to use wrapper around stb_image.h and stb_image_write.h

## INSTALL
```console
mkdir img_view && cd img_view &&
wget https://raw.githubusercontent.com/mousammm/sh_img/refs/heads/main/sh_img.h &&
wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h &&
wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_write.h &&
cd ..
```

## USAGE
```c
#define SH_IMG_VIEW_SDL2 
#define SH_IMAGE_IMPLEMENTATION
#include "./sh_img.h"

int main(void) {
  /* STEP 1: Load the image. */
  SH_IMG* my_img = sh_load_image("input_img.png");

  /* STEP 2: Modify the image. */
  int total_pixels = my_img->width * my_img->height;
  for (int i = 0; i < total_pixels; i++) {
    unsigned char* pixel_bytes = (unsigned char*)&my_img->pixels[i];
    pixel_bytes[2] = 0; 
  }

  /* STEP 3: Export the image to (ppm,png,jpg). */
  sh_export_image(my_img, "output_img", SH_FORMAT_PNG);

  /* OPTIONAL: Open the image in a sdl2 window. */
  sh_image_view_sdl2(my_img, "");

  /* CLEANUP. */
  sh_free_image(my_img);

  return 0;
}
```
