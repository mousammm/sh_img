# sh_img.h

Easy to use wrapper around stb_image.h and stb_image_write.h

```
STEP 1: load img (png,jpg,ppm)
STEP 2: modify the img
STEP 3: export the img to (png,jpg,ppm)
```

## INSTALL
```console
$ wget https://raw.githubusercontent.com/mousammm/sh_img/refs/heads/main/sh_img.h
$ wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h
$ wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_write.h
```

## USAGE
```c
#define SH_IMAGE_IMPLEMENTATION
#include "./sh_img.h"

int main(void) {
  SH_IMG* my_img = sh_load_image("input_img.png");

  int total_pixels = my_img->width * my_img->height;
  for (int i = 0; i < total_pixels; i++) {
    unsigned char* pixel_bytes = (unsigned char*)&my_img->pixels[i];
    pixel_bytes[2] = 0; 
  }

  sh_export_image(my_img, "output_img", SH_FORMAT_PNG);
  sh_free_image(my_img);

  return 0;
}
```
