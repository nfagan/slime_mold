#include "gui.hpp"
#include "slime_mold_component.hpp"
#include "slime_mold.hpp"
#include <imgui.h>

GUIUpdateResult render_gui(SlimeMoldComponent& component, const GUIParams& params) {
  float fps = params.app_fps;
  float sim_t = params.sim_t;
  bool* use_bw = params.use_bw;
  bool* full_screen = params.full_screen;

  static uint32_t update_index{};
  static float last_sim_t{};

  if ((++update_index % 30) == 0) {
    last_sim_t = sim_t;
  }

  GUIUpdateResult result;

  ImGui::Begin("GUI");

  ImGui::Text("Cursor: %0.2f, %0.2f", params.cursor_x, params.cursor_y);

  bool enabled = component.params.enabled;
  if (ImGui::Checkbox("Enabled", &enabled)) {
    result.enabled = enabled;
  }

  if (ImGui::Button("Reinitialize")) {
    result.reinitialize = true;
  }

  if (ImGui::Button("ReinitializeHighRes")) {
    result.new_texture_size = 1024;
    result.new_num_particles = 25000;
  }

  ImGui::SameLine();
  if (ImGui::Button("ReinitializeLowRes")) {
    result.new_texture_size = 256;
    result.new_num_particles = 1000;
  }

  if (ImGui::TreeNode("Presets")) {
    const bool is_high_res = gen::SlimeMoldConfig::texture_dim > 512;
    if (ImGui::SmallButton("MidCoherence")) {
      result.turn_speed_power = 2;
      result.speed_power = 2 - int(is_high_res);
      result.only_right_turns = false;
    }
    if (ImGui::SmallButton("HighCoherence")) {
      result.turn_speed_power = 3 + int(is_high_res);
      result.speed_power = 2 - int(is_high_res);
      result.only_right_turns = false;
    }
    if (ImGui::SmallButton("Chaotic")) {
      result.turn_speed_power = 0;
      result.speed_power = 2 - int(is_high_res);
      result.only_right_turns = true;
    }
    if (ImGui::SmallButton("Fragile")) {
      result.turn_speed_power = 2;
      result.speed_power = 1 - int(is_high_res);
      result.only_right_turns = false;
    }
    if (ImGui::SmallButton("Clustered")) {
      result.turn_speed_power = 4;
      result.speed_power = is_high_res ? 0 : 2;
      result.only_right_turns = true;
    }
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
    if (ImGui::SliderFloat("TimeScale", &ts, 0.01f, 8.0f)) {
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
    {
      char text[2048];
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
  return result;
}