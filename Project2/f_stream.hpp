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
  FStream() : pFile(nullptr) {}
  
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    end_ = end;
    start_ = start;

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
  }
  
  bool has_next() {
    auto pos = ftell(pFile);
    auto value = pos / (long)sizeof(I);
    return value < end_;
  }

private:
  uint64_t start_;
  uint64_t end_;
  FILE* pFile;
};
