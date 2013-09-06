#pragma once

#include <string>
#include <cstdint>

#include "mmap_stream.h"
#include "stream.h"
#include "mmap_stream.h"

using namespace std;

template <uint64_t B, typename T>
class MMapOutputStream : public MMapStream<B, T>, public OutputStream<T> {
public:
  void open(string filename, uint64_t start, uint64_t end) {
    MMapStream<B, T>::open(filename, start, end, Stream<T>::OUT);
  }
  
  void write(T value) {
    const int count = 1;
    
    if (this->remaining == 0)
      throw runtime_error("Tried to write to full output stream!");
    this->remaining -= count;

    if (this->current - B == this->memory)
      this->remap();
    
    *this->current = value;
    this->current += 1;
  }
};
