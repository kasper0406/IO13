#pragma once

#include <string>
#include <fcntl.h>
#include <cstdio>
#include <cstdint>
#include <cassert>

template <typename I>
class SysStream {
public:
  SysStream() : fd(-1) {}
  
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
    if (fread(&element, sizeof(I), 1, pFile) != 1) {
      perror("Error peek");
      exit(1);
    }
    _fseeki64(pFile, -sizeof(I), SEEK_CUR);

    return element;
  }
  
  I read_next() {
    I element;
    if (fread(&element, sizeof(I), 1, pFile) != 1) {
      perror("Error reading");
      exit(1);
    }

    return element;
  }
  
  void write(I value) {
    if (fwrite(&value, sizeof(I), 1, pFile) != 1) {
      perror("Error writing");
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
    if (_fseeki64(pFile, position * sizeof(I), SEEK_SET) != 0) {
      perror("Error seek");
      exit(1);
    }
  }
  
  bool has_next() {
    auto pos = ftell(pFile);
    auto value = pos / (long)sizeof(I);
    return value < end_;
  }

private:
  uint64_t start_;
  uint64_t end_;
  ind fd;
};