#include "text_rasterizer.hpp"
#include "font.hpp"
#include "font_env.hpp"
#include "util.hpp"

namespace {

bool cpu_rasterize(
  uint8_t* dst, int w, int h, const font::FontBitmapSampleInfo* infos, int num_infos,
  const font::ReadFontImages& font_images, int font_image_index, bool flip_y) {
  //

  if (font_image_index >= font_images.num_images) {
    return false;
  }

  const unsigned char* font_image_data = font_images.images[font_image_index];

  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      int res{};
      for (int k = 0; k < num_infos; k++) {
        const font::FontBitmapSampleInfo& info = infos[k];

        const float u = float(j) + 0.5f;
        const float v = float(i) + 0.5f;

        const bool ib = u >= info.x0 && u < info.x1 && v >= info.y0 && v < info.y1;
        if (!ib) {
          continue;
        }

        const float tx = (u - info.x0) / (info.x1 - info.x0);
        const float ty = (v - info.y0) / (info.y1 - info.y0);
        const float fx = (info.u1 - info.u0) * tx + info.u0;
        const float fy = (info.v1 - info.v0) * ty + info.v0;
        const int fi = int(fy * float(font_images.image_dim));
        const int fj = int(fx * float(font_images.image_dim));

        if (fi >= 0 && fi < font_images.image_dim && fj >= 0 && fj < font_images.image_dim) {
          res += font_image_data[fi * font_images.image_dim + fj];  //  1 component
        }
      }

      const int di = flip_y ? (h - i - 1) : i;
      dst[di * w + j] = uint8_t(std::min(255, res));
    }
  }

  return true;
}

} //  anon

void font::initialize_text_rasterizer() {
  font::initialize_fonts();
}

void font::terminate_text_rasterizer() {
  font::terminate_fonts();
}

bool font::rasterize_text(uint8_t* dst, const TextRasterizerParams& params) {
  auto font_handle = font::get_text_font();
  if (!font_handle) {
    return false;
  }

  auto font_images = font::read_font_images();
  if (!font_images) {
    return false;
  }

  //  @NOTE: 1 based index.
  const int font_index = int(font_handle.value().id) - 1;

  Temporary<font::FontBitmapSampleInfo, 256> store_sample_infos;
  auto* sample_infos = store_sample_infos.require(int(std::strlen(params.text)));

  const float w = std::max(1e-3f, params.text_x1 - params.text_x0);
  float x{};
  float y{};
  const float font_size = params.font_size;
  const bool flip_y = true; //  @TODO

  const int num_gen = font::ascii_left_justified(
    font_handle.value(), params.text, font_size, w, sample_infos, &x, &y);
  font::offset_bitmap_samples(sample_infos, num_gen, params.text_x0, params.text_y0);

  return cpu_rasterize(
    dst, params.image_width, params.image_height,
    sample_infos, num_gen, font_images.value(), font_index, flip_y);
}
