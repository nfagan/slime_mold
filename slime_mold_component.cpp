#include "slime_mold_component.hpp"
#include "slime_mold.hpp"
#include "gui.hpp"
#include "image_manip.hpp"
#include "text_rasterizer.hpp"

namespace {

void set_sim_context_ptrs(
  gen::SlimeMoldSimulationContext& context,
  gen::DefaultSlimeMoldSimulationTextureData& tex_data,
  const gen::SlimeMoldParams* params,
  const gen::DirectionInfluencingImage* dir_im) {
  //
  context.texture_data0 = tex_data.texture_data0.get();
  context.texture_data1 = tex_data.texture_data1.get();
  context.texture_data2 = tex_data.texture_data2.get();
  context.signal_data = tex_data.signal_data.get();
  context.perturb_data = tex_data.perturb_data.get();
  context.rgbau8_texture_data0 = tex_data.rgbau8_texture_data.get();
  context.params = params;
  context.direction_influencing_image = dir_im;
}

void init_sim(SlimeMoldComponent& component) {
  auto* impl = &component.sim;
  impl->texture_data = gen::make_default_slime_mold_texture_data();
  impl->particles = gen::make_slime_mold_particles(impl->config);
  set_sim_context_ptrs(
    impl->sim_context, impl->texture_data,
    &impl->params, &impl->direction_influencing_image);
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

std::unique_ptr<uint8_t[]> load_src_image(const std::string& im_p, int rd) {
  std::unique_ptr<uint8_t[]> im_data_src;
  int sw;
  int sh;
  int sc;
  if (!im::read_image(im_p.c_str(), true, im_data_src, &sw, &sh, &sc)) {
    return nullptr;
  }

  auto im_data_resize = std::make_unique<uint8_t[]>(rd * rd * sc);
  im::resize_image(im_data_src.get(), sw, sh, sc, im_data_resize.get(), rd, rd);

  auto im_gray = std::make_unique<uint8_t[]>(rd * rd);
  const int nc = std::min(sc, 3);
  for (int i = 0; i < rd; i++) {
    for (int j = 0; j < rd; j++) {
      int s{};
      for (int ci = 0; ci < nc; ci++) {
        const int ii = (i * rd + j) * sc + ci; //  note, use sc here in case it is > 3
        s += im_data_resize[ii];
      }
      float mu = float(s) / float(nc);
      uint8_t r = uint8_t(std::min(std::max(int(mu), 0), 255));
      im_gray[i * rd + j] = r;
    }
  }

  return im_gray;
}

void try_update_direction_influencing_image(
  SlimeMoldComponent& comp, const std::string& im_p, const std::string& text) {
  //
  const int rd = 512;
  auto im_gray = load_src_image(im_p, rd);
  if (!im_gray) {
    return;
  }

#if 1
  {
    const int im_dim = rd;
    auto raster_data = std::make_unique<uint8_t[]>(im_dim * im_dim);
    font::TextRasterizerParams rp{};
    rp.image_width = im_dim;
    rp.image_height = im_dim;
    rp.text_x0 = 10.0f;
    rp.text_x1 = 500.0f;
    rp.text_y0 = 30.0f;
    rp.text_y1 = 80.0f;
    rp.font_size = 48.0f;
    rp.text = text.c_str();
    if (font::rasterize_text(raster_data.get(), rp)) {
      for (int i = 0; i < im_dim * im_dim; i++) {
        const auto ig = float(im_gray[i]);
        const auto tg = float(raster_data[i]);
        im_gray[i] = uint8_t(lerp(0.5f, ig, tg));
      }
    }
  }
#endif

#if 1
  auto im_edge = std::make_unique<uint8_t[]>(rd * rd);
  im::edge_detect(im_gray.get(), rd, rd, im_edge.get(), comp.params.edge_detection_threshold);
  auto& im_read = im_edge;
#else
  auto& im_read = im_gray;
#endif

  auto dir_im_f = std::make_unique<float[]>(rd * rd);
  im::compute_directions_to_edges(im_read.get(), rd, rd, dir_im_f.get());

#if 0
  {
    auto show_res = std::make_unique<uint8_t[]>(rd * rd);
    std::fill(show_res.get(), show_res.get() + rd * rd, 0);
    const float minv = *std::min_element(dir_im_f.get(), dir_im_f.get() + rd * rd);
    const float maxv = *std::max_element(dir_im_f.get(), dir_im_f.get() + rd * rd);
    const float d = minv == maxv ? 1.0f : maxv - minv;
    for (int i = 0; i < rd * rd; i++) {
      show_res[i] = uint8_t(255.0f * (dir_im_f[i] - minv) / d);
    }
    im::write_image("/Users/nick/Downloads/edge_im2.png", show_res.get(), rd, rd, 1);
    im::write_image("/Users/nick/Downloads/edge_im3.png", im_read.get(), rd, rd, 1);
  }
#endif

  auto& dir_im = comp.sim.direction_influencing_image;
  dir_im.theta = std::move(dir_im_f);
  dir_im.w = rd;
  dir_im.h = rd;
  comp.sim.direction_influencing_src_image = std::move(im_gray);
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

const uint8_t* SlimeMoldComponent::read_r_dir_image_data(int* dim) const {
  *dim = sim.direction_influencing_image.w;
  return sim.direction_influencing_src_image.get();
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

  bool need_update_dir_image{};
  if (res.overlay_text) {
    params.overlay_text = res.overlay_text.value();
    need_update_dir_image = true;
  }
  if (res.direction_influencing_image_path) {
    params.direction_influencing_image_path = res.direction_influencing_image_path.value();
    need_update_dir_image = true;
  }
  if (need_update_dir_image) {
    try_update_direction_influencing_image(
      *this, params.direction_influencing_image_path, params.overlay_text);
  }
  if (res.direction_influencing_image_scale) {
    config->direction_influencing_image_scale = res.direction_influencing_image_scale.value();
  }
}
