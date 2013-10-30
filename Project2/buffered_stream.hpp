#pragma once

#include <string>
#include <fcntl.h>
#include <cstdio>
#include <cstdint>
#include <cassert>
#include <unistd.h>

template <typename I>
class BufferedStream {
public:
  BufferedStream() : fd(-1), buffer_(nullptr), buffer_start_(-1) {}
  
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    end_ = end;
    start_ = start;
    position_ = start;
    buffer_start_ = numeric_limits<int64_t>::min();
    buffer_size_ = buffer_size;

    buffer_ = new I[buffer_size];

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
  
  void write(I value) {
    if (::write(fd, &value, sizeof(I)) != sizeof(I)) {
      perror("Error writing");
      exit(1);
    }
  }
  
  void close() {
    // TODO(lespeholt): Flush buffer, og lav test

    if (::close(fd) == -1) {
      perror("Error closing");
      exit(1);
    }
    delete[] buffer_;
    buffer_ = nullptr;
    fd = -1;
  }
  
  void seek(uint64_t position) {
    if (::lseek(fd, position * sizeof(I), SEEK_SET) == -1) {
      perror("Error seek");
      exit(1);
    }
  }
  
  bool has_next() {
    auto pos = lseek(fd, 0, SEEK_CUR);
    auto value = pos / (long)sizeof(I);
    return value < end_;
  }

  void refresh_buffer() {
    // TODO(lespeholt): Flush buffer, og lav test


    // TODO(lespeholt): Heuristik, hvis nye position er foer, saa laes bagud, ellers forud
    // 'position_' is in the middle of the new buffer
    buffer_start_ = position_ - (buffer_size_ / 2);
    int64_t buffer_end = min(buffer_start_ + buffer_size_, end_);
    int64_t utilized_buffer_size = buffer_end - buffer_start_;

    if (::lseek(fd, buffer_start_ * sizeof(I), SEEK_SET) == -1) {
      perror("Error seek");
      exit(1);
    }

    // TODO(lespeholt): Not really necessary to read buffer when only writes are performed!

    if (::read(fd, buffer_, utilized_buffer_size) != utilized_buffer_size) {
      perror("Error reading");
    }
  }

private:
  uint64_t start_;
  uint64_t end_;
  int fd;
  I* buffer_;
  size_t buffer_size_;
  int64_t buffer_start_;
  size_t position_;
};