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
    int desired_num_particles{};
    int desired_texture_size{};
  };

public:
  void reinitialize();
  float update();
  void on_gui_update(const SoilGUIUpdateResult& res);
  const Soil* get_soil() const {
    return &soil;
  }
  Soil* get_soil() {
    return &soil;
  }
  int get_texture_dim() const;
  int get_current_num_particles() const;

private:
  Soil soil;
  Params params;
};