#include "util.hpp"
#include <fstream>

bool fs::file_size(const std::string& file_path, size_t* size) {
  *size = 0;

  std::ifstream file;
  try {
    file.open(file_path.c_str(), std::ios_base::in | std::ios_base::binary);
  } catch (...) {
    return false;
  }

  if (!file.good()) {
    return false;
  }

  file.seekg(0, file.end);
  const int64_t length = file.tellg();
  file.seekg(0, file.beg);
  *size = size_t(length);
  return true;
}

bool fs::read_bytes(
  const std::string& file_path, void* data, size_t data_capacity, size_t* want_write) {
  //
  *want_write = 0;

  std::ifstream file;
  try {
    file.open(file_path.c_str(), std::ios_base::in | std::ios_base::binary);
  } catch (...) {
    return false;
  }

  if (!file.good()) {
    return false;
  }

  file.seekg(0, file.end);
  const int64_t length = file.tellg();
  file.seekg(0, file.beg);
  *want_write = size_t(length);

  if (*want_write <= data_capacity) {
    file.read(static_cast<char*>(data), length);
    return true;
  } else {
    return false;
  }
}