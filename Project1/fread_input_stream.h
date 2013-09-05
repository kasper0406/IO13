#pragma once

#include <string>
#include <cstdint>
#include <cstdio>

#include "input_stream.h"
#include "f_stream.h"

template <typename T>
class FREADInputStream : public FStream<T>, public InputStream<T> {
public:
  T read_next() {
    const int count = 1;
    this->remaining -= count;
    
    T result;
    size_t read = fread(&result, sizeof(T), count, this->m_file);
    if (read != count)
      throw runtime_error("Failed to read next entry from file (EOF or error).");
    
    return result;
  }
};
