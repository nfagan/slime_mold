#pragma once

#include <cstdint>
#include <memory>

namespace im {

//  vec_im has 1 component (theta) giving the angle of the vector pointing to the nearest edge for
//  each pixel in edge_im, a binary image (1 component; true = any value > 0)
void compute_directions_to_edges(
  const uint8_t* edge_im, int edge_im_w, int edge_im_h, float* vec_im);

bool read_image(
  const char* file_path, bool flip_y_on_load,
  std::unique_ptr<uint8_t[]>& dst, int* width, int* height, int* num_components);

void write_image(
  const char* file_path, uint8_t* dst, int width, int height, int num_components);

void resize_image(
  const uint8_t* src, int sw, int sh, int nc, uint8_t* dst, int dw, int dh);

void edge_detect(const uint8_t* src, int sw, int sh, uint8_t* dst, int t);

}