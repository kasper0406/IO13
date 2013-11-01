#pragma once

#include <cstdint>
#include <cassert>

template <typename I>
class DummyStream {
public:
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    end_ = end;
    position_ = start;
    filename_ = filename;
  }

  I peek() {
    assert(position_ < kMax);
    assert(position_ < end_);
    return buffer_[position_];
  }
  
  I read_next() {
    I res = peek();
    position_++;
    return res;
  }
  
  void write(I value) {
    assert(position_ < kMax);
    assert(position_ < end_);
    buffer_[position_++] = value;
  }
  void close() {}
  void seek(uint64_t position) {
    position_ = position;
  }
  
  bool has_next() {
    return position_ < end_;
  }

  static const size_t kMax = 100000000;
  static I buffer_[kMax];
  
  static void cleanup() {
  }

private:
  uint64_t position_;
  uint64_t end_;
  string filename_;
};

template <typename I>
I DummyStream<I>::buffer_[kMax] = {0};
