#pragma once

#include <cstdint>
#include <cassert>

template <typename I>
class DummyStream {
public:
  void open(uint64_t start, uint64_t end, size_t buffer_size) {
    end_ = end;
    position_ = start;
  }

  I read_next();
  void write(I value) {
    assert(position_ < kMax);
    assert(position_ < end_);
    buffer_[position_++] = value;
  }
  void close() {}
  void seek(uint64_t position) {
    position_ = position;
  }

  static const size_t kMax = 10000;
  static I buffer_[kMax];

private:
  uint64_t position_;
  uint64_t end_;
};

template <typename I>
I DummyStream<I>::buffer_[kMax] = {0};
