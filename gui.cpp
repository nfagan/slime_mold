#include "gui.hpp"
#include "slime_mold_component.hpp"
#include "slime_mold.hpp"
#include <imgui.h>

#ifdef SM_IS_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

/*
 * https://stackoverflow.com/questions/74755250/how-pass-a-large-array-from-js-to-c-using-emscripten
 * https://stackoverflow.com/questions/50615377/how-do-you-call-a-c-function-that-takes-or-returns-a-struct-by-value-from-js-v
 * https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
 */

class GUIData {
public:
  void set_quality_preset(std::string p) { quality_preset = p; }
  std::string check_quality_preset() { auto p = quality_preset; quality_preset = ""; return p; }

  void set_style_preset(std::string p) { style_preset = p; }
  std::string check_style_preset() { auto p = style_preset; style_preset = ""; return p; }

  void set_text(std::string p) { text = p; }
  std::string check_text() { auto p = text; text = ""; return p; }

  void set_direction_influence_image_path(std::string p) { dir_influence_image_path = p; }
  std::string check_direction_influence_image_path() { auto p = dir_influence_image_path; dir_influence_image_path = ""; return p; }

  void disable_direction_influence_image() { set_dir_influence_image_enabled = false; }
  int should_disable_direction_influence_image() {
    auto v =
      set_dir_influence_image_enabled.has_value() && !set_dir_influence_image_enabled.value();
    set_dir_influence_image_enabled = std::nullopt;
    return v;
  }

  void set_direction_influence_scale(float v) { dir_influence_scale = v; }
  void set_time_scale(float v) { time_scale = v; }
  void set_direction_influence_render_mix(float v) { dir_influence_render_mix = v; }

  float check_direction_influence_scale() {
    auto v = dir_influence_scale.has_value() ? dir_influence_scale.value() : -1.0f;
    dir_influence_scale = std::nullopt;
    return v;
  }

  float check_time_scale() {
    auto v = time_scale.has_value() ? time_scale.value() : -1.0f;
    time_scale = std::nullopt;
    return v;
  }

  float check_direction_influence_render_mix() {
    auto v = dir_influence_render_mix.has_value() ? dir_influence_render_mix.value() : -1.0f;
    dir_influence_render_mix = std::nullopt;
    return v;
  }

  void do_set_debug_gui_enabled(bool v) { set_debug_gui_enabled = v; }
  int check_set_debug_gui_enabled() {
    auto v = set_debug_gui_enabled;
    set_debug_gui_enabled = -1;
    return v;
  }

  int check_render_bw() { auto v = render_bw; render_bw = -1; return v; }
  void set_render_bw(bool v) { render_bw = int(v); }

  int check_enabled() { auto v = enabled; enabled = -1; return v; }
  void set_enabled(bool v) { enabled = int(v); }

private:
  std::string quality_preset;
  std::string style_preset;
  std::string text;
  std::string dir_influence_image_path;
  int set_debug_gui_enabled{-1};
  int render_bw{-1};
  int enabled{-1};
  std::optional<bool> set_dir_influence_image_enabled;
  std::optional<float> dir_influence_scale;
  std::optional<float> time_scale;
  std::optional<float> dir_influence_render_mix;
};

EMSCRIPTEN_BINDINGS(my_class_example) {
  class_<GUIData>("GUIData")
    .constructor<>()
    .function("check_quality_preset", &GUIData::check_quality_preset)
    .function("set_quality_preset", &GUIData::set_quality_preset)
    .function("check_style_preset", &GUIData::check_style_preset)
    .function("set_style_preset", &GUIData::set_style_preset)
    .function("check_text", &GUIData::check_text)
    .function("set_text", &GUIData::set_text)
    .function("disable_direction_influence_image", &GUIData::disable_direction_influence_image)
    .function("should_disable_direction_influence_image", &GUIData::should_disable_direction_influence_image)
    .function("check_direction_influence_scale", &GUIData::check_direction_influence_scale)
    .function("set_direction_influence_scale", &GUIData::set_direction_influence_scale)
    .function("check_time_scale", &GUIData::check_time_scale)
    .function("set_time_scale", &GUIData::set_time_scale)
    .function("check_direction_influence_render_mix", &GUIData::check_direction_influence_render_mix)
    .function("set_direction_influence_render_mix", &GUIData::set_direction_influence_render_mix)
    .function("check_set_debug_gui_enabled", &GUIData::check_set_debug_gui_enabled)
    .function("do_set_debug_gui_enabled", &GUIData::do_set_debug_gui_enabled)
    .function("check_direction_influence_image_path", &GUIData::check_direction_influence_image_path)
    .function("set_direction_influence_image_path", &GUIData::set_direction_influence_image_path)
    .function("set_render_bw", &GUIData::set_render_bw)
    .function("check_render_bw", &GUIData::check_render_bw)
    .function("set_enabled", &GUIData::set_enabled)
    .function("check_enabled", &GUIData::check_enabled)
  ;
}

namespace {

struct WebGUIResult {
  std::string quality_preset;
  std::string style_preset;
  std::string text;
  std::string direction_influence_image_path;
  std::optional<bool> set_dir_influence_image_enabled;
  std::optional<float> time_scale;
  std::optional<float> direction_influence_scale;
  std::optional<float> direction_influence_render_mix;
  std::optional<bool> set_debug_gui_enabled;
  std::optional<bool> render_bw;
  std::optional<bool> enabled;
};

void web_gui_init() {
  EM_ASM({
    const script = document.createElement('script');
    script.src = 'gui.js';
    document.head.appendChild(script);
  });
}

WebGUIResult web_gui_update() {
  WebGUIResult res{};

  const int has_gui = EM_ASM_INT({
    const div = document.getElementById('gui');
    return div !== null;
  });

  if (!has_gui) {
    return res;
  }

  {
    char* str = (char*) EM_ASM_PTR({
      const div = document.getElementById('gui');
      const preset = div.instance.check_quality_preset();
      return stringToNewUTF8(preset);
    });
    res.quality_preset = str;
    free(str);
  }
  {
    char* str = (char*) EM_ASM_PTR({
      const div = document.getElementById('gui');
      const preset = div.instance.check_style_preset();
      return stringToNewUTF8(preset);
    });
    res.style_preset = str;
    free(str);
  }
  {
    char* str = (char*) EM_ASM_PTR({
      const div = document.getElementById('gui');
      const preset = div.instance.check_text();
      return stringToNewUTF8(preset);
    });
    res.text = str;
    free(str);
  }
  {
    char* str = (char*) EM_ASM_PTR({
      const div = document.getElementById('gui');
      const preset = div.instance.check_direction_influence_image_path();
      return stringToNewUTF8(preset);
    });
    res.direction_influence_image_path = str;
    free(str);
  }
  {
    int v = EM_ASM_INT({
      const div = document.getElementById('gui');
      return div.instance.should_disable_direction_influence_image();
    });
    if (v) { res.set_dir_influence_image_enabled = false; }
  }
  {
    float v = EM_ASM_DOUBLE({
      const div = document.getElementById('gui');
      return div.instance.check_time_scale();
    });
    if (v >= 0.0f) { res.time_scale = v; }
  }
  {
    float v = EM_ASM_DOUBLE({
      const div = document.getElementById('gui');
      return div.instance.check_direction_influence_scale();
    });
    if (v >= 0.0f) { res.direction_influence_scale = v; }
  }
  {
    float v = EM_ASM_DOUBLE({
      const div = document.getElementById('gui');
      return div.instance.check_direction_influence_render_mix();
    });
    if (v >= 0.0f) { res.direction_influence_render_mix = v; }
  }
  {
    int v = EM_ASM_INT({
      const div = document.getElementById('gui');
      return div.instance.check_set_debug_gui_enabled();
    });
    if (v >= 0) { res.set_debug_gui_enabled = bool(v); }
  }
  {
    int v = EM_ASM_INT({
      const div = document.getElementById('gui');
      return div.instance.check_render_bw();
    });
    if (v >= 0) { res.render_bw = bool(v); }
  }
  {
    int v = EM_ASM_INT({
      const div = document.getElementById('gui');
      return div.instance.check_enabled();
    });
    if (v >= 0) { res.enabled = bool(v); }
  }

  return res;
}

} //  anon

#endif

void init_gui() {
#ifdef SM_IS_EMSCRIPTEN
  web_gui_init();
#endif
}

GUIUpdateResult render_gui(SlimeMoldComponent& component, const GUIParams& params) {
  float fps = params.app_fps;
  float sim_t = params.sim_t;
  bool* use_bw = params.use_bw;
  bool* full_screen = params.full_screen;

  static uint32_t update_index{};
  static float last_sim_t{};
#ifdef SM_IS_EMSCRIPTEN
  static bool debug_gui_enabled{false};
#else
  static bool debug_gui_enabled{true};
#endif

  if ((++update_index % 30) == 0) {
    last_sim_t = sim_t;
  }

  GUIUpdateResult result;

  bool high_res{};
  bool med_res{};
  bool low_res{};

  bool mid_coh{};
  bool high_coh{};
  bool chaotic{};
  bool fragile{};
  bool clustered{};
  const bool is_high_res = gen::SlimeMoldConfig::texture_dim > 512;

  const float min_time_scale = 0.01f;
  const float max_time_scale = 8.0f;

#ifdef SM_IS_EMSCRIPTEN
  auto web_gui_res = web_gui_update();
  high_res = web_gui_res.quality_preset == "high";
  med_res = web_gui_res.quality_preset == "med";
  low_res = web_gui_res.quality_preset == "low";

  mid_coh = web_gui_res.style_preset == "mid_coh";
  high_coh = web_gui_res.style_preset == "high_coh";
  chaotic = web_gui_res.style_preset == "chaotic";
  fragile = web_gui_res.style_preset == "fragile";
  clustered = web_gui_res.style_preset == "clustered";

  if (!web_gui_res.text.empty()) {
    result.overlay_text = web_gui_res.text;
//    result.direction_influencing_image_scale = 0.05f;
//    result.direction_influencing_image_path = "images/336AA021.jpeg";
    *params.dir_image_mix = 0.0f;
  }
  if (web_gui_res.set_dir_influence_image_enabled) {
    if (web_gui_res.set_dir_influence_image_enabled.value()) {
      *params.dir_image_mix = 0.0f;
    } else {
      *params.dir_image_mix = 1.0f;
      result.direction_influencing_image_scale = 0.0f;
    }
  }
  if (web_gui_res.direction_influence_scale) {
    result.direction_influencing_image_scale = web_gui_res.direction_influence_scale.value();
  }
  if (web_gui_res.time_scale) {
    result.time_scale = lerp(web_gui_res.time_scale.value(), min_time_scale, max_time_scale);
  }
  if (web_gui_res.direction_influence_render_mix) {
    *params.dir_image_mix = web_gui_res.direction_influence_render_mix.value();
  }
  if (web_gui_res.set_debug_gui_enabled) {
    debug_gui_enabled = web_gui_res.set_debug_gui_enabled.value();
  }
  if (!web_gui_res.direction_influence_image_path.empty()) {
    result.direction_influencing_image_path = web_gui_res.direction_influence_image_path;
  }
  if (web_gui_res.render_bw) {
    *params.use_bw = web_gui_res.render_bw.value();
  }
  if (web_gui_res.enabled) {
    result.enabled = web_gui_res.enabled.value();
  }
#endif

  if (debug_gui_enabled) {
    ImGui::Begin("GUI");
    ImGui::Text("Cursor: %0.2f, %0.2f", params.cursor_x, params.cursor_y);

    bool enabled = component.params.enabled;
    if (ImGui::Checkbox("Enabled", &enabled)) {
      result.enabled = enabled;
    }

    if (ImGui::Button("Reinitialize")) {
      result.reinitialize = true;
    }

    high_res = high_res | ImGui::Button("HighRes");
    ImGui::SameLine();
    med_res = med_res | ImGui::Button("MedRes");
    ImGui::SameLine();
    low_res = low_res | ImGui::Button("LowRes");

    if (ImGui::TreeNode("Presets")) {
      mid_coh = mid_coh | ImGui::SmallButton("MidCoherence");
      high_coh = high_coh | ImGui::SmallButton("HighCoherence");
      chaotic = chaotic | ImGui::SmallButton("Chaotic");
      fragile = fragile | ImGui::SmallButton("Fragile");
      clustered = clustered | ImGui::SmallButton("Clustered");
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("NumParticles")) {
      static int item{};
      static constexpr int num_items = 7;
      const char* const num_particles_str[num_items]{"1000", "2000", "4000", "8000", "16000", "25000", "32000"};
      const int num_particles[num_items]{1000, 2000, 4000, 8000, 16000, 25000, 32000};
      for (int i = 0; i < num_items; i++) {
        if (num_particles[i] == component.get_current_num_particles()) {
          item = i;
          break;
        }
      }
      if (ImGui::ListBox("", &item, num_particles_str, num_items)) {
        result.new_num_particles = num_particles[item];
      }
      ImGui::TreePop();
    }

#if DYNAMIC_TEXTURE_SIZE
    if (ImGui::TreeNode("ImageSize")) {
      static int item{};
      static constexpr int num_items = 3;
      const char* const items_str[num_items]{"256x256", "512x512", "1024x1024"};
      const int items[num_items]{256, 512, 1024};
      for (int i = 0; i < num_items; i++) {
        if (items[i] == component.get_texture_dim()) {
          item = i;
          break;
        }
      }
      if (ImGui::ListBox("", &item, items_str, num_items)) {
        result.new_texture_size = items[item];
      }
      ImGui::TreePop();
    }
#endif

    const auto& soil_config = component.sim.config;

    if (ImGui::TreeNode("Time")) {
      static int item{};
      static constexpr int num_items = 5;
      const char* const items_str[num_items]{"Default", "Fast", "Faster", "Slow", "Slower"};
      const float items[num_items]{1.0f, 4.0f, 8.0f, 0.5f, 0.01f};
      if (ImGui::ListBox("", &item, items_str, num_items)) {
        result.time_scale = items[item];
      }
      auto ts = soil_config.time_scale;
      if (ImGui::SliderFloat("TimeScale", &ts, min_time_scale, max_time_scale)) {
        result.time_scale = ts;
      }
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Speed")) {
      ImGui::Text("TurnSpeedPower %d", soil_config.turn_speed_power);
      if (ImGui::SmallButton("ScaleTurnSpeed2")) {
        result.turn_speed_power = soil_config.turn_speed_power + 1;
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("ScaleTurnSpeed0.5")) {
        result.turn_speed_power = soil_config.turn_speed_power - 1;
      }

      ImGui::Text("Speed Power %d", soil_config.scale_speed_power);
      if (ImGui::SmallButton("ScaleSpeed2")) {
        result.speed_power = soil_config.scale_speed_power + 1;
      }
      ImGui::SameLine();
      if (ImGui::SmallButton("ScaleSpeed0.5")) {
        result.speed_power = soil_config.scale_speed_power - 1;
      }
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Diffusion")) {
      auto decay = soil_config.decay;
      if (ImGui::SliderFloat("Decay", &decay, 0.001f, 0.5f)) {
        result.decay = decay;
      }

      auto ds = soil_config.diffuse_speed;
      if (ImGui::SliderFloat("DiffuseSpeed", &ds, 0.01f, 1.0f)) {
        result.diffuse_speed = ds;
      }

      if (ImGui::Button("ResetDiffusion")) {
        result.reset_diffuse_parameters = true;
      }

      bool diff_enabled = soil_config.diffuse_enabled;
      if (ImGui::Checkbox("DiffuseEnabled", &diff_enabled)) {
        result.diffuse_enabled = diff_enabled;
      }

      ImGui::TreePop();
    }

    bool allow_perturb = soil_config.allow_perturb_event;
    if (ImGui::Checkbox("AllowPerturbEvent", &allow_perturb)) {
      result.allow_perturb_event = allow_perturb;
    }

    bool circ_world = soil_config.circular_world;
    if (ImGui::Checkbox("CircularWorld", &circ_world)) {
      result.circular_world = circ_world;
    }

    bool only_right_turns = soil_config.only_right_turns;
    if (ImGui::Checkbox("OnlyRightTurns", &only_right_turns)) {
      result.only_right_turns = only_right_turns;
    }

    bool avg_img = soil_config.average_image;
    if (ImGui::Checkbox("AverageImage", &avg_img)) {
      result.average_image = avg_img;
    }

    if (ImGui::TreeNode("DirectionInfluence")) {
      if (ImGui::Button("Ex. 1")) {
        result.overlay_text = "Warping, or warped; tugging bits of self by lines, anchors set down shallow.";
        result.direction_influencing_image_scale = 0.05f;
        result.direction_influencing_image_path = "images/336AA021.jpeg";
        *params.dir_image_mix = 0.0f;
      }
      if (ImGui::Button("Ex. 2")) {
        result.overlay_text = "Approaching the sea, the self curves.";
        result.direction_influencing_image_scale = 0.05f;
        result.direction_influencing_image_path = "images/336AA030.jpeg";
        *params.dir_image_mix = 0.0f;
      }
#ifndef SM_IS_EMSCRIPTEN
      if (ImGui::Button("LoadImage")) {
        //    result.direction_influencing_image_path = "/Users/nick/Downloads/edge_im.png";
  //      result.direction_influencing_image_path = "/Users/nick/Downloads/00003652_0002_15 copy.jpeg";
        result.direction_influencing_image_path = "/Volumes/external3/dunes/337AA033.jpeg";
        //    result.direction_influencing_image_path = "/Users/nick/Downloads/00003653_0014_2.jpeg";
        result.overlay_text = "Some example text setsdfsdfsdfs sdlfksjdflkj sdfs";
      }
      {
        char text[2048];
        const auto f = ImGuiInputTextFlags_EnterReturnsTrue;
        if (ImGui::InputText("ImageFilePath", text, 2048, f)) {
          result.direction_influencing_image_path = text;
        }
      }
#endif
      {
        char text[2048];
        std::fill(text, text + 2048, 0);
        const auto f = ImGuiInputTextFlags_EnterReturnsTrue;
        if (ImGui::InputText("OverlayText", text, 2048, f)) {
          result.overlay_text = text;
        }
      }

      float s = soil_config.direction_influencing_image_scale;
      if (ImGui::SliderFloat("InfluenceScale", &s, 0.0f, 1.0f)) {
        result.direction_influencing_image_scale = s;
      }
      {
        int edge_thresh = component.params.edge_detection_threshold;
        if (ImGui::InputInt("EdgeThreshold", &edge_thresh)) {
          component.params.edge_detection_threshold = edge_thresh;
        }
      }
      ImGui::SliderFloat("RenderMix", params.dir_image_mix, 0.0f, 1.0f);
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Render")) {
      ImGui::Checkbox("RenderB&W", use_bw);
      ImGui::Checkbox("RenderFullScreen", full_screen);
      ImGui::TreePop();
    }

    ImGui::Text("%.3f ms/frame (%.1f FPS)", 1e3f / fps, fps);
    ImGui::Text("%.3f ms/sim step", last_sim_t);
    ImGui::End();
  } //  debug_gui_enabled;

  if (high_res) {
    result.new_texture_size = 1024;
    result.new_num_particles = 25000;
  }
  if (med_res) {
    result.new_texture_size = 512;
    result.new_num_particles = 8000;
  }
  if (low_res) {
    result.new_texture_size = 256;
    result.new_num_particles = 1000;
  }

  if (mid_coh) {
    result.turn_speed_power = 2;
    result.speed_power = 2 - int(is_high_res);
    result.only_right_turns = false;
  }

  if (high_coh) {
    result.turn_speed_power = 3 + int(is_high_res);
    result.speed_power = 2 - int(is_high_res);
    result.only_right_turns = false;
  }

  if (chaotic) {
    result.turn_speed_power = 0;
    result.speed_power = 2 - int(is_high_res);
    result.only_right_turns = true;
  }

  if (fragile) {
    result.turn_speed_power = 2;
    result.speed_power = 1 - int(is_high_res);
    result.only_right_turns = false;
  }

  if (clustered) {
    result.turn_speed_power = 4;
    result.speed_power = is_high_res ? 0 : 2;
    result.only_right_turns = true;
  }

  return result;
}