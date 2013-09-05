#pragma once

#include <string>
#include <cstdint>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "input_stream.h"

template<uint64_t B, typename T>
class MMapInputStream : public InputStream<T> {
public:
  MMapInputStream() : memory(NULL), remaining(0) {
  }
  
  void open(string filename, uint64_t start, uint64_t end) {
    remaining = end - start;
    
    int fd = open(filename.c_str(), O_RDONLY, 0);
    if (fd == -1)
      throw runtime_error("Failed to open file!");
    
    if (((start * sizeof(T)) % sysconf(_SC_PAGESIZE)) != 0)
      throw runtime_error("Start address should be multiple of page size!");
    
    memory = mmap(NULL, B * sizeof(T), PROT_READ, MAP_SHARED, fd, start * sizeof(T));
    if (memory == MAP_FAILED) {
      memory = NULL;
      throw runtime_error("Failed to map memory!");
    }
    
    current = memory;
  }
  
  T read_next() {
    remaining -= 1;
    
    if (current == memory + B * sizeof(T)) {
      // Remap
    }
    
    return *current;
  }
  
  bool end_of_stream() const {
    return remaining == 0;
  }
  
  void close() {
    if (memory != NULL)
      munmap(memory, length);
  }

private:
  void* memory;
  T* current;
  
  uint64_t length;
  
  uint64_t remaining;
};
