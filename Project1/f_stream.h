#pragma once

#include <string>
#include <cstdint>
#include <cstdio>

#include "stream.h"

template <typename T>
class FStream : public virtual Stream<T> {
public:
  void open(string filename, uint64_t start, uint64_t end) {
    remaining = end - start;
    
    m_file = fopen(filename.c_str(), "rb");
    if (m_file == NULL)
      throw runtime_error("Failed to open file: " + filename);
    
    if (fseek(m_file, sizeof(T) * start, SEEK_SET) != 0)
      throw runtime_error("Failed to set position in file!");
  }
  
  bool end_of_stream() const {
    return remaining == 0;
  }
  
  void close() {
    if (m_file != NULL)
      fclose(m_file);
  }
  
protected:
  FILE* m_file;
  uint32_t remaining;
};
