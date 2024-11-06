#pragma once

#include <optional>

class SoilComponent;

struct SoilGUIUpdateResult {
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
  bool close{};
};

struct SoilGUIParams {
  float app_fps;
  float sim_t;
  bool* use_bw;
  bool* full_screen;
  float cursor_x;
  float cursor_y;
};

class SoilGUI {
public:
  static SoilGUIUpdateResult render(const SoilComponent& component, const SoilGUIParams& params);
};