#include "SoilComponent.hpp"
#include "SoilGUI.hpp"
#include "wgpu_imshow.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include <stdio.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>

static void MainLoopStep(void* window);
static void print_glfw_error(int error, const char* description);

namespace {

struct {
  SoilComponent soil;
  bool use_bw{true};
  bool full_screen_image{};
  float cursor_x{};
  float cursor_y{};
} globals;

float main_update() {
  return globals.soil.update();
}

void main_gui_update(float sim_dt) {
  const float fps = ImGui::GetIO().Framerate;

  ImGui_ImplWGPU_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  auto res = SoilGUI::render(globals.soil, {
    fps, sim_dt, &globals.use_bw, &globals.full_screen_image, globals.cursor_x, globals.cursor_y});
  globals.soil.on_gui_update(res);
}

void main_render() {
  ImGui::Render();
  wgpu::render([](void* rp) {
    auto render_pass = (WGPURenderPassEncoder) rp;
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), render_pass);
  });
}

int mouse_move_callback(int, const EmscriptenMouseEvent* e, void*) {
  globals.cursor_x = float(e->clientX);
  globals.cursor_y = float(e->clientY);
  return 0;
}

} //  anon

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

int main(int, char**)
{
  emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, mouse_move_callback);

  glfwSetErrorCallback(print_glfw_error);
  if (!glfwInit()) {
    return 1;
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  GLFWwindow* window = glfwCreateWindow(1280, 720, "SlimeMold", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }

  // Initialize the WebGPU environment
  if (!wgpu::init()) {
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }

  glfwShowWindow(window);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.IniFilename = nullptr;
  ImGui::StyleColorsDark();
  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOther(window, true);
  ImGui_ImplWGPU_Init(
    (WGPUDevice) wgpu::get_device(), 3,
    (WGPUTextureFormat) wgpu::get_preferred_surface_format(), WGPUTextureFormat_Undefined);
#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
  io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
#endif

  emscripten_set_main_loop_arg(MainLoopStep, window, 0, false);
  return 0;
}

void main_begin_frame(GLFWwindow* window) {
  int width, height;
  glfwGetFramebufferSize((GLFWwindow*)window, &width, &height);

  int window_width;
  int window_height;
  glfwGetWindowSize(window, &window_width, &window_height);

  int browser_w;
  int browser_h;
  get_window_dimensions(&browser_w, &browser_h);

  if (browser_w != width || browser_h != height) {
    glfwSetWindowSize((GLFWwindow*) window, browser_w, browser_h);
  }

  if (wgpu::need_resize_surface(width, height)) {
    ImGui_ImplWGPU_InvalidateDeviceObjects();
    wgpu::resize_surface(width, height);
    ImGui_ImplWGPU_CreateDeviceObjects();
  }

  wgpu::begin_frame({
    globals.use_bw,
    globals.full_screen_image,
    globals.soil.get_texture_dim()
  }, globals.soil.get_soil()->read_rgbau8_image_data());
}

static void MainLoopStep(void* window)
{
  /*
   * update
   */

  glfwPollEvents();
  const float sim_dt = main_update();

  /*
   * begin frame
   */

  main_begin_frame((GLFWwindow*) window);

  /*
   * gui update
   */

  main_gui_update(sim_dt);

  /*
   * gpu render pass
   */
  main_render();
}

static void print_glfw_error(int error, const char* description) {
  printf("GLFW Error %d: %s\n", error, description);
}