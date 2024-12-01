#pragma once

#include <optional>
#include <string>

class SlimeMoldComponent;

struct GUIUpdateResult {
  std::optional<bool> enabled;
  std::optional<bool> parameter_capture_enabled;
  std::optional<bool> lock_parameter_targets;
  std::optional<bool> draw_texture;
  std::optional<bool> overlay_player_position;
  std::optional<float> overlay_radius;
  std::optional<float> decay;
  std::optional<float> diffuse_speed;
  std::optional<bool> diffuse_enabled;
  std::optional<bool> average_image;
  std::optional<bool> allow_perturb_event;
  std::optional<float> time_scale;
  std::optional<bool> circular_world;
  std::optional<bool> only_right_turns;
  std::optional<int> turn_speed_power;
  std::optional<int> speed_power;
  std::optional<bool> reinitialize;
  std::optional<bool> reset_diffuse_parameters;
  std::optional<int> new_num_particles;
  std::optional<int> new_texture_size;
  std::optional<std::string> direction_influencing_image_path;
  std::optional<float> direction_influencing_image_scale;
  bool close{};
};

struct GUIParams {
  float app_fps;
  float sim_t;
  bool* use_bw;
  bool* full_screen;
  float* dir_image_mix;
  float cursor_x;
  float cursor_y;
};

GUIUpdateResult render_gui(SlimeMoldComponent& component, const GUIParams& params);