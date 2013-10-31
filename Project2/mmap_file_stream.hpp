#pragma once

#include <cstdint>
#include <cassert>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;

template <typename I>
class MMapFileStream {
public:
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    position_ = 0;
    end_ = end;
    start_ = start;
    
    const uint64_t elements_in_file = size_of_mapped_ / sizeof(I);
    if (elements_in_file < end) {
      map_file(filename);
      assert(size_of_mapped_ >= end * sizeof(I));
    }
  }
  
  I peek() {
    assert(position_ < end_ - start_);
    return mapped_[start_ + position_];
  }
  
  I read_next() {
    I res = peek();
    position_++;
    return res;
  }
  
  void write(I value) {
    assert(position_ < end_ - start_);
    mapped_[start_ + position_++] = value;
  }
  
  void close() {
  }
  
  void seek(uint64_t position) {
    assert(position >= start_);
    assert(position < end_);
    position_ = position - start_;
  }
  
  bool has_next() {
    return position_ < end_ - start_;
  }
  
  /**
   * Free up all the static resources!
   */
  static void cleanup() {
    if (mapped_ != nullptr) {
      if (munmap(mapped_, size_of_mapped_) == -1)
        throw runtime_error("Failed to unmap mapped memory.");
      mapped_ = nullptr;
      size_of_mapped_ = 0;
    }
    
    if (fd_ != -1) {
      if (::close(fd_) == -1)
        throw runtime_error("Failed to close file.");
      fd_ = -1;
    }
  }
  
private:
  void map_file(string filename) {
    if (mapped_ != nullptr) {
      // Unmap already mapped memory
      if (munmap(mapped_, size_of_mapped_) == -1)
        throw runtime_error("Failed to unmap mapped memory.");
    }
    
    if (fd_ == -1) {
      // Open the file for MMap
      fd_ = ::open(filename.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
      if (fd_ == -1)
        throw runtime_error("Failed to open file!");
    }
    
    // Get the size of the file
    size_of_mapped_ = lseek(fd_, 0, SEEK_END);
    if (size_of_mapped_ == -1)
      throw runtime_error("Failed to obtain the size of the file");
    
    // Map the entire file
    mapped_ = (I*) mmap(NULL, size_of_mapped_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (mapped_ == MAP_FAILED)
      throw runtime_error("Failed to map file!");
  }
  
  uint64_t position_;
  uint64_t start_;
  uint64_t end_;
  
  static int fd_;
  static size_t size_of_mapped_;
  static I* mapped_;
};

template<class I> int MMapFileStream<I>::fd_ = -1;
template<class I> size_t MMapFileStream<I>::size_of_mapped_ = 0;
template<class I> I* MMapFileStream<I>::mapped_ = nullptr;
