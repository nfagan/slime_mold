#pragma once

#include "base_math.hpp"

struct SoilImpl;

namespace gen {
struct SlimeMoldConfig;
}

class Soil {
public:
  Soil();
  ~Soil();

  void initialize();
  float update();

  void set_particle_turn_speed_power(int pow);
  void set_particle_speed_power(int pow);
  void set_particle_use_only_right_turns(bool v);
  const uint8_t* read_rgbau8_image_data() const;
  const gen::SlimeMoldConfig* read_config() const;
  gen::SlimeMoldConfig* get_config();

private:
  SoilImpl* impl{};
};