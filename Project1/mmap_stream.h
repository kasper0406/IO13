#pragma once

#include <string>
#include <cstdint>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

#include "stream.h"

using namespace std;

template<uint64_t B, typename T>
class MMapStream : public virtual Stream<T> {
public:
  MMapStream() : memory(NULL), fd(-1), remaining(0) {
  }
  
  void open(string filename, uint64_t start, uint64_t end, typename Stream<T>::Direction direction) {
    this->direction = direction;
    nextBlock = start;
    elements = end - start;
    remaining = elements;
  }
  
  bool end_of_stream() const {
    return remaining == 0;
  }
  
  void close() {
    if (memory != NULL) {
      if (munmap(memory, B * sizeof(T)) == -1) {
        throw runtime_error("Munmap failed");
      }
    }
    
    if (fd != -1) {
      if (::close(fd) == -1) {
        throw runtime_error("Could not close stream");
      }
    }

    memory = nullptr;
    fd = -1;
    remaining = 0;
  }
  
  uint64_t size() const {
    return elements;
  }
  
protected:
  T* memory;
  T* current;
  int fd;
  uint64_t remaining;
  uint64_t nextBlock;
  typename Stream<T>::Direction direction;
  uint64_t elements;
  uint64_t oldOffWrtPageSize;
  
  bool needsRemap() const {
    return this->memory == NULL || this->current - this->memory >= B + this->oldOffWrtPageSize / sizeof(T);
  }
  
  void remap() {
    if (memory != NULL) {
      if (munmap(memory, B * sizeof(T) + oldOffWrtPageSize) == -1) {
        throw runtime_error("Munmap 2 failed");
      }
    }
    
    // Align the offset to a multiple of page size
    uint64_t offset = nextBlock * sizeof(T);
    const uint64_t offWrtPageSize = offset % sysconf(_SC_PAGESIZE);
    if (offWrtPageSize != 0)
      offset -= offWrtPageSize;
    
    int prot;
    if (direction == Stream<T>::Direction::IN)
      prot = PROT_READ;
    else
      prot = PROT_WRITE;
    
    memory = (T*) mmap(NULL, B * sizeof(T) + offWrtPageSize, prot, MAP_SHARED, fd, offset);
    if (memory == MAP_FAILED) {
      memory = NULL;
      throw runtime_error("Failed to map memory: " + to_string(errno));
    }
    
    // Skip the elements loaded in to satisfy the "offset should be multiple of page size"-restriction.
    current = memory + offWrtPageSize / sizeof(T);
    
    oldOffWrtPageSize = offWrtPageSize;
    nextBlock += B;
  }
};
