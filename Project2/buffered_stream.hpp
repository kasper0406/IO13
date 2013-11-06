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
    assert(buffer_size > 0);
    end_ = end;
    start_ = start;
    position_ = start;
    buffer_start_ = -1000000000000;
    buffer_size_ = buffer_size;

    buffer_ = new I[buffer_size];
    write_map_ = new uint8_t[(buffer_size + 7) / 8];
    for (int64_t i = 0; i < (buffer_size_ + 7) / 8; ++i) {
      write_map_[i] = 0;
    }

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

  I read_prev() {
    I result = read_from_buffer();
    position_--;
    return result;
  }
  
  void write(I value) {
    write_to_buffer(value);
    position_++;
  }

  void backward_write(I value) {
    write_to_buffer(value);
    position_--;
  }
  
  void close() {
    flush_buffer();

    if (::close(fd) == -1) {
      perror("Error closing");
      exit(1);
    }
    delete[] buffer_;
    buffer_ = nullptr;
    delete[] write_map_;
    write_map_ = nullptr;
    fd = -1;
  }
  
  void seek(uint64_t position) {
    assert(start_ <= position);
    assert(position < end_);
    position_ = position;
  }

  int64_t position() {
    return position_;
  }
  
  bool has_next() {
    return position_ < end_;
  }

  static void cleanup() {}

private:
  void update(int64_t position) {
    uint8_t selector = 1 << (position - buffer_start_) % 8;
    write_map_[(position - buffer_start_) / 8] |= selector;
  }

  bool is_updated(int64_t position) {
    uint8_t bits = write_map_[(position - buffer_start_) / 8];
    uint8_t selector = 1 << (position - buffer_start_) % 8;
    uint8_t masked = bits & selector;
    return masked == selector;
  }

  // Does not change position_
  I read_from_buffer() {
    assert(position_ < end_);
    assert(start_ <= position_);

    int buffer_position = position_ - buffer_start_;

    if (0 <= buffer_position && buffer_position < buffer_size_
        && is_updated(position_)) {
      return buffer_[buffer_position];
    } else {
      refresh_buffer(true);

      return read_from_buffer();  // Should not cycle
    }
  }

  // Does not change position_
  void write_to_buffer(I value) {
    assert(position_ < end_);
    assert(start_ <= position_);

    int buffer_position = position_ - buffer_start_;

    if (0 <= buffer_position && buffer_position < buffer_size_) {
      buffer_[buffer_position] = value;
      update(position_);
    } else {
      refresh_buffer(false);

      write_to_buffer(value);  // Should not cycle
    }
  }

  void flush_buffer() {
    if (buffer_start_ < 0) return;  // Not even read.

    int writes_needed = 0;
    for (int64_t start = 0; start < utilized_buffer_size_;) {
      if (is_updated(start + buffer_start_)) {
        // 'end' is exclusive
        int64_t end = start + 1;
        while (end < utilized_buffer_size_ && is_updated(buffer_start_ + end)) {
          end++;
        }

        if (::lseek(fd, (buffer_start_ + start) * sizeof(I), SEEK_SET) == -1) {
          perror("Error seek");
          exit(1);
        }

        if (::write(fd, buffer_ + start, (end - start) * sizeof(I)) != (end - start) * sizeof(I)) {
          perror("Error writing");
          exit(1);
        }

        writes_needed++;

        start = end;
      } else {
        start++;
      }
    }

    // cout << "Writes needed: " << writes_needed << endl;
  }

  void refresh_buffer(bool need_read) {
    flush_buffer();

    // start count: 1065 end count: 10725 else count: 2922
    // if (position_ == buffer_start_ + utilized_buffer_size_ || position_ == start_)
    //   end_count++;
    // else if (position_ == buffer_start_ - 1 || position_ == end_ - 1)
    //   start_count++;
    // else
    //   else_count++;

    // cout << "start: " << buffer_start_ << " end: " << (buffer_start_ + utilized_buffer_size_) << " new pos: " << position_ << endl;
    // cout << "start count: " << start_count << " end count: " << end_count << " else count: " << else_count << endl;

    int64_t buffer_end;
    if (position_ == buffer_start_ + utilized_buffer_size_ || position_ == start_) {
      buffer_start_ = position_;
      buffer_end = min(buffer_start_ + buffer_size_, end_);
    } else if (position_ == buffer_start_ - 1 || position_ == end_ - 1) {
      buffer_start_ = max(position_ - buffer_size_ + 1, (int64_t)start_);
      buffer_end = min(buffer_start_ + buffer_size_, position_ + 1);
    } else {
      buffer_start_ = max(position_ - (buffer_size_ / 2), (int64_t)start_);
      buffer_end = min(buffer_start_ + buffer_size_, end_);
    }

    utilized_buffer_size_ = buffer_end - buffer_start_;

    if (need_read) {
      if (::lseek(fd, buffer_start_ * sizeof(I), SEEK_SET) == -1) {
        perror("Error seek");
        exit(1);
      }

      if (::read(fd, buffer_, utilized_buffer_size_ * sizeof(I)) != utilized_buffer_size_ * sizeof(I)) {
        perror("Error reading");
      }

      for (int64_t i = 0; i < utilized_buffer_size_; ++i) {
        update(buffer_start_ + i);
      }
      // for (int64_t i = 0; i < utilized_buffer_size_ / 8; ++i) {
      //   write_map_[i] = 255;
      // }
    } else {
      for (int64_t i = 0; i < (buffer_size_ + 7) / 8; ++i) {
        write_map_[i] = 0;
      }
    }
  }

  // TODO(lespeholt): delete
  static int64_t start_count;
  static int64_t end_count;
  static int64_t else_count;

  int64_t start_;
  int64_t end_;
  int fd;
  I* buffer_;
  uint8_t* write_map_;
  int64_t buffer_size_;
  int64_t utilized_buffer_size_;
  int64_t buffer_start_;
  int64_t position_;
};

template <typename I>
int64_t BufferedStream<I>::start_count = 0;
template <typename I>
int64_t BufferedStream<I>::end_count = 0;
template <typename I>
int64_t BufferedStream<I>::else_count = 0;