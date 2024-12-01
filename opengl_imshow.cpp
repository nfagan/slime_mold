#include "opengl_imshow.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <iostream>

namespace {

struct {
  bool glfw_init{};
  bool init{};
  bool prepared{};
  GLFWwindow* window{};
  int width{};
  int height{};
  int texture_dim{};
  GLuint texture{};
  GLuint program{};
  GLuint vao{};
  GLuint dummy_texture{};
  bool use_dir_image{};
  int main_image_uniform_location{};
  int dir_image_uniform_location{};
  int dir_image_mix_uniform_location{};
  int bw_uniform_location{};
  int dir_texture_dim{};
  GLuint dir_texture{};
  bool render_bw{};
  float dir_image_mix{};
} globals;

bool init_context() {
  if (!glfwInit()) {
    return false;
  } else {
    globals.glfw_init = true;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);
  GLFWwindow* window = glfwCreateWindow(1024, 1024, "", nullptr, nullptr);
  if (!window) {
    return false;
  } else {
    globals.window = window;
  }

  glfwMakeContextCurrent(window);
  gladLoadGL();
  glfwShowWindow(window);
  glfwSwapInterval(1);
  return true;
}

bool init_imgui(GLFWwindow* window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 150");
  return true;
}

bool create_main_texture(int dim) {
  if (globals.texture) {
    glDeleteTextures(1, &globals.texture);
    globals.texture = 0;
  }

  glGenTextures(1, &globals.texture);
  glBindTexture(GL_TEXTURE_2D, globals.texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dim, dim, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);
  globals.texture_dim = dim;
  return true;
}

bool create_dir_texture(int dim) {
  if (globals.dir_texture) {
    glDeleteTextures(1, &globals.dir_texture);
    globals.dir_texture = 0;
  }

  glGenTextures(1, &globals.dir_texture);
  glBindTexture(GL_TEXTURE_2D, globals.dir_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, dim, dim, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);
  globals.dir_texture_dim = dim;
  return true;
}

bool create_dummy_texture() {
  if (globals.dummy_texture) {
    glDeleteTextures(1, &globals.dummy_texture);
    globals.dummy_texture = 0;
  }

  const int dim = 1;
  glGenTextures(1, &globals.dummy_texture);
  glBindTexture(GL_TEXTURE_2D, globals.dummy_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, dim, dim, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
  glBindTexture(GL_TEXTURE_2D, 0);
  return true;
}

bool create_program(
  const char* vertex_shader_text, const char* fragment_shader_text, GLuint* dst_prog) {
  //
  const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, nullptr);
  glCompileShader(vertex_shader);

  {
    char info_log[512];
    int success{};
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
      std::cout << info_log << std::endl;
      return false;
    }
  }

  const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, nullptr);
  glCompileShader(fragment_shader);

  {
    char info_log[512];
    int success{};
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
      std::cout << info_log << std::endl;
      return false;
    }
  }

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  {
    char info_log[512];
    int success{};
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(program, 512, nullptr, info_log);
      std::cout << info_log << std::endl;
      return false;
    }
  }

  *dst_prog = program;
  return true;
}

bool init_program() {
  const char* vertex_shader_text =
    "#version 330 core\n"
    "layout (location = 0) in vec4 position;\n"
    "out vec2 uv;\n"
    "void main() {\n"
    " uv = position.xy * 0.5 + 0.5;\n"
    " gl_Position = vec4(position.x, position.y, 0.0, 1.0);\n"
    "}";

  const char* fragment_shader_text =
    "#version 330 core\n"
    "in vec2 uv;\n"
    "out vec4 frag_color;\n"
    "uniform sampler2D image;\n"
    "uniform sampler2D dir_image;\n"
    "uniform int bw;\n"
    "uniform float dir_image_mix;\n"
    "void main() {\n"
    " vec3 col = texture(image, uv).rgb;\n"
    " float dir_color = texture(dir_image, uv).r;\n"
    " if (bw == 1) {\n"
    "   col = vec3((col.r + col.g + col.b) / 3);\n"
    " }\n"
    " frag_color = vec4(mix(col, vec3(dir_color), dir_image_mix), 1.0);\n"
    "}";

  if (!create_program(vertex_shader_text, fragment_shader_text, &globals.program)) {
    return false;
  }

  {
    int loc = glGetUniformLocation(globals.program, "image");
    if (loc == -1) {
      return false;
    } else {
      globals.main_image_uniform_location = loc;
    }
  }
  {
    int loc = glGetUniformLocation(globals.program, "dir_image");
    if (loc == -1) {
      return false;
    } else {
      globals.dir_image_uniform_location = loc;
    }
  }
  {
    int loc = glGetUniformLocation(globals.program, "dir_image_mix");
    if (loc == -1) {
      return false;
    } else {
      globals.dir_image_mix_uniform_location = loc;
    }
  }
  {
    int loc = glGetUniformLocation(globals.program, "bw");
    if (loc == -1) {
      return false;
    } else {
      globals.bw_uniform_location = loc;
    }
  }

  return true;
}

bool init_vao() {
  struct Position {
    float x;
    float y;
    float z;
    float w;
  };

  static const Position vertices[6] = {
    {-1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, -1.0f, 0.0f, 1.0f},
    {-1.0f, 1.0f, 0.0f, 1.0f},
    {-1.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, -1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f}
  };

  GLuint vertex_position_buffer;
  glGenBuffers(1, &vertex_position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Position), (void*) 0);
  glBindVertexArray(0);

  globals.vao = vertex_array;

  return true;
}

bool init_drawable_components() {
  if (!init_program()) {
    return false;
  }

  if (!init_vao()) {
    return false;
  }

  return true;
}

} //  anon

void* ogl::boot() {
  if (!init_context()) {
    return nullptr;
  }

  if (!init_imgui(globals.window)) {
    return nullptr;
  }

  if (!init_drawable_components()) {
    return nullptr;
  }

  globals.init = true;
  return globals.window;
}

void ogl::gui_new_frame() {
  if (!globals.init) {
    return;
  }
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ogl::begin_frame(
  const Context& context, const void* image_data, const uint8_t* dir_im, int dir_im_dim) {
  //
  globals.prepared = false;
  globals.use_dir_image = false;
  globals.dir_image_mix = context.dir_image_mix;

  if (!globals.init) {
    return;
  }

  globals.width = context.surface_width;
  globals.height = context.surface_height;
  globals.render_bw = context.enable_bw;

  if (context.texture_dim != globals.texture_dim) {
    if (!create_main_texture(context.texture_dim)) {
      return;
    }
  }

  if (dir_im_dim != globals.dir_texture_dim) {
    if (!create_dir_texture(dir_im_dim)) {
      return;
    }
  }

  if (!globals.dummy_texture) {
    if (!create_dummy_texture()) {
      return;
    }
  }

  {
    const int dim = context.texture_dim;
    glBindTexture(GL_TEXTURE_2D, globals.texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dim, dim, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
  }

  if (dir_im) {
    const int dim = dir_im_dim;
    glBindTexture(GL_TEXTURE_2D, globals.dir_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dim, dim, GL_RED, GL_UNSIGNED_BYTE, dir_im);
    globals.use_dir_image = true;
  }

  globals.prepared = true;
}

void ogl::render() {
  if (!globals.prepared) {
    return;
  }
  ImGui::Render();
  glViewport(0, 0, globals.width, globals.height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(globals.program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, globals.texture);
  glActiveTexture(GL_TEXTURE1);
  if (globals.use_dir_image) {
    glBindTexture(GL_TEXTURE_2D, globals.dir_texture);
  } else {
    glBindTexture(GL_TEXTURE_2D, globals.dummy_texture);
  }
  glUniform1i(globals.main_image_uniform_location, 0);
  glUniform1i(globals.dir_image_uniform_location, 1);
  glUniform1i(globals.bw_uniform_location, int(globals.render_bw));
  glUniform1f(globals.dir_image_mix_uniform_location, globals.dir_image_mix);
  glBindVertexArray(globals.vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(globals.window);
}

void ogl::terminate() {
  if (globals.window) {
    glfwDestroyWindow(globals.window);
    globals.window = nullptr;
  }
  if (globals.glfw_init) {
    glfwTerminate();
    globals.glfw_init = false;
  }
  globals.init = false;
}