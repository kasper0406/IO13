#pragma once

#include <string>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

#include "input_stream.h"
#include "f_stream.h"

template <typename T>
class FREADInputStream : public FStream<T>, public InputStream<T> {
public:
  void open(string filename, uint64_t start, uint64_t end) {
    FStream<T>::open(filename, start, end, FStream<T>::IN);
  }
  
  T read_next() {
    const int count = 1;
    this->remaining -= count;
    
    T result;
    size_t read = fread(&result, sizeof(T), count, this->m_file);
    if (read != count)
      throw runtime_error("Failed to read next entry from file (EOF or error).");
    
    return result;
  }
  uint64_t remaining;
};
