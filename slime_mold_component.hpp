#pragma once

#include "slime_mold.hpp"

struct GUIUpdateResult;

struct SlimeMoldComponent {
  friend class SoilGUI;

public:
  struct Params {
    bool enabled{true};
    bool initialized{};
    bool need_reinitialize{};
    int desired_num_particles{};
    int desired_texture_size{};
  };

  struct Sim {
    gen::SlimeMoldParams params;
    gen::SlimeMoldConfig config;
    gen::SlimeMoldSimulationContext sim_context{};
    gen::DefaultSlimeMoldSimulationTextureData texture_data;
    std::unique_ptr<gen::SlimeParticle[]> particles;
    bool initialized{};
  };

public:
  void reinitialize();
  float update();
  void on_gui_update(const GUIUpdateResult& res);
  int get_texture_dim() const;
  int get_current_num_particles() const;
  const uint8_t* read_rgbau8_image_data() const;

public:
  Params params;
  Sim sim;
};