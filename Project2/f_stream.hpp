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
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    end_ = end;
    start_ = start;

    pFile = fopen(filename().c_str(), "w+");

    if (pFile == nullptr) {
      throw logic_error("Failed to open file");
    }

    if (_fseeki64(pFile, start * sizeof(I), SEEK_SET) != 0) {
      throw logic_error("Failed to seek");
    }
  }

  I peek() {
    I element;
    if (fread(&element, sizeof(I), 1, pFile) != 1) {
      throw logic_error("Failed to read from file");
    }
    _fseeki64(pFile, -sizeof(I), SEEK_CUR);

    return element;
  }
  
  I read_next() {
    I element;
    if (fread(&element, sizeof(I), 1, pFile) != 1) {
      throw logic_error("Failed to read from file");
    }

    return element;
  }
  
  void write(I value) {
    if (fwrite(&value, sizeof(I), 1, pFile) != 1) {
      throw logic_error("Failed to write");
    }
  }
  void close() {
    if (fclose(pFile) != 0) {
      throw logic_error("Failed to close file");
    }
  }
  void seek(uint64_t position) {
    if (_fseeki64(pFile, position * sizeof(I), SEEK_SET) != 0) {
      throw logic_error("Seek failed");
    }
  }
  
  bool has_next() {
    return ftell(pFile) / sizeof(I) - 1 < end_;
  }

private:
  uint64_t start_;
  uint64_t end_;
  FILE* pFile;
};
