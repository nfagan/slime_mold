#include "SoilComponent.hpp"
#include "SoilGUI.hpp"
#include "wgpu_imshow.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_wgpu.h"
#include <stdio.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu_cpp.h>

static WGPUDevice        wgpu_device = nullptr;
static WGPUSurface       wgpu_surface = nullptr;
static WGPUTextureFormat wgpu_preferred_fmt = WGPUTextureFormat_RGBA8Unorm;
static WGPUSwapChain     wgpu_swap_chain = nullptr;
static int               wgpu_swap_chain_width = 0;
static int               wgpu_swap_chain_height = 0;

static void MainLoopStep(void* window);
static bool InitWGPU();
static void print_glfw_error(int error, const char* description);
static void print_wgpu_error(WGPUErrorType error_type, const char* message, void*);

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

  static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  WGPURenderPassColorAttachment color_attachments = {};
  color_attachments.loadOp = WGPULoadOp_Clear;
  color_attachments.storeOp = WGPUStoreOp_Store;
  color_attachments.clearValue = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
  color_attachments.view = wgpuSwapChainGetCurrentTextureView(wgpu_swap_chain);
  WGPURenderPassDescriptor render_pass_desc = {};
  render_pass_desc.colorAttachmentCount = 1;
  render_pass_desc.colorAttachments = &color_attachments;
  render_pass_desc.depthStencilAttachment = nullptr;

  WGPUCommandEncoderDescriptor enc_desc = {};
  WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(wgpu_device, &enc_desc);

  WGPURenderPassEncoder pass = wgpuCommandEncoderBeginRenderPass(encoder, &render_pass_desc);

  //  my renderer
  wgpu::draw_image(pass);

  ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);
  wgpuRenderPassEncoderEnd(pass);

  WGPUCommandBufferDescriptor cmd_buffer_desc = {};
  WGPUCommandBuffer cmd_buffer = wgpuCommandEncoderFinish(encoder, &cmd_buffer_desc);
  WGPUQueue queue = wgpuDeviceGetQueue(wgpu_device);
  wgpuQueueSubmit(queue, 1, &cmd_buffer);
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
  if (!InitWGPU()) {
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
  ImGui_ImplWGPU_Init(wgpu_device, 3, wgpu_preferred_fmt, WGPUTextureFormat_Undefined);
#ifndef IMGUI_DISABLE_FILE_FUNCTIONS
  io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
#endif

  emscripten_set_main_loop_arg(MainLoopStep, window, 0, false);
  return 0;
}

static bool InitWGPU()
{
  wgpu_device = emscripten_webgpu_get_device();
  if (!wgpu_device)
    return false;

  wgpuDeviceSetUncapturedErrorCallback(wgpu_device, print_wgpu_error, nullptr);

  // Use C++ wrapper due to misbehavior in Emscripten.
  // Some offset computation for wgpuInstanceCreateSurface in JavaScript
  // seem to be inline with struct alignments in the C++ structure
  wgpu::SurfaceDescriptorFromCanvasHTMLSelector html_surface_desc = {};
  html_surface_desc.selector = "#canvas";

  wgpu::SurfaceDescriptor surface_desc = {};
  surface_desc.nextInChain = &html_surface_desc;

  wgpu::Instance instance = wgpuCreateInstance(nullptr);
  wgpu::Surface surface = instance.CreateSurface(&surface_desc);
  wgpu::Adapter adapter = {};
  wgpu_preferred_fmt = (WGPUTextureFormat)surface.GetPreferredFormat(adapter);
  wgpu_surface = surface.Release();

  return true;
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

  // React to changes in screen size
  if (width != wgpu_swap_chain_width && height != wgpu_swap_chain_height)
  {
    ImGui_ImplWGPU_InvalidateDeviceObjects();
    if (wgpu_swap_chain)
      wgpuSwapChainRelease(wgpu_swap_chain);
    wgpu_swap_chain_width = width;
    wgpu_swap_chain_height = height;
    WGPUSwapChainDescriptor swap_chain_desc = {};
    swap_chain_desc.usage = WGPUTextureUsage_RenderAttachment;
    swap_chain_desc.format = wgpu_preferred_fmt;
    swap_chain_desc.width = width;
    swap_chain_desc.height = height;
    swap_chain_desc.presentMode = WGPUPresentMode_Fifo;
    wgpu_swap_chain = wgpuDeviceCreateSwapChain(wgpu_device, wgpu_surface, &swap_chain_desc);
    ImGui_ImplWGPU_CreateDeviceObjects();
  }

  wgpu::begin_frame({
    wgpu_device,
    wgpu_preferred_fmt,
    globals.use_bw,
    globals.full_screen_image,
    wgpu_swap_chain_width,
    wgpu_swap_chain_height,
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

static void print_glfw_error(int error, const char* description)
{
  printf("GLFW Error %d: %s\n", error, description);
}

static void print_wgpu_error(WGPUErrorType error_type, const char* message, void*)
{
  const char* error_type_lbl = "";
  switch (error_type)
  {
    case WGPUErrorType_Validation:  error_type_lbl = "Validation"; break;
    case WGPUErrorType_OutOfMemory: error_type_lbl = "Out of memory"; break;
    case WGPUErrorType_Unknown:     error_type_lbl = "Unknown"; break;
    case WGPUErrorType_DeviceLost:  error_type_lbl = "Device lost"; break;
    default:                        error_type_lbl = "Unknown";
  }
  printf("%s error: %s\n", error_type_lbl, message);
}