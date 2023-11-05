#include "buffer.h"
#include <stdexcept>

// Returns a buffer initialized to value
void get_buffer(uint8_t * &buf, size_t &len, uint32_t value) {
  buf = new uint8_t[len];
  auto *data = reinterpret_cast<uint32_t *>(buf);
  for (size_t i = 0; i < len * sizeof(uint8_t) / sizeof(uint32_t); i++) {
    data[i] = value;
  }
}

// Receives a buffer that must be initialized to value
void put_buffer(uint8_t *buf, size_t len, uint32_t value) {
  auto *data = reinterpret_cast<uint32_t *>(buf);
  for (size_t i = 0; i < len * sizeof(uint8_t) / sizeof(uint32_t); i++) {
    if (data[i] != value) throw std::runtime_error("Invalid value");
  }
}
