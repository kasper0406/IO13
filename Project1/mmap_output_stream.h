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
    this->fd = ::open(filename.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (this->fd == -1)
      throw runtime_error("Failed to open file!");
    
    // Grow file if necessary
    const int64_t size = lseek(this->fd, 0, SEEK_END);
    if (size == -1)
      throw runtime_error("Failed to go to end of file.");
    
    if (size < sizeof(T) * end) {
      // Grow file
      const int val = 0;
      
      if (lseek(this->fd, sizeof(T) * end - sizeof(int), SEEK_SET) == -1)
        throw runtime_error("Failed to seek to new file end;");
      
      if (::write(this->fd, &val, sizeof(int)) != sizeof(int))
        throw runtime_error("Failed to extend file!");
    }
    
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
