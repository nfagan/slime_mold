#include "slime_mold_component.hpp"
#include "slime_mold.hpp"
#include "gui.hpp"

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

void init_sim(SlimeMoldComponent& component) {
  auto* impl = &component.sim;
  impl->texture_data = gen::make_default_slime_mold_texture_data();
  impl->particles = gen::make_slime_mold_particles(impl->config);
  set_sim_context_ptrs(impl->sim_context, impl->texture_data, &impl->params);
  impl->initialized = true;
}

float update_sim(SlimeMoldComponent& component) {
  float res{};
  auto& sim = component.sim;
  if (sim.initialized) {
    res = gen::update_slime_mold_particles(
      sim.particles.get(),
      sim.config,
      &sim.sim_context);
  }
  return res;
}

void set_particle_turn_speed_power(SlimeMoldComponent& comp, int pow) {
  if (comp.sim.initialized) {
    gen::set_particle_turn_speed_power(comp.sim.particles.get(), comp.sim.config, pow);
  }
}

void set_particle_speed_power(SlimeMoldComponent& comp, int pow) {
  if (comp.sim.initialized) {
    gen::set_particle_speed_power(comp.sim.particles.get(), comp.sim.config, pow);
  }
}

void set_particle_use_only_right_turns(SlimeMoldComponent& comp, bool v) {
  if (comp.sim.initialized) {
    gen::set_particle_right_only(comp.sim.particles.get(), comp.sim.config, v);
  }
}

} //  anon

void SlimeMoldComponent::reinitialize() {
  params.need_reinitialize = true;
}

int SlimeMoldComponent::get_texture_dim() const {
  return gen::SlimeMoldConfig::texture_dim;
}

int SlimeMoldComponent::get_current_num_particles() const {
  return sim.config.num_particles;
}

const uint8_t* SlimeMoldComponent::read_rgbau8_image_data() const {
  return sim.texture_data.rgbau8_texture_data.get();
}

float SlimeMoldComponent::update() {
  if (params.initialized && params.need_reinitialize) {
#if DYNAMIC_TEXTURE_SIZE
    if (params.desired_texture_size > 0) {
      gen::SlimeMoldConfig::texture_dim = params.desired_texture_size;
    }
#endif
    const int curr_num_particles = sim.config.num_particles;
    sim.config.num_particles = params.desired_num_particles > 0 ?
      params.desired_num_particles : curr_num_particles;
    init_sim(*this);
    params.need_reinitialize = false;
  }

  float res{};
  if (params.enabled) {
    if (!params.initialized) {
      init_sim(*this);
      params.initialized = true;
    }
    res = update_sim(*this);
  }
  return res;
}

void SlimeMoldComponent::on_gui_update(const GUIUpdateResult& res) {
  if (res.enabled) {
    params.enabled = res.enabled.value();
  }
  auto* config = &sim.config;
  if (res.circular_world) {
    config->circular_world = res.circular_world.value();
  }
  if (res.decay) {
    config->decay = res.decay.value();
  }
  if (res.diffuse_speed) {
    config->diffuse_speed = res.diffuse_speed.value();
  }
  if (res.diffuse_enabled) {
    config->diffuse_enabled = res.diffuse_enabled.value();
  }
  if (res.allow_perturb_event) {
    config->allow_perturb_event = res.allow_perturb_event.value();
  }
  if (res.time_scale) {
    config->time_scale = res.time_scale.value();
  }
  if (res.only_right_turns) {
    config->only_right_turns = res.only_right_turns.value();
  }
  if (res.turn_speed_power) {
    set_particle_turn_speed_power(*this, res.turn_speed_power.value());
  }
  if (res.speed_power) {
    set_particle_speed_power(*this, res.speed_power.value());
  }
  if (res.only_right_turns) {
    set_particle_use_only_right_turns(*this, res.only_right_turns.value());
  }
  if (res.average_image) {
    config->average_image = res.average_image.value();
  }
  if (res.reset_diffuse_parameters) {
    config->reset_diffuse_parameters();
  }
  if (res.new_num_particles) {
    params.desired_num_particles = res.new_num_particles.value();
    params.need_reinitialize = true;
  }
  if (res.new_texture_size) {
    params.desired_texture_size = res.new_texture_size.value();
    params.need_reinitialize = true;
  }
  if (res.reinitialize) {
    params.need_reinitialize = true;
  }
}
