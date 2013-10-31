#pragma once

#include <cstdint>
#include <cassert>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

template <typename I>
class MMapStream {
public:
  MMapStream() : fd_(-1), mapped_(nullptr) { }
  
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    position_ = 0;
    end_ = end;
    start_ = start;
    
    // Open file. It is ensured by ExternalHeap that the file is big enough.
    fd_ = ::open(filename.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_ == -1)
      throw runtime_error("Failed to open file!");
    
    // MMap file into memory
    const uint64_t elements = end_ - start_;
    const uint64_t offset = start_ * sizeof(I);
    
    assert(offset % sysconf(_SC_PAGE_SIZE) == 0); // Offset should be multiple of page size
    mapped_ = (I*) mmap(NULL, sizeof(I) * elements, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, offset);
    if (mapped_ == MAP_FAILED)
      throw runtime_error("Failed to map file!");
  }
  
  I peek() {
    assert(position_ < end_ - start_);
    return mapped_[position_];
  }
  
  I read_next() {
    I res = peek();
    position_++;
    return res;
  }
  
  void write(I value) {
    assert(position_ < end_ - start_);
    mapped_[position_++] = value;
  }
  
  void close() {
    if (mapped_ != nullptr) {
      const uint64_t elements = end_ - start_;
      if (munmap(mapped_, sizeof(I) * elements) == -1)
        throw runtime_error("Failed to unmap file!");
    }
    
    if (fd_ != -1) {
      if (::close(fd_) == -1)
        throw runtime_error("Failed to close file!");
    }
  }
  
  void seek(uint64_t position) {
    assert(position >= start_);
    assert(position <= end_);
    position_ = position - start_;
  }
  
  bool has_next() {
    return position_ < end_ - start_;
  }
  
  static void cleanup() { }
  
private:
  uint64_t position_;
  uint64_t start_;
  uint64_t end_;
  
  int fd_;
  I* mapped_;
};
