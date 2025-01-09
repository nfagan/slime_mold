#pragma once

#include <optional>

namespace font {

struct FontBitmapSampleInfo;

struct FontHandle {
  bool is_valid() const { return id > 0; }
  uint32_t id;
};

struct ReadFontImages {
  const unsigned char* images[16];
  int num_images;
  int image_dim;
};

void initialize_fonts();
void terminate_fonts();
std::optional<ReadFontImages> read_font_images();
std::optional<FontHandle> get_text_font();

[[nodiscard]] int ascii_left_justified(
  FontHandle font, const char* text, float font_size, float max_width, FontBitmapSampleInfo* dst,
  float* xoff = nullptr, float* yoff = nullptr);

float get_glyph_sequence_width_ascii(FontHandle font, const char* text, float font_size);

}