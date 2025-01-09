#pragma once

#include "slime_mold.hpp"
#include <string>

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
    int edge_detection_threshold{13};
    std::string overlay_text;
    std::string direction_influencing_image_path;
  };

  struct Sim {
    gen::SlimeMoldParams params;
    gen::SlimeMoldConfig config;
    gen::SlimeMoldSimulationContext sim_context{};
    gen::DefaultSlimeMoldSimulationTextureData texture_data;
    std::unique_ptr<gen::SlimeParticle[]> particles;
    gen::DirectionInfluencingImage direction_influencing_image{};
    std::unique_ptr<uint8_t[]> direction_influencing_src_image;
    bool initialized{};
  };

public:
  void reinitialize();
  float update();
  void on_gui_update(const GUIUpdateResult& res);
  int get_texture_dim() const;
  int get_current_num_particles() const;
  const uint8_t* read_rgbau8_image_data() const;
  const uint8_t* read_r_dir_image_data(int* dim) const;

public:
  Params params;
  Sim sim;
};