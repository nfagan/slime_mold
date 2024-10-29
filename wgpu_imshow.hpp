#pragma once

namespace wgpu {

struct Context {
  void* wgpu_device;
  unsigned int surface_format;
  bool enable_bw;
  bool full_screen;
  int viewport_width;
  int viewport_height;
};

void begin_frame(const Context& context, const void* image_data);
void draw_image(void* render_pass);

}