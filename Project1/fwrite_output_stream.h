#pragma once

#include <stdexcept>

#include "output_stream.h"
#include "f_stream.h"

template<typename T>
class FWRITEOutputStream : public FStream<T>, public OutputStream<T> {
public:
  void open(string filename, uint64_t start, uint64_t end) {
    FStream<T>::open(filename, start, end, Stream<T>::OUT);
  }
  
  void write(T value) {
    const int count = 1;
    
    if (this->remaining == 0)
      throw runtime_error("Tried to write to full output stream!");
    this->remaining -= count;
    
    if (fwrite(&value, sizeof(T), count, this->m_file) != count)
      throw runtime_error("Failed to write element to output stream!");
  }
};
