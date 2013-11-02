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
    I element;
    if (::read(fd, &element, sizeof(I)) != sizeof(I)) {
      perror("Error peek");
      exit(1);
    }
    ::lseek(fd, -sizeof(I), SEEK_CUR);

    return element;
  }
  
  I read_next() {
    I element;
    if (::read(fd, &element, sizeof(I)) != sizeof(I)) {
      perror("Error reading");
      exit(1);
    }

    return element;
  }

  I read_prev() {
    I result = read_next();
    if (::lseek(fd, -2 * sizeof(I), SEEK_CUR) == -1) {
      perror("Error seek");
      exit(1);
    }
    return result;
  }

  uint64_t position() {
    uint64_t position = lseek(fd, 0, SEEK_CUR);
    if (position == -1) {
      perror("Error get position");
      exit(1);
    }
    return position / (long)sizeof(I);
  }
  
  void write(I value) {
    if (::write(fd, &value, sizeof(I)) != sizeof(I)) {
      perror("Error writing");
      exit(1);
    }
  }

  void backward_write(I value) {
    write(value);
    if (::lseek(fd, -2 * sizeof(I), SEEK_CUR) == -1) {
      perror("Error seek");
      exit(1);
    }
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
  }
  
  bool has_next() {
    return position() < end_;
  }

private:
  uint64_t start_;
  uint64_t end_;
  int fd;
};