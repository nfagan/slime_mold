#pragma once

#include <cmath>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <limits>

/*
 * Vec2
 */

template <typename T>
struct Vec2 {
  struct Less {
    inline bool operator()(const Vec2<T>& a, const Vec2<T>& b) const noexcept {
      return a.x < b.x || (a.x == b.x && a.y < b.y);
    }
  };

  constexpr Vec2() = default;
  constexpr explicit Vec2(T c);
  constexpr Vec2(T x, T y);

  Vec2<T>& operator+=(T v);
  Vec2<T>& operator-=(T v);
  Vec2<T>& operator*=(T v);
  Vec2<T>& operator/=(T v);

  Vec2<T>& operator+=(const Vec2<T>& v);
  Vec2<T>& operator-=(const Vec2<T>& v);
  Vec2<T>& operator*=(const Vec2<T>& v);
  Vec2<T>& operator/=(const Vec2<T>& v);

  T& operator[](int idx);
  const T& operator[](int idx) const;

  T length_squared() const;
  T length() const;
  void normalize();

  T x;
  T y;
};

/*
* Impl
*/

template <typename T>
constexpr Vec2<T>::Vec2(T x, T y) : x(x), y(y) {
  //
}

template <typename T>
constexpr Vec2<T>::Vec2(T c) : x(c), y(c) {
  //
}

//  Member functions
template <typename T>
T Vec2<T>::length() const {
  return std::sqrt(length_squared());
}

template <typename T>
T Vec2<T>::length_squared() const {
  return x * x + y * y;
}

template <typename T>
void Vec2<T>::normalize() {
  auto inv_len = T(1) / length();
  x *= inv_len;
  y *= inv_len;
}

template <typename T>
T& Vec2<T>::operator[](int idx) {
  switch (idx) {
    case 0:
      return x;
    case 1:
      return y;
    default:
      assert(false);
      return x;
  }
}

template <typename T>
const T& Vec2<T>::operator[](int idx) const {
  switch (idx) {
    case 0:
      return x;
    case 1:
      return y;
    default:
      assert(false);
      return x;
  }
}

//  Member ops
template <typename T>
Vec2<T>& Vec2<T>::operator+=(T v) {
  x += v;
  y += v;
  return *this;
}

template <typename T>
Vec2<T>& Vec2<T>::operator-=(T v) {
  x -= v;
  y -= v;
  return *this;
}

template <typename T>
Vec2<T>& Vec2<T>::operator*=(T v) {
  x *= v;
  y *= v;
  return *this;
}

template <typename T>
Vec2<T>& Vec2<T>::operator/=(T v) {
  x /= v;
  y /= v;
  return *this;
}

//  Vec other
template <typename T>
Vec2<T>& Vec2<T>::operator+=(const Vec2<T>& v) {
  x += v.x;
  y += v.y;
  return *this;
}

template <typename T>
Vec2<T>& Vec2<T>::operator-=(const Vec2<T>& v) {
  x -= v.x;
  y -= v.y;
  return *this;
}

template <typename T>
Vec2<T>& Vec2<T>::operator*=(const Vec2<T>& v) {
  x *= v.x;
  y *= v.y;
  return *this;
}

template <typename T>
Vec2<T>& Vec2<T>::operator/=(const Vec2<T>& v) {
  x /= v.x;
  y /= v.y;
  return *this;
}

//  (vec, vec)
template <typename T>
inline Vec2<T> operator+(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<T>(a.x + b.x, a.y + b.y);
}

template <typename T>
inline Vec2<T> operator-(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<T>(a.x - b.x, a.y - b.y);
}

template <typename T>
inline Vec2<T> operator*(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<T>(a.x * b.x, a.y * b.y);
}

template <typename T>
inline Vec2<T> operator/(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<T>(a.x / b.x, a.y / b.y);
}

template <typename T>
inline Vec2<T> operator-(const Vec2<T>& a) {
  return Vec2<T>(-a.x, -a.y);
}

//  (T, vec)
template <typename T>
inline Vec2<T> operator+(T v, const Vec2<T>& a) {
  return Vec2<T>(v + a.x, v + a.y);
}

template <typename T>
inline Vec2<T> operator-(T v, const Vec2<T>& a) {
  return Vec2<T>(v - a.x, v - a.y);
}

template <typename T>
inline Vec2<T> operator*(T v, const Vec2<T>& a) {
  return Vec2<T>(v * a.x, v * a.y);
}

template <typename T>
inline Vec2<T> operator/(T v, const Vec2<T>& a) {
  return Vec2<T>(v / a.x, v / a.y);
}

//  (vec, T)
template <typename T>
inline Vec2<T> operator+(const Vec2<T>& a, T v) {
  return Vec2<T>(a.x + v, a.y + v);
}

template <typename T>
inline Vec2<T> operator-(const Vec2<T>& a, T v) {
  return Vec2<T>(a.x - v, a.y - v);
}

template <typename T>
inline Vec2<T> operator*(const Vec2<T>& a, T v) {
  return Vec2<T>(a.x * v, a.y * v);
}

template <typename T>
inline Vec2<T> operator/(const Vec2<T>& a, T v) {
  return Vec2<T>(a.x / v, a.y / v);
}

template <typename T>
inline bool operator==(const Vec2<T>& a, const Vec2<T>& b) {
  return a.x == b.x && a.y == b.y;
}

template <typename T>
inline bool operator!=(const Vec2<T>& a, const Vec2<T>& b) {
  return !(a == b);
}

//  Util
template <typename T>
Vec2<T> normalize(const Vec2<T>& a) {
  auto inv_len = T(1) / a.length();
  return Vec2<T>(a.x * inv_len, a.y * inv_len);
}

template <typename T>
T dot(const Vec2<T>& a, const Vec2<T>& b) {
  return a.x * b.x + a.y * b.y;
}

template <typename T>
Vec2<T> min(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<T>(std::min(a.x, b.x),
                 std::min(a.y, b.y));
}

template <typename T>
Vec2<T> max(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<T>(std::max(a.x, b.x),
                 std::max(a.y, b.y));
}

template <typename T>
Vec2<T> floor(const Vec2<T>& a) {
  return Vec2<T>(std::floor(a.x), std::floor(a.y));
}

template <typename T>
Vec2<T> round(const Vec2<T>& a) {
  return Vec2<T>(std::round(a.x), std::round(a.y));
}

template <typename T>
Vec2<T> abs(const Vec2<T>& a) {
  return Vec2<T>(std::abs(a.x), std::abs(a.y));
}

template <typename T>
bool any(const Vec2<T>& a) {
  return bool(a.x) || bool(a.y);
}

template <typename T>
bool all(const Vec2<T>& a) {
  return bool(a.x) && bool(a.y);
}

template <typename T>
Vec2<bool> lt(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<bool>{a.x < b.x, a.y < b.y};
}

template <typename T>
Vec2<bool> le(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<bool>{a.x <= b.x, a.y <= b.y};
}

template <typename T>
Vec2<bool> gt(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<bool>{a.x > b.x, a.y > b.y};
}

template <typename T>
Vec2<bool> ge(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<bool>{a.x >= b.x, a.y >= b.y};
}

template <typename T>
Vec2<bool> eq(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<bool>{a.x == b.x, a.y == b.y};
}

template <typename T>
Vec2<bool> ne(const Vec2<T>& a, const Vec2<T>& b) {
  return Vec2<bool>{a.x != b.x, a.y != b.y};
}

template <typename T>
Vec2<T> clamp_each(const Vec2<T>& a, const Vec2<T>& min, const Vec2<T>& max) {
  return Vec2<T>(a.x < min.x ? min.x : a.x > max.x ? max.x : a.x,
                 a.y < min.y ? min.y : a.y > max.y ? max.y : a.y);
}

template <typename T>
T det3_implicit(const Vec2<T>& p0, const Vec2<T>& p1, const Vec2<T>& p2) {
  //  Determinant of 3x3 matrix [p0, p1, p2] with a last row of [1]
  T a = p0.x;
  T b = p0.y;
  T c = p1.x;
  T d = p1.y;
  T e = p2.x;
  T f = p2.y;
  return a * (d - f) - c * (b - f) + e * (b - d);
}

//  Global typedefs
using Vec2f = Vec2<float>;

template <typename T>
struct Vec2;

/*
 * Vec3
 */

template <typename T>
struct Vec3 {
  struct Less {
    inline bool operator()(const Vec3<T>& a, const Vec3<T>& b) const noexcept {
      return std::tie(a.x, a.y, a.z) < std::tie(b.x, b.y, b.z);
    }
  };

  Vec3() = default;
  constexpr explicit Vec3(T c);
  constexpr Vec3(T x, T y, T z);
  Vec3(const Vec2<T>& v, T c);

  Vec3<T>& operator+=(T v);
  Vec3<T>& operator-=(T v);
  Vec3<T>& operator*=(T v);
  Vec3<T>& operator/=(T v);

  Vec3<T>& operator+=(const Vec3<T>& v);
  Vec3<T>& operator-=(const Vec3<T>& v);
  Vec3<T>& operator*=(const Vec3<T>& v);
  Vec3<T>& operator/=(const Vec3<T>& v);

  T& operator[](int idx);
  const T& operator[](int idx) const;

  T length_squared() const;
  T length() const;
  void normalize();

  T x;
  T y;
  T z;
};

template <typename T>
struct ConstVec3 {
  static constexpr Vec3<T> positive_x{T(1), T(0), T(0)};
  static constexpr Vec3<T> positive_y{T(0), T(1), T(0)};
  static constexpr Vec3<T> positive_z{T(0), T(0), T(1)};
};

/*
 * Impl
 */

template <typename T>
constexpr Vec3<T>::Vec3(T x, T y, T z) : x(x), y(y), z(z) {
  //
}

template <typename T>
constexpr Vec3<T>::Vec3(T c) : x(c), y(c), z(c) {
  //
}

template <typename T>
Vec3<T>::Vec3(const Vec2<T>& v, T c) : x(v.x), y(v.y), z(c) {
  //
}

//  Member functions
template <typename T>
T Vec3<T>::length() const {
  return std::sqrt(length_squared());
}

template <typename T>
T Vec3<T>::length_squared() const {
  return x * x + y * y + z * z;
}

template <typename T>
void Vec3<T>::normalize() {
  auto inv_len = T(1) / length();
  x *= inv_len;
  y *= inv_len;
  z *= inv_len;
}

template <typename T>
T& Vec3<T>::operator[](int idx) {
  switch (idx) {
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    default:
      assert(false);
      return x;
  }
}

template <typename T>
const T& Vec3<T>::operator[](int idx) const {
  switch (idx) {
    case 0:
      return x;
    case 1:
      return y;
    case 2:
      return z;
    default:
      assert(false);
      return x;
  }
}

//  Member ops
template <typename T>
Vec3<T>& Vec3<T>::operator+=(T v) {
  x += v;
  y += v;
  z += v;
  return *this;
}

template <typename T>
Vec3<T>& Vec3<T>::operator-=(T v) {
  x -= v;
  y -= v;
  z -= v;
  return *this;
}

template <typename T>
Vec3<T>& Vec3<T>::operator*=(T v) {
  x *= v;
  y *= v;
  z *= v;
  return *this;
}

template <typename T>
Vec3<T>& Vec3<T>::operator/=(T v) {
  x /= v;
  y /= v;
  z /= v;
  return *this;
}

//  Vec other
template <typename T>
Vec3<T>& Vec3<T>::operator+=(const Vec3<T>& v) {
  x += v.x;
  y += v.y;
  z += v.z;
  return *this;
}

template <typename T>
Vec3<T>& Vec3<T>::operator-=(const Vec3<T>& v) {
  x -= v.x;
  y -= v.y;
  z -= v.z;
  return *this;
}

template <typename T>
Vec3<T>& Vec3<T>::operator*=(const Vec3<T>& v) {
  x *= v.x;
  y *= v.y;
  z *= v.z;
  return *this;
}

template <typename T>
Vec3<T>& Vec3<T>::operator/=(const Vec3<T>& v) {
  x /= v.x;
  y /= v.y;
  z /= v.z;
  return *this;
}

//  (vec, vec)
template <typename T>
inline Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}

template <typename T>
inline Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template <typename T>
inline Vec3<T> operator*(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<T>(a.x * b.x, a.y * b.y, a.z * b.z);
}

template <typename T>
inline Vec3<T> operator/(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<T>(a.x / b.x, a.y / b.y, a.z / b.z);
}

template <typename T>
inline Vec3<T> operator-(const Vec3<T>& a) {
  return Vec3<T>(-a.x, -a.y, -a.z);
}

//  (T, vec)
template <typename T>
inline Vec3<T> operator+(T v, const Vec3<T>& a) {
  return Vec3<T>(v + a.x, v + a.y, v + a.z);
}

template <typename T>
inline Vec3<T> operator-(T v, const Vec3<T>& a) {
  return Vec3<T>(v - a.x, v - a.y, v - a.z);
}

template <typename T>
inline Vec3<T> operator*(T v, const Vec3<T>& a) {
  return Vec3<T>(v * a.x, v * a.y, v * a.z);
}

template <typename T>
inline Vec3<T> operator/(T v, const Vec3<T>& a) {
  return Vec3<T>(v / a.x, v / a.y, v / a.z);
}

//  (vec, T)
template <typename T>
inline Vec3<T> operator+(const Vec3<T>& a, T v) {
  return Vec3<T>(a.x + v, a.y + v, a.z + v);
}

template <typename T>
inline Vec3<T> operator-(const Vec3<T>& a, T v) {
  return Vec3<T>(a.x - v, a.y - v, a.z - v);
}

template <typename T>
inline Vec3<T> operator*(const Vec3<T>& a, T v) {
  return Vec3<T>(a.x * v, a.y * v, a.z * v);
}

template <typename T>
inline Vec3<T> operator/(const Vec3<T>& a, T v) {
  return Vec3<T>(a.x / v, a.y / v, a.z / v);
}

template <typename T>
inline bool operator==(const Vec3<T>& a, const Vec3<T>& b) {
  return a.x == b.x && a.y == b.y && a.z == b.z;
}

template <typename T>
inline bool operator!=(const Vec3<T>& a, const Vec3<T>& b) {
  return !(a == b);
}

//  Util
template <typename T>
Vec3<T> normalize(const Vec3<T>& a) {
  auto inv_len = T(1) / a.length();
  return Vec3<T>(a.x * inv_len, a.y * inv_len, a.z * inv_len);
}

template <typename T>
Vec3<T> normalize_or_default(const Vec3<T>& a, const Vec3<T>& dflt) {
  auto len = a.length();
  if (len == T(0)) {
    return dflt;
  } else {
    auto inv_len = T(1) / len;
    return Vec3<T>(a.x * inv_len, a.y * inv_len, a.z * inv_len);
  }
}

template <typename T>
T dot(const Vec3<T>& a, const Vec3<T>& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
Vec3<T> cross(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<T>(a.y * b.z - a.z * b.y,
                 a.z * b.x - a.x * b.z,
                 a.x * b.y - a.y * b.x);
}

template <typename T>
Vec3<T> min(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<T>(std::min(a.x, b.x),
                 std::min(a.y, b.y),
                 std::min(a.z, b.z));
}

template <typename T>
Vec3<T> max(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<T>(std::max(a.x, b.x),
                 std::max(a.y, b.y),
                 std::max(a.z, b.z));
}

template <typename T>
Vec3<T> clamp_each(const Vec3<T>& a, const Vec3<T>& min, const Vec3<T>& max) {
  return Vec3<T>(a.x < min.x ? min.x : a.x > max.x ? max.x : a.x,
                 a.y < min.y ? min.y : a.y > max.y ? max.y : a.y,
                 a.z < min.z ? min.z : a.z > max.z ? max.z : a.z);
}

template <typename T>
Vec3<T> abs(const Vec3<T>& a) {
  return Vec3<T>(std::abs(a.x), std::abs(a.y), std::abs(a.z));
}

template <typename T>
Vec3<T> floor(const Vec3<T>& a) {
  return Vec3<T>(std::floor(a.x), std::floor(a.y), std::floor(a.z));
}

template <typename T>
Vec3<T> ceil(const Vec3<T>& a) {
  return Vec3<T>(std::ceil(a.x), std::ceil(a.y), std::ceil(a.z));
}

template <typename T>
Vec3<T> round(const Vec3<T>& a) {
  return Vec3<T>(std::round(a.x), std::round(a.y), std::round(a.z));
}

template <typename T>
T prod(const Vec3<T>& a) {
  return a.x * a.y * a.z;
}

template <typename T>
T sum(const Vec3<T>& a) {
  return a.x + a.y + a.z;
}

template <typename T>
int max_dimension(const Vec3<T>& a) {
  auto m = a.x;
  int i = 0;

  if (a.y > m) {
    m = a.y;
    i = 1;
  }

  if (a.z > m) {
    i = 2;
  }

  return i;
}

template <typename T>
int min_dimension(const Vec3<T>& a) {
  auto m = a.x;
  int i = 0;

  if (a.y < m) {
    m = a.y;
    i = 1;
  }

  if (a.z < m) {
    i = 2;
  }

  return i;
}

template <typename T>
bool any(const Vec3<T>& a) {
  return bool(a.x) || bool(a.y) || bool(a.z);
}

template <typename T>
bool all(const Vec3<T>& a) {
  return bool(a.x) && bool(a.y) && bool(a.z);
}

template <typename T>
Vec3<bool> lt(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<bool>{a.x < b.x, a.y < b.y, a.z < b.z};
}

template <typename T>
Vec3<bool> le(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<bool>{a.x <= b.x, a.y <= b.y, a.z <= b.z};
}

template <typename T>
Vec3<bool> gt(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<bool>{a.x > b.x, a.y > b.y, a.z > b.z};
}

template <typename T>
Vec3<bool> ge(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<bool>{a.x >= b.x, a.y >= b.y, a.z >= b.z};
}

template <typename T>
Vec3<bool> eq(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<bool>{a.x == b.x, a.y == b.y, a.z == b.z};
}

template <typename T>
Vec3<bool> ne(const Vec3<T>& a, const Vec3<T>& b) {
  return Vec3<bool>{a.x != b.x, a.y != b.y, a.z != b.z};
}

template <typename T>
Vec3<T> permute(const Vec3<T>& a, int i, int j, int k) {
  return Vec3<T>(a[i], a[j], a[k]);
}

template <typename T>
T det(const Vec3<T>& p0, const Vec3<T>& p1, const Vec3<T>& p2) {
  T m0 = p0.x * (p1.y * p2.z - p2.y * p1.z);
  T m1 = p1.x * (p0.y * p2.z - p2.y * p0.z);
  T m2 = p2.x * (p0.y * p1.z - p1.y * p0.z);
  return m0 - m1 + m2;
}

template <typename T>
Vec2<T> exclude(const Vec3<T>& a, int dim) {
  assert(dim >= 0 && dim < 3);
  return dim == 0 ? Vec2<T>{a.y, a.z} : dim == 1 ? Vec2<T>{a.x, a.z} : Vec2<T>{a.x, a.y};
}

template <typename T>
void invert_implicit_3x3(const Vec3<T>& a, const Vec3<T>& b, const Vec3<T>& c,
                         Vec3<T>* i, Vec3<T>* j, Vec3<T>* k) {
  //  Lengyel, E. Foundations of Game Engine Development, Vol. 1. pp. 48.
  auto r0 = cross(b, c);
  auto r1 = cross(c, a);
  auto r2 = cross(a, b);
  auto inv_det = T(1) / dot(r2, c);
  r0 *= inv_det;
  r1 *= inv_det;
  r2 *= inv_det;
  *i = Vec3<T>{r0.x, r1.x, r2.x};
  *j = Vec3<T>{r0.y, r1.y, r2.y};
  *k = Vec3<T>{r0.z, r1.z, r2.z};
}

template <typename T>
void mul_implicit_3x3(const Vec3<T>& a, const Vec3<T>& b, const Vec3<T>& c,
                      const Vec3<T>& d, const Vec3<T>& e, const Vec3<T>& f,
                      Vec3<T>* i, Vec3<T>* j, Vec3<T>* k) {
  *i = a * d.x + b * d.y + c * d.z;
  *j = a * e.x + b * e.y + c * e.z;
  *k = a * f.x + b * f.y + c * f.z;
}

template <typename T>
Vec3<float> to_vec3f(const Vec3<T>& a) {
  return Vec3<float>{float(a.x), float(a.y), float(a.z)};
}

template <typename T>
Vec3<int> to_vec3i(const Vec3<T>& a) {
  return Vec3<int>{int(a.x), int(a.y), int(a.z)};
}

//  Global typedefs
using Vec3f = Vec3<float>;
using ConstVec3f = ConstVec3<float>;

/*
 * Bounds2
 */

template <typename T>
struct Bounds2 {
public:
  using Vector = Vec2<T>;

public:
  constexpr Bounds2();
  constexpr Bounds2(const Vector& min, const Vector& max);

  Vector center() const;
  Vector size() const;
  Vector to_fraction(const Vector& p) const;

  static Bounds2<T> from_min_max_components(const Vector& a, const Vector& b);
  static Bounds2<T> largest();

public:
  Vector min;
  Vector max;
};

template <typename T>
constexpr Bounds2<T>::Bounds2() :
  min{std::numeric_limits<T>::max()},
  max{std::numeric_limits<T>::lowest()} {
  //
}

template <typename T>
constexpr Bounds2<T>::Bounds2(const Vector& min, const Vector& max) :
  min{min}, max{max} {
  //
}

template <typename T>
Bounds2<T> Bounds2<T>::from_min_max_components(const Vector& a, const Vector& b) {
  Vector p0{std::min(a.x, b.x),
            std::min(a.y, b.y)};
  Vector p1{std::max(a.x, b.x),
            std::max(a.y, b.y)};
  return Bounds2<T>{p0, p1};
}

template <typename T>
Bounds2<T> Bounds2<T>::largest() {
  return Bounds2<T>{Vec2<T>(std::numeric_limits<T>::lowest()),
                    Vec2<T>(std::numeric_limits<T>::max())};
}

template <typename T>
inline typename Bounds2<T>::Vector Bounds2<T>::center() const {
  return min + (max - min) / T(2);
}

template <typename T>
inline typename Bounds2<T>::Vector Bounds2<T>::size() const {
  return max - min;
}

template <typename T>
inline typename Bounds2<T>::Vector Bounds2<T>::to_fraction(const Vector& p) const {
  auto span = max - min;
  return (p - min) / span;
}

template <typename T>
Bounds2<T> scale(Bounds2<T> b, const Vec2<T>& frac) {
  auto curr = b.size();
  auto next = curr * frac;
  auto delta = (next - curr) * T(0.5);
  b.min -= delta;
  b.max += delta;
  return b;
}

template <typename T>
Bounds2<T> union_of(const Bounds2<T>& a, const Bounds2<T>& b) {
  return Bounds2<T>{min(a.min, b.min), max(a.max, b.max)};
}

template <typename T>
Bounds2<T> intersect_of(const Bounds2<T>& a, const Bounds2<T>& b) {
  return Bounds2<T>{max(a.min, b.min), min(a.max, b.max)};
}

using Bounds2f = Bounds2<float>;

/*
 * util
 */

namespace detail {

template <typename T>
T integer_wrap(T value, T size) {
  assert(size > 0);
  if (value >= 0) {
    return value % size;
  } else {
    auto v = std::abs(value) % size;
#if 0
    return size - v;
#else
    return v == 0 ? 0 : size - v;
#endif
  }
}

template <typename T>
T float_wrap(T value, T size) {
  auto floor_cnt = std::floor(value / size);
  auto floor_v = floor_cnt * size;
  return value - floor_v;
}

template <typename T>
struct WrapImpl {};

template <>
struct WrapImpl<float> {
  static float evaluate(float value, float size) {
    return float_wrap(value, size);
  }
};

template <>
struct WrapImpl<double> {
  static double evaluate(double value, double size) {
    return float_wrap(value, size);
  }
};

template <>
struct WrapImpl<int> {
  static int evaluate(int value, int size) {
    return integer_wrap(value, size);
  }
};

template <>
struct WrapImpl<int64_t> {
  static int64_t evaluate(int64_t value, int64_t size) {
    return integer_wrap(value, size);
  }
};

template <typename T, typename U>
struct CountImpl {
  static void count_wraps(T* value, T size, U* count) {
    assert(size != 0);
    while (*value >= size) {
      (*value) -= size;
      (*count)++;
    }
    while (*value < 0) {
      (*value) += size;
      (*count)--;
    }
  }
};

template <typename T, typename U>
struct NonLoopedCount {
  static void count_wraps(T* value, T size, U* count) {
    assert(size != 0);
    auto v = *value;

    auto floor_cnt = std::floor(v / size);
    auto floor_v = floor_cnt * size;
    auto full_count = int64_t(floor_cnt);

    *value = v - floor_v;
    *count += full_count;
  }
};

template <typename T>
struct CountImpl<double, T> {
  static void count_wraps(double* value, double size, T* count) {
    return NonLoopedCount<double, T>::count_wraps(value, size, count);
  }
};

template <typename T>
struct CountImpl<float, T> {
  static void count_wraps(float* value, float size, T* count) {
    return NonLoopedCount<float, T>::count_wraps(value, size, count);
  }
};
} //  detail

template <typename T>
inline T wrap_within_range(T value, T size) {
  return detail::WrapImpl<T>::evaluate(value, size);
}

template <typename T, typename U>
inline void count_wraps_to_range(T* value, T size, U* count) {
  detail::CountImpl<T, U>::count_wraps(value, size, count);
}

template <typename T, typename U>
inline T lerp(U frac, const T& a, const T& b) {
  return (U(1) - frac) * a + frac * b;
}

/*
 * random
 */

double urand();
double urand_closed();
double urand_11();

float urandf();
float urand_11f();

template <typename T>
T* uniform_array_sample(T* array, size_t size) {
  return size == 0 ? nullptr : array + size_t(double(size) * urand());
}