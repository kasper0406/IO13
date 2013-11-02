#pragma once

#include <string>
#include <fcntl.h>
#include <cstdio>
#include <cstdint>
#include <cassert>
#include <unistd.h>

template <typename I>
class SysStream {
public:
  SysStream() : fd(-1) {}

  static void cleanup() {}
  
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    end_ = end;
    start_ = start;
    position_ = start;

    fd = ::open(filename.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

    if (fd == -1) {
      perror("Error open file");
      exit(1);
    }

    if (::lseek(fd, start * sizeof(I), SEEK_SET) == -1) {
      perror("Error seek in open");
      exit(1);
    }
  }

  I peek() {
    assert(position_ >= start_ && position_ < end_);
    I element;
    if (::read(fd, &element, sizeof(I)) != sizeof(I)) {
      perror("Error peek");
      exit(1);
    }
    ::lseek(fd, -sizeof(I), SEEK_CUR);

    return element;
  }
  
  I read_next() {
    assert(position_ >= start_ && position_ < end_);
    I element;
    if (::read(fd, &element, sizeof(I)) != sizeof(I)) {
      perror("Error reading");
      exit(1);
    }
    position_++;

    return element;
  }

  I read_prev() {
    assert(position_ >= start_ && position_ < end_);
    I result = read_next();
    if (position_ != 1 && ::lseek(fd, -2 * sizeof(I), SEEK_CUR) == -1) {
      perror("Error seek");
      exit(1);
    }
    position_-=2;

    return result;
  }

  int64_t position() {
    return position_;
  }
  
  void write(I value) {
    if (::write(fd, &value, sizeof(I)) != sizeof(I)) {
      perror("Error writing");
      exit(1);
    }
    position_++;
  }

  void backward_write(I value) {
    write(value);
    if (position_ != 1 && ::lseek(fd, -2 * sizeof(I), SEEK_CUR) == -1) {
      perror("Error seek");
      exit(1);
    }
    position_-=2;
  }
  
  void close() {
    if (::close(fd) == -1) {
      perror("Error closing");
      exit(1);
    }
    fd = -1;
  }
  
  void seek(uint64_t position) {
    if (::lseek(fd, position * sizeof(I), SEEK_SET) == -1) {
      perror("Error seek");
      exit(1);
    }
    position_ = position;
  }
  
  bool has_next() {
    return position() < end_;
  }

private:
  uint64_t start_;
  uint64_t end_;
  int fd;
  int64_t position_;
};