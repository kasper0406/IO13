#pragma once

#include <string>
#include <cstdint>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "input_stream.h"
#include "mmap_stream.h"

using namespace std;

template<uint64_t B, typename T>
class MMapInputStream : public InputStream<T>, public MMapStream<B, T> {
public:
  void open(string filename, uint64_t start, uint64_t end) {
    MMapStream<B, T>::open(filename, start, end, Stream<T>::IN);
  }
  
  T read_next() {
    this->remaining -= 1;
    
    if (this->current - B == this->memory)
      this->remap();
    
    T result = *this->current;
    this->current += 1;
    
    return result;
  }
};
