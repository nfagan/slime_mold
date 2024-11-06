#include "Soil.hpp"
#include "slime_mold.hpp"

struct SoilImpl {
  gen::SlimeMoldParams params;
  gen::SlimeMoldConfig config;
  gen::SlimeMoldSimulationContext sim_context{};
  gen::DefaultSlimeMoldSimulationTextureData texture_data;
  std::unique_ptr<gen::SlimeParticle[]> particles;
  bool initialized{};
};

namespace {

void set_sim_context_ptrs(
  gen::SlimeMoldSimulationContext& context,
  gen::DefaultSlimeMoldSimulationTextureData& tex_data,
  const gen::SlimeMoldParams* params) {
  //
  context.texture_data0 = tex_data.texture_data0.get();
  context.texture_data1 = tex_data.texture_data1.get();
  context.texture_data2 = tex_data.texture_data2.get();
  context.signal_data = tex_data.signal_data.get();
  context.perturb_data = tex_data.perturb_data.get();
  context.rgbau8_texture_data0 = tex_data.rgbau8_texture_data.get();
  context.params = params;
}

} //  anon

Soil::Soil() : impl{new SoilImpl()} {
  //
}

Soil::~Soil() {
  delete impl;
}

void Soil::initialize() {
  impl->texture_data = gen::make_default_slime_mold_texture_data();
  impl->particles = gen::make_slime_mold_particles(impl->config);
  set_sim_context_ptrs(impl->sim_context, impl->texture_data, &impl->params);
  impl->initialized = true;
}

float Soil::update() {
  float res{};
  if (impl->initialized) {
    res = gen::update_slime_mold_particles(
      impl->particles.get(),
      impl->config,
      &impl->sim_context);
  }
  return res;
}

void Soil::set_particle_turn_speed_power(int pow) {
  if (impl->initialized) {
    gen::set_particle_turn_speed_power(impl->particles.get(), impl->config, pow);
  }
}

void Soil::set_particle_speed_power(int pow) {
  if (impl->initialized) {
    gen::set_particle_speed_power(impl->particles.get(), impl->config, pow);
  }
}

void Soil::set_particle_use_only_right_turns(bool v) {
  if (impl->initialized) {
    gen::set_particle_right_only(impl->particles.get(), impl->config, v);
  }
}

const uint8_t* Soil::read_rgbau8_image_data() const {
  return impl->texture_data.rgbau8_texture_data.get();
}

const gen::SlimeMoldConfig* Soil::read_config() const {
  return &impl->config;
}

gen::SlimeMoldConfig* Soil::get_config() {
  return &impl->config;
}