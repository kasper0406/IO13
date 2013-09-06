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
    nextBlock = start;
    remaining = end - start;
    
    fd = ::open(filename.c_str(), O_RDONLY);
    if (fd == -1)
      throw runtime_error("Failed to open file!");
    
    remap();
  }
  
  T read_next() {
    remaining -= 1;
    
    if (current - B == memory)
      remap();
    
    T result = *current;
    current += 1;
    
    return result;
  }
  
  bool end_of_stream() const {
    return remaining == 0;
  }
  
  void close() {
    if (memory != NULL)
      munmap(memory, B * sizeof(T));
  }

private:
  T* memory;
  T* current;
  int fd;
  uint64_t remaining;
  uint64_t nextBlock;
  
  void remap() {
    close();
    
    // Align the offset to a multiple of page size
    uint64_t offset = nextBlock * sizeof(T);
    const uint64_t offWrtPageSize = offset % sysconf(_SC_PAGESIZE);
    if (offWrtPageSize != 0)
      offset -= offWrtPageSize;
    
    memory = (T*) mmap(NULL, B * sizeof(T), PROT_READ, MAP_SHARED, fd, offset);
    if (memory == MAP_FAILED) {
      memory = NULL;
      throw runtime_error("Failed to map memory!");
    }
    
    // Skip the elements loaded in to satisfy the "offset should be multiple of page size"-restriction.
    current = memory + offWrtPageSize / sizeof(T);
    
    nextBlock += B;
  }
};
