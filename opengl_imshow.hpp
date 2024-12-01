#pragma once

namespace ogl {

struct Context {
  int surface_width;
  int surface_height;
  bool enable_bw;
  bool full_screen;
  int texture_dim;
};

bool init();
void gui_init(void* window);
void gui_new_frame();
void begin_frame(const Context& context, const void* image_data);
void render();

}