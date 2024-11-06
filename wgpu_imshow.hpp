#pragma once

#include <functional>

namespace wgpu {

struct Context {
  bool enable_bw;
  bool full_screen;
  int texture_dim;
};

void* get_device();
unsigned int get_preferred_surface_format();

bool init();
bool need_resize_surface(int w, int h);
void resize_surface(int w, int h);
void begin_frame(const Context& context, const void* image_data);
void render(const std::function<void(void*)>& before_draw_end);

}