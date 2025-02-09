#pragma once

#include <memory>
#include <array>
#include <optional>
#include <cassert>
#include <vector>

template <typename T>
struct TemporaryView {
  T* require(int count) {
    if (count > stack_size) {
      heap = std::make_unique<T[]>(count);
      return heap.get();
    } else {
      return stack;
    }
  }

  T* stack;
  std::unique_ptr<T[]>& heap;
  int stack_size;
};

template <typename T>
struct TemporaryViewStack {
  T* begin() {
    return size <= stack_capacity ? stack : heap.get();
  }
  T* end() {
    return size <= stack_capacity ? (stack + size) : (heap.get() + size);
  }

  T* push(int count) {
    if (size + count <= stack_capacity) {
      auto curr_size = size;
      size += count;
      return stack + curr_size;

    } else if (size + count <= heap_capacity) {
      auto curr_size = size;
      size += count;
      return heap.get() + curr_size;

    } else if (size <= stack_capacity) {
      //  resize, copy from stack to heap.
      heap_capacity = stack_capacity == 0 ? 2 : stack_capacity * 2;
      while (heap_capacity < size + count) {
        heap_capacity *= 2;
      }
      heap = std::make_unique<T[]>(heap_capacity);
      std::copy(stack, stack + size, heap.get());
      auto curr_size = size;
      size += count;
      return heap.get() + curr_size;

    } else {
      //  resize, copy from heap to heap.
      heap_capacity = heap_capacity == 0 ? 2 : heap_capacity * 2;
      while (heap_capacity < size + count) {
        heap_capacity *= 2;
      }
      auto dst = std::make_unique<T[]>(heap_capacity);
      std::copy(heap.get(), heap.get() + size, dst.get());
      heap = std::move(dst);
      auto curr_size = size;
      size += count;
      return heap.get() + curr_size;
    }
  }

  T* stack;
  std::unique_ptr<T[]>& heap;
  int size;
  int stack_capacity;
  int heap_capacity;
};

template <typename T, int StackSize>
struct Temporary {
  TemporaryViewStack<T> view_stack() {
    return TemporaryViewStack<T>{stack, heap, 0, StackSize, 0};
  }

  TemporaryView<T> view() {
    return TemporaryView<T>{stack, heap, StackSize};
  }

  T* require(int count) {
    if (count > StackSize) {
      heap = std::make_unique<T[]>(count);
      return heap.get();
    } else {
      return stack;
    }
  }

  T stack[StackSize];
  std::unique_ptr<T[]> heap;
};

template <typename T, int N>
class DynamicArray;

template <typename T>
class ArrayView {
public:
  ArrayView();
  ArrayView(T* begin, T* end);

  const T& operator[](int64_t off) const;
  T& operator[](int64_t off);

  int64_t size() const;
  bool empty() const {
    return size() == 0;
  }

  T* begin();
  T* end();

  const T* begin() const;
  const T* end() const;

  T* data() {
    return beg_;
  }
  const T* data() const {
    return beg_;
  }

private:
  T* beg_;
  T* end_;
};

template <typename T, typename U>
ArrayView<T> make_iterator_array_view(U&& source) {
  return ArrayView<T>(std::begin(source), std::end(source));
}

template <typename T, typename U>
ArrayView<T> make_data_array_view(U&& source) {
  return ArrayView<T>(std::forward<U>(source).data(),
                      std::forward<U>(source).data() + std::forward<U>(source).size());
}

template <typename T>
ArrayView<const T> make_view(const std::vector<T>& v) {
  return ArrayView<const T>{v.data(), v.data() + v.size()};
}

template <typename T, int N>
ArrayView<const T> make_view(const DynamicArray<T, N>& v) {
  return ArrayView<const T>{v.data(), v.data() + v.size()};
}

template <typename T>
ArrayView<T> make_mut_view(std::vector<T>& v) {
  return ArrayView<T>{v.data(), v.data() + v.size()};
}

template <typename T, int N>
ArrayView<T> make_mut_view(DynamicArray<T, N>& v) {
  return ArrayView<T>{v.data(), v.data() + v.size()};
}

/*
 * Impl
 */

template <typename T>
ArrayView<T>::ArrayView() :
  beg_(nullptr),
  end_(nullptr) {
  //
}

template <typename T>
ArrayView<T>::ArrayView(T* beg, T* end) :
  beg_(beg),
  end_(end) {
  //
}

template <typename T>
int64_t ArrayView<T>::size() const {
  return end_ - beg_;
}

template <typename T>
T* ArrayView<T>::begin() {
  return beg_;
}

template <typename T>
const T* ArrayView<T>::begin() const {
  return beg_;
}

template <typename T>
T* ArrayView<T>::end() {
  return end_;
}

template <typename T>
const T* ArrayView<T>::end() const {
  return end_;
}

template <typename T>
const T& ArrayView<T>::operator[](int64_t off) const {
  assert(off >= 0 && off < size());
  return begin()[off];
}

template <typename T>
T& ArrayView<T>::operator[](int64_t off) {
  assert(off >= 0 && off < size());
  return begin()[off];
}

//  image

enum class IntegralType {
  Byte,
  UnsignedByte,
  Short,
  UnsignedShort,
  Int,
  UnsignedInt,
  UnconvertedUnsignedInt,
  HalfFloat,
  Float,
  Double
};

inline std::size_t size_of_integral_type(IntegralType type) {
  switch (type) {
    case IntegralType::Byte:
      return 1;
    case IntegralType::UnsignedByte:
      return 1;
    case IntegralType::Short:
      return 2;
    case IntegralType::UnsignedShort:
      return 2;
    case IntegralType::Int:
      return 4;
    case IntegralType::UnsignedInt:
    case IntegralType::UnconvertedUnsignedInt:
      return 4;
    case IntegralType::HalfFloat:
      return 2;
    case IntegralType::Float:
      return 4;
    case IntegralType::Double:
      return 8;
    default:
      assert(false);
      return 1;
  }
}

namespace image {

struct Shape {
  static Shape make_2d(int width, int height) {
    return Shape{width, height, 1};
  }
  static Shape make_3d(int width, int height, int depth) {
    return Shape{width, height, depth};
  }
  int64_t num_elements() const {
    return int64_t(width) * int64_t(height) * int64_t(depth);
  }

  int width;
  int height;
  int depth;
};

struct Channels {
  static constexpr int max_num_channels = 8;
  using Array = std::array<IntegralType, max_num_channels>;

  static Channels make_n_of_type(int num, IntegralType type) {
    assert(num > 0);
    Channels res{};
    for (int i = 0; i < num; i++) {
      res.add_channel(type);
    }
    return res;
  }

  static Channels make_uint8n(int num) {
    return make_n_of_type(num, IntegralType::UnsignedByte);
  }

  static Channels make_floatn(int num) {
    return make_n_of_type(num, IntegralType::Float);
  }

  void add_channel(IntegralType type) {
    assert(num_channels < max_num_channels);
    channels[num_channels++] = type;
  }

  std::optional<IntegralType> single_channel_type() const;
  bool has_single_channel_type(IntegralType type) const;
  bool is_uint8n(int n) const;
  bool is_floatn(int n) const;

  IntegralType operator[](int idx) const {
    return channels[idx];
  }

  IntegralType& operator[](int idx) {
    return channels[idx];
  }

  size_t size_bytes() const {
    size_t sz{};
    for (int i = 0; i < num_channels; i++) {
      sz += size_of_integral_type(channels[i]);
    }
    return sz;
  }

  Array channels;
  int num_channels;
};

struct Descriptor {
  static Descriptor make_2d_floatn(int w, int h, int nc) {
    return Descriptor{Shape::make_2d(w, h), Channels::make_floatn(nc)};
  }
  static Descriptor make_2d_uint8n(int w, int h, int nc) {
    return Descriptor{Shape::make_2d(w, h), Channels::make_uint8n(nc)};
  }
  static Descriptor make_2d_int32n(int w, int h, int nc) {
    return Descriptor{Shape::make_2d(w, h), Channels::make_n_of_type(nc, IntegralType::Int)};
  }

  void add_channel(IntegralType type) {
    channels.add_channel(type);
  }

  size_t element_size_bytes() const {
    return channels.size_bytes();
  }

  int64_t num_elements() const {
    return shape.num_elements();
  }

  size_t total_size_bytes() const {
    return num_elements() * element_size_bytes();
  }

  bool is_2d() const {
    return shape.depth <= 1;
  }

  int rows() const {
    return shape.height;
  }
  int height() const {
    return shape.height;
  }
  int cols() const {
    return shape.width;
  }
  int width() const {
    return shape.width;
  }
  int num_channels() const {
    return channels.num_channels;
  }

  Shape shape;
  Channels channels;
};

} //  image

//  fs

namespace fs {

bool file_size(const std::string& file_path, size_t* size);
bool read_bytes(const std::string& file_path, void* data, size_t data_capacity, size_t* want_write);

} //  fs