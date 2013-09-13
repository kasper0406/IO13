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
    this->fd = ::open(filename.c_str(), O_RDONLY);
    if (this->fd == -1)
      throw runtime_error("Failed to open file!");
    
    MMapStream<B, T>::open(filename, start, end, Stream<T>::IN);
  }
  
  T read_next() {
    if (this->remaining == 0)
      throw runtime_error("Trying to read from empty stream!");
    
    if (this->needsRemap())
      this->remap();
    
    this->remaining--;
    
    T result = *this->current;
    this->current += 1;
    
    return result;
  }
};
