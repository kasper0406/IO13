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
    filename_ = filename;
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
  
  void closeAndRemove() {
    MMapInputStream<B, T>::close();
    if (filename_ != "tmp" && remove(this->filename_.c_str()) != 0) {
      throw runtime_error("did not delete " + this->filename_ + " foo");
    }
  }
private:
  string filename_;
};
