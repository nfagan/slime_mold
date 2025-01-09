#include "image_manip.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <cmath>
#include <algorithm>
#include <optional>
#include <limits>

namespace {

template <typename T>
struct LoadFunction {
  //
};

template<>
struct LoadFunction<uint8_t> {
  static uint8_t* load(const char* file_path, int* width, int* height, int* num_components) {
    return stbi_load(file_path, width, height, num_components, 0);
  }
};

template <typename T>
bool load_image_impl(
  const char* file_path, bool flip_y_on_load,
  std::unique_ptr<T[]>& dst, int* width, int* height, int* num_components) {
  //
  if (flip_y_on_load) {
    stbi_set_flip_vertically_on_load(true);
  }

  T* data = LoadFunction<T>::load(file_path, width, height, num_components);

  if (flip_y_on_load) {
    stbi_set_flip_vertically_on_load(false);
  }

  if (!data) {
    return false;
  }

  const std::size_t data_size = *width * *height * *num_components;
  auto data_copy = std::make_unique<T[]>(data_size);
  std::memcpy(data_copy.get(), data, data_size * sizeof(T));
  stbi_image_free(data);

  dst = std::move(data_copy);
  return true;
}

struct V2i {
public:
  int x;
  int y;

public:
  float normf() const {
    float fi = float(y);
    float fj = float(x);
    return std::sqrt(fi * fi + fj * fj);
  }

  friend inline V2i operator-(const V2i& a, const V2i& b) {
    return {a.x - b.x, a.y - b.y};
  }
};

std::optional<float> nearest_edge_dir_beam(
  const uint8_t* edge_im, int w, int h, int si, int sj) {
  //
  int max_stp = std::max(w, h);

  const auto read_im = [edge_im, w](int i, int j) {
    return edge_im[i * w + j] > 0;
  };

  std::optional<V2i> gv;
  V2i s{si, sj};
  int stp = 0;
  while (stp < max_stp) {
    int off = 1 + stp;
    stp = stp + 1;

    // init indices
    int i0 = std::max(0, si - off);
    int i1 = std::min(h, si + off + 1);
    int j0 = std::max(0, sj - off);
    int j1 = std::min(w, sj + off + 1);

    // init iteration
    float dmin = std::numeric_limits<float>::infinity();
    std::optional<V2i> v;
    // top/bot strips
    for (int j = j0; j < j1; j++) {
      // top
      int ei;
      ei = si - off;
      if (ei >= 0 && read_im(ei, j)) {
        V2i vt = V2i{ei, j} - s;
        if (vt.normf() < dmin) {
          dmin = vt.normf();
          v = vt;
        }
      }
      //  bot
      ei = si + off;
      if (ei < h && read_im(ei, j)) {
        V2i vt = V2i{ei, j} - s;
        if (vt.normf() < dmin) {
          dmin = vt.normf();
          v = vt;
        }
      }
    }
    // l/r strips
    for (int i = i0; i < i1; i++) {
      //  left
      int ej;
      ej = sj - off;
      if (ej >= 0 && read_im(i, ej)) {
        V2i vt = V2i{i, ej} - s;
        if (vt.normf() < dmin) {
          dmin = vt.normf();
          v = vt;
        }
      }
      //  right
      ej = sj + off;
      if (ej < w && read_im(i, ej)) {
        V2i vt = V2i{i, ej} - s;
        if (vt.normf() < dmin) {
          dmin = vt.normf();
          v = vt;
        }
      }
    }

    if (v) {
      gv = v.value();
      break;
    }
  }

  if (gv) {
    const V2i vt = gv.value();
    return std::atan2(float(vt.y), float(vt.x));
  } else {
    return std::nullopt;
  }
}

} //  anon

void im::compute_directions_to_edges(
  const uint8_t* edge_im, int edge_im_w, int edge_im_h, float* vec_im) {
  //
  for (int i = 0; i < edge_im_h; i++) {
    for (int j = 0; j < edge_im_w; j++) {
      auto v = nearest_edge_dir_beam(edge_im, edge_im_w, edge_im_h, i, j);
      if (v) {
        const float vv = v.value();
        vec_im[i * edge_im_w + j] = vv;
      }
    }
  }
}

bool im::read_image(
  const char* file_path, bool flip_y_on_load, std::unique_ptr<uint8_t[]>& dst,
  int* width, int* height, int* num_components) {
  //
  return load_image_impl<uint8_t>(file_path, flip_y_on_load, dst, width, height, num_components);
}

void im::write_image(
  const char* file_path, uint8_t* dst, int width, int height, int num_components) {
  //
  stbi_write_png(file_path, width, height, num_components, dst, 0);
}

void im::resize_image(const uint8_t* src, int sw, int sh, int nc, uint8_t* dst, int dw, int dh) {
  stbir_resize_uint8(src, sw, sh, 0, dst, dw, dh, 0, nc);
}

void im::edge_detect(const uint8_t* src, int sw, int sh, uint8_t* dst, int thresh) {
  for (int i = 0; i < sh; i++) {
    for (int j = 0; j < sw; j++) {
      const int c = src[i * sw + j];
      const int i1 = (i + 1) < sh ? i + 1 : i;
      const int j1 = (j + 1) < sw ? j + 1 : j;
      const int dx = std::abs(src[i1 * sw + j] - c);
      const int dy = std::abs(src[i * sw + j1] - c);
      dst[i * sw + j] = 255 * (dx > thresh || dy > thresh);
    }
  }
}