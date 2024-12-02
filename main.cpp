#include "slime_mold_component.hpp"
#include "gui.hpp"
#include "imshow.hpp"

#ifdef SM_IS_WGPU
#define SM_IS_EMSCRIPTEN (1)
#endif

#ifdef SM_IS_OPENGL
#define SM_IS_EMSCRIPTEN (0)
#endif

#include <imgui.h>

#ifdef SM_IS_WGPU
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include <GLFW/glfw3.h>

//  ------------------------------------------------------------------------------------

static void main_loop(void* window);

namespace {

struct {
  SlimeMoldComponent sm;
  bool use_bw{true};
  bool full_screen_image{};
  float cursor_x{};
  float cursor_y{};
  float dir_image_mix{};
} globals;

float main_update() {
  return globals.sm.update();
}

void main_gui_update(float sim_dt) {
  const float fps = ImGui::GetIO().Framerate;

  gfx::gui_new_frame();

  auto res = render_gui(globals.sm, {
    fps, sim_dt, &globals.use_bw, &globals.full_screen_image,
    &globals.dir_image_mix, globals.cursor_x, globals.cursor_y});
  globals.sm.on_gui_update(res);
}

void main_render() {
  gfx::render();
}

#if SM_IS_EMSCRIPTEN
int mouse_move_callback(int, const EmscriptenMouseEvent* e, void*) {
  globals.cursor_x = float(e->clientX);
  globals.cursor_y = float(e->clientY);
  return 0;
}
#endif

} //  anon

#if SM_IS_EMSCRIPTEN
//  https://github.com/ocornut/imgui/issues/6640
EM_JS(int, browser_get_width, (), {
  const { width, height } = canvas.getBoundingClientRect();
  return width;
});

EM_JS(int, browser_get_height, (), {
  const { width, height } = canvas.getBoundingClientRect();
  return height;
});

static void get_window_dimensions(int* dw, int* dh) {
  *dw = browser_get_width();
  *dh = browser_get_height();
}
#endif

int main(int, char**) {
#if SM_IS_EMSCRIPTEN
  emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_move_callback);
#endif

  void* window = gfx::boot();
  if (!window) {
    return 1;
  }

#if SM_IS_EMSCRIPTEN
  emscripten_set_main_loop_arg(main_loop, window, 0, false);
#else
  while (!glfwWindowShouldClose((GLFWwindow*) window)) {
    main_loop(window);
  }
#endif
  gfx::terminate();

  return 0;
}

void main_begin_frame(GLFWwindow* window) {
  int width, height;
  glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);

  int window_width;
  int window_height;
  glfwGetWindowSize(window, &window_width, &window_height);

#if SM_IS_EMSCRIPTEN
  int browser_w;
  int browser_h;
  get_window_dimensions(&browser_w, &browser_h);

  if (browser_w != width || browser_h != height) {
    glfwSetWindowSize((GLFWwindow*) window, browser_w, browser_h);
  }
#endif

  const uint8_t* tex_data = globals.sm.read_rgbau8_image_data();
  int dir_im_dim{};
  const uint8_t* dir_im_data = globals.sm.read_r_dir_image_data(&dir_im_dim);

  gfx::begin_frame({
    width,
    height,
    globals.use_bw,
    globals.full_screen_image,
    globals.sm.get_texture_dim(),
    globals.dir_image_mix
  }, tex_data, dir_im_data, dir_im_dim);
}

static void main_loop(void* window) {
  glfwPollEvents();
  const float sim_dt = main_update();
  main_begin_frame((GLFWwindow*) window);
  main_gui_update(sim_dt);
  main_render();
}