#pragma once

#include <string>
#include <fcntl.h>
#include <cstdio>
#include <cstdint>
#include <cassert>

template <typename I>
class FStream {
public:
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    end_ = end;
    position_ = start;

    //fd = ::open(filename.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
  }

  I peek() {
    assert(position_ < end_);
    //return buffer_[position_];
    return 0;
  }
  
  I read_next() {
    I res = peek();
    position_++;
    return res;
  }
  
  void write(I value) {
    assert(position_ < end_);
    //buffer_[position_++] = value;
  }
  void close() {
    /*if (::close(fd) == -1) {
      throw runtime_error("Could not close stream");
    }*/
  }
  void seek(uint64_t position) {
    position_ = position;
  }
  
  bool has_next() {
    return position_ < end_;
  }

private:
  uint64_t position_;
  uint64_t end_;
  int fd;
};
