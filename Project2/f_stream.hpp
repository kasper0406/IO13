#pragma once

#include <string>
#include <fcntl.h>
#include <cstdio>
#include <cstdint>
#include <cassert>

#ifdef WIN32
  #define _fseeki64 _fseeki64
#else
  #define _fseeki64 fseek
#endif

template <typename I>
class FStream {
public:
  FStream(uint64_t cache_size) : pFile(nullptr) {}
  
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    end_ = end;
    start_ = start;
    position_ = start;

    pFile = fopen(filename.c_str(), "r+b");

    if (pFile == nullptr) {
      perror("Error open file");
      exit(1);
    }

    if (_fseeki64(pFile, start * sizeof(I), SEEK_SET) != 0) {
      perror("Error seek in open");
      exit(1);
    }
  }

  I peek() {
    assert(start_ <= position_ && position_ < end_);
    I element;
    if (fread(&element, sizeof(I), 1, pFile) != 1) {
      perror("Error peek");
      exit(1);
    }
    _fseeki64(pFile, -sizeof(I), SEEK_CUR);

    return element;
  }
  
  I read_next() {
    assert(start_ <= position_ && position_ < end_);
    I element;
    if (fread(&element, sizeof(I), 1, pFile) != 1) {
      perror("Error reading");
      exit(1);
    }
    position_++;

    return element;
  }

  I read_prev() {
    assert(start_ <= position_ && position_ < end_);
    I result = read_next();
    if (position_ != 1 && _fseeki64(pFile, -2 * sizeof(I), SEEK_CUR) != 0) {
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
    assert(start_ <= position_ && position_ < end_);
    if (fwrite(&value, sizeof(I), 1, pFile) != 1) {
      perror("Error writing");
      exit(1);
    }
    position_++;
  }

  void backward_write(I value) {
    assert(start_ <= position_ && position_ < end_);
    write(value);
    if (position_ != 1 && _fseeki64(pFile, -2 * sizeof(I), SEEK_CUR) != 0) {
      perror("Error seek");
      exit(1);
    }
    position_-=2;
  }
  
  void close() {
    if (fclose(pFile) != 0) {
      perror("Error closing");
      exit(1);
    }
    pFile = nullptr;
  }
  
  void seek(uint64_t position) {
    if (_fseeki64(pFile, position * sizeof(I), SEEK_SET) != 0) {
      perror("Error seek");
      exit(1);
    }
    position_ = position;
  }
  
  bool has_next() {
    return position() < end_;
  }
  
  static void cleanup() { }

private:
  uint64_t start_;
  uint64_t end_;
  int64_t position_;
  FILE* pFile;
};
