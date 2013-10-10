#pragma once

#include <cstdint>

template <typename I>
class DummyStream {
public:
  void open(uint64_t start, uint64_t end, size_t buffer_size);
  I read_next();
  void write(I value);
  void close();

private:
  static const size_t kMax = 10000;
  static I buffer_[kMax];
  uint64_t position;
};