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
    buffer_start_ = -1000000000000;
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
    return read_from_buffer();
  }
  
  I read_next() {
    I result = read_from_buffer();
    position_++;
    return result;
  }
  
  void write(I value) {
    write_to_buffer(value);
    position_++;
  }
  
  void close() {
    flush_buffer();

    if (::close(fd) == -1) {
      perror("Error closing");
      exit(1);
    }
    delete[] buffer_;
    buffer_ = nullptr;
    fd = -1;
  }
  
  void seek(uint64_t position) {
    assert(start_ <= position);
    assert(position < end_);
    position_ = position;
  }
  
  bool has_next() {
    return position_ < end_;
  }

private:
  // Does not change position_
  I read_from_buffer() {
    assert(position_ < end_);
    assert(start_ <= position_);

    int buffer_position = position_ - buffer_start_;

    if (0 <= buffer_position && buffer_position < buffer_size_) {
      return buffer_[buffer_position];
    } else {
      refresh_buffer();

      return read_from_buffer();  // Should not cycle
    }
  }

  // Does not change position_
  void write_to_buffer(I value) {
    assert(position_ < end_);
    assert(start_ <= position_);

    int buffer_position = position_ - buffer_start_;

    if (buffer_position < buffer_size_) {
      buffer_[buffer_position] = value;
    } else {
      refresh_buffer();

      write_to_buffer(value);  // Should not cycle
    }
  }

  void flush_buffer() {
    if (buffer_start_ < 0) return;  // Not even read.
    
    if (::lseek(fd, buffer_start_ * sizeof(I), SEEK_SET) == -1) {
      perror("Error seek");
      exit(1);
    }

    if (::write(fd, buffer_, utilized_buffer_size_ * sizeof(I)) != utilized_buffer_size_ * sizeof(I)) {
      perror("Error writing");
      exit(1);
    }
  }

  void refresh_buffer() {
    flush_buffer();

    // TODO(lespeholt): Heuristik, hvis nye position er foer, saa laes bagud, ellers forud
    // 'position_' is in the middle of the new buffer
    buffer_start_ = max(position_ - (buffer_size_ / 2), (int64_t)0);
    int64_t buffer_end = min(buffer_start_ + buffer_size_, end_);
    utilized_buffer_size_ = buffer_end - buffer_start_;

    if (::lseek(fd, buffer_start_ * sizeof(I), SEEK_SET) == -1) {
      perror("Error seek");
      exit(1);
    }

    // TODO(lespeholt): Not really necessary to read buffer when only writes are performed!

    if (::read(fd, buffer_, utilized_buffer_size_ * sizeof(I)) != utilized_buffer_size_ * sizeof(I)) {
      perror("Error reading");
    }
  }

  int64_t start_;
  int64_t end_;
  int fd;
  I* buffer_;
  int64_t buffer_size_;
  int64_t utilized_buffer_size_;
  int64_t buffer_start_;
  int64_t position_;
};