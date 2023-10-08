#pragma once

namespace wgpu {

struct Context {
  void* wgpu_device;
  unsigned int surface_format;
};

void begin_frame(const Context& context, const void* image_data);
void draw_image(void* render_pass);

}