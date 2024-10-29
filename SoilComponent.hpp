#pragma once

#include "Soil.hpp"

struct SoilGUIUpdateResult;

class SoilComponent {
  friend class SoilGUI;

public:
  struct Params {
    bool enabled{true};
    bool initialized{};
    bool need_reinitialize{};
    bool draw_debug_image{};
    bool overlay_player_position{true};
    float overlay_radius{8.0f};
    int desired_num_particles{};
  };

public:
  void initialize();
  void reinitialize();
  float update();
  void on_gui_update(const SoilGUIUpdateResult& res);
  const Soil* get_soil() const {
    return &soil;
  }
  Soil* get_soil() {
    return &soil;
  }

private:
  Soil soil;
  Params params;
};