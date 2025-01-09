#pragma once

#include <memory>

namespace font {

struct TextRasterizerParams {
  int image_width;
  int image_height;
  float text_x0;
  float text_x1;
  float text_y0;
  float text_y1;
  float font_size;
  const char* text;
};

void initialize_text_rasterizer();
void terminate_text_rasterizer();
bool rasterize_text(uint8_t* dst, const TextRasterizerParams& params);

}