#pragma once

#include <cstdint>

namespace ogl {

struct Context {
  int surface_width;
  int surface_height;
  bool enable_bw;
  bool full_screen;
  int texture_dim;
  float dir_image_mix;
};

void* boot();
void terminate();
void gui_new_frame();
void begin_frame(
  const Context& context, const void* image_data, const uint8_t* dir_image, int dir_im_dim);
void render();

}