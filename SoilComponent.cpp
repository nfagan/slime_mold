#include "SoilComponent.hpp"
#include "slime_mold.hpp"
#include "SoilGUI.hpp"

void SoilComponent::reinitialize() {
  params.need_reinitialize = true;
}

int SoilComponent::get_texture_dim() const {
  return gen::SlimeMoldConfig::texture_dim;
}

float SoilComponent::update() {
  if (params.initialized && params.need_reinitialize) {
#if DYNAMIC_TEXTURE_SIZE
    if (params.desired_texture_size > 0) {
      gen::SlimeMoldConfig::texture_dim = params.desired_texture_size;
    }
#endif
    const int curr_num_particles = soil.get_config()->num_particles;
    soil.get_config()->num_particles = params.desired_num_particles > 0 ?
      params.desired_num_particles : curr_num_particles;
    soil.initialize();
    params.need_reinitialize = false;
  }

  float res{};
  if (params.enabled) {
    if (!params.initialized) {
      soil.initialize();
      params.initialized = true;
    }
    res = soil.update();
  }
  return res;
}

void SoilComponent::on_gui_update(const SoilGUIUpdateResult& res) {
  if (res.enabled) {
    params.enabled = res.enabled.value();
  }
  if (res.draw_texture) {
    params.draw_debug_image = res.draw_texture.value();
  }
  if (res.overlay_player_position) {
    params.overlay_player_position = res.overlay_player_position.value();
  }
  if (res.overlay_radius) {
    params.overlay_radius = res.overlay_radius.value();
  }
  auto* config = soil.get_config();
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
    soil.set_particle_turn_speed_power(res.turn_speed_power.value());
  }
  if (res.speed_power) {
    soil.set_particle_speed_power(res.speed_power.value());
  }
  if (res.only_right_turns) {
    soil.set_particle_use_only_right_turns(res.only_right_turns.value());
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
