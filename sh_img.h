/*
  INSTALL SCRIPT:
    https://raw.githubusercontent.com/mousammm/sh_img/refs/heads/main/sh_img.h
    wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image.h
    wget https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_image_write.h

#define SH_IMG_VIEW_SDL2 
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
  sh_image_view_sdl2(my_img, "");
  sh_free_image(my_img);

  return 0;
}

  ALL the function of <sh_img.h>
    SH_IMG* sh_load_image(const char* file_name);
    void sh_set_pixel(SH_IMG* img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
    bool sh_export_image(SH_IMG* img, const char* base_name, SH_IMG_FORMAT format);
    void sh_free_image(SH_IMG* img);
*/

#ifndef SH_IMG_HEADER
#define SH_IMG_HEADER

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
  int width;
  int height;
  int channels;          // locked at 4 (RGBA) for easy manipulation
  uint32_t* pixels;      // 32-bit buffer array
} SH_IMG;

typedef enum {
  SH_FORMAT_PNG,
  SH_FORMAT_JPG,
  SH_FORMAT_PPM,
  SH_FORMAT_ALL
} SH_IMG_FORMAT;

SH_IMG* sh_load_image(const char* file_name);
void sh_set_pixel(SH_IMG* img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
bool sh_export_image(SH_IMG* img, const char* base_name, SH_IMG_FORMAT format);
void sh_free_image(SH_IMG* img);

#ifdef SH_IMG_VIEW_SDL2
void sh_image_view_sdl2(const SH_IMG* img, const char* window_title);
#endif

#endif 

#ifdef SH_IMAGE_IMPLEMENTATION

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

SH_IMG* sh_load_image(const char* file_name) {
  int w, h, original_channels;
  // Force 4 channels (RGBA) to make manipulation consistent
  unsigned char* raw_data = stbi_load(file_name, &w, &h, &original_channels, 4);
  
  if (!raw_data) {
    fprintf(stderr, "Error: Failed to load image '%s'. Reason: %s\n", file_name, stbi_failure_reason());
    return NULL;
  }

  SH_IMG* img = (SH_IMG*)malloc(sizeof(SH_IMG));
  if (!img) {
    stbi_image_free(raw_data);
    return NULL;
  }

  img->width = w;
  img->height = h;
  img->channels = 4;
  img->pixels = (uint32_t*)raw_data;

  return img;
}

void sh_set_pixel(SH_IMG* img, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  if (!img || x < 0 || x >= img->width || y < 0 || y >= img->height) return;
  
  int index = y * img->width + x;
  unsigned char* pixel_bytes = (unsigned char*)&img->pixels[index];
  pixel_bytes[0] = r;
  pixel_bytes[1] = g;
  pixel_bytes[2] = b;
  pixel_bytes[3] = a;
}

// output raw RGB data to PPM (P6)
static bool internal_save_ppm(const char* filename, int w, int h, const unsigned char* rgb_data) {
  FILE* f = fopen(filename, "wb");
  if (!f) return false;
  fprintf(f, "P6\n%d %d\n255\n", w, h);
  fwrite(rgb_data, 1, w * h * 3, f);
  fclose(f);
  return true;
}

/* create a temp (24bit RGB buffer) from (32bit RGBA) */
static unsigned char* internal_create_rgb_buffer(SH_IMG* img) {
  int pixel_count = img->width * img->height;
  unsigned char* rgb_buffer = (unsigned char*)malloc(pixel_count * 3);
  if (!rgb_buffer) return NULL;

  unsigned char* rgba_bytes = (unsigned char*)img->pixels;
  for (int i = 0; i < pixel_count; i++) {
    rgb_buffer[i * 3 + 0] = rgba_bytes[i * 4 + 0]; // R
    rgb_buffer[i * 3 + 1] = rgba_bytes[i * 4 + 1]; // G
    rgb_buffer[i * 3 + 2] = rgba_bytes[i * 4 + 2]; // B
  }
  return rgb_buffer;
}

bool sh_export_image(SH_IMG* img, const char* base_name, SH_IMG_FORMAT format) {
  if (!img || !img->pixels || !base_name) return false;

  char filename[512];
  unsigned char* rgb_buffer = NULL;
  bool success = true;

  /* PNG RGBA */
  if (format == SH_FORMAT_PNG || format == SH_FORMAT_ALL) {
    snprintf(filename, sizeof(filename), "%s.png", base_name);
    if (stbi_write_png(filename, img->width, img->height, 4, img->pixels, img->width * 4)) {
      printf("Exported: %s\n", filename);
    } else {
      success = false;
    }
  }

  /* JPG/PPM these need to drop the alpha channel */
  if (format == SH_FORMAT_JPG || format == SH_FORMAT_PPM || format == SH_FORMAT_ALL) {
    rgb_buffer = internal_create_rgb_buffer(img);
    if (!rgb_buffer) {
      fprintf(stderr, "Error: Memory allocation failed for RGB conversion buffer.\n");
      return false;
    }

    // EXPORT JPG
    if (format == SH_FORMAT_JPG || format == SH_FORMAT_ALL) {
      snprintf(filename, sizeof(filename), "%s.jpg", base_name);
      if (stbi_write_jpg(filename, img->width, img->height, 3, rgb_buffer, 90)) {
        printf("Exported: %s\n", filename);
      } else {
        success = false;
      }
    }

    // EXPORT PPM
    if (format == SH_FORMAT_PPM || format == SH_FORMAT_ALL) {
      snprintf(filename, sizeof(filename), "%s.ppm", base_name);
      if (internal_save_ppm(filename, img->width, img->height, rgb_buffer)) {
        printf("Exported: %s\n", filename);
      } else {
        success = false;
      }
    }

    free(rgb_buffer);
  }

  return success;
}

void sh_free_image(SH_IMG* img) {
  if (img) {
    if (img->pixels) {
      stbi_image_free(img->pixels);
    }
    free(img);
  }
}

#ifdef SH_IMG_VIEW_SDL2
#include <SDL2/SDL.h>

void sh_image_view_sdl2(const SH_IMG* img, const char* window_title) {
  if (!img || !img->pixels) return;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return;
  }

  // Spin up window exact sized to the loaded image width & height
  SDL_Window* window = SDL_CreateWindow(
    window_title ? window_title : "Image Viewer",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    img->width, img->height, 0
  );
  
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  SDL_Texture* texture = SDL_CreateTexture(
    renderer, 
    SDL_PIXELFORMAT_ARGB8888, 
    SDL_TEXTUREACCESS_STREAMING, 
    img->width, img->height
  );

  // Create and fill frame buffer converting standard pixel array mapping
  uint32_t* display_buffer = (uint32_t*)malloc(img->width * img->height * sizeof(uint32_t));
  if (display_buffer) {
    int total_pixels = img->width * img->height;
    for (int i = 0; i < total_pixels; i++) {
      unsigned char* c = (unsigned char*)&img->pixels[i];
      // Format to 0xAARRGGBB
      display_buffer[i] = (c[3] << 24) | (c[0] << 16) | (c[1] << 8) | c[2];
    }

    // Push directly onto textures
    SDL_UpdateTexture(texture, NULL, display_buffer, img->width * sizeof(uint32_t));
    free(display_buffer);
  }

  // Local event checking frame loop (Blocks main execution dynamically)
  bool close_viewer = false;
  SDL_Event ev;
  while (!close_viewer) {
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_QUIT || 
        (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)) {
        close_viewer = true;
      }
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  // Immediate local scope teardown
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
#endif // SH_IMG_VIEW_SDL2

#endif // SH_IMAGE_IMPLEMENTATION
