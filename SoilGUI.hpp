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
  std::optional<bool> allow_perturb_event;
  std::optional<float> time_scale;
  std::optional<bool> circular_world;
  std::optional<bool> only_right_turns;
  std::optional<int> turn_speed_power;
  std::optional<int> speed_power;
  bool close{};
};

class SoilGUI {
public:
  static SoilGUIUpdateResult render(const SoilComponent& component, float fps);
};