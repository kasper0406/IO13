#pragma once

#include <string>
#include <cstdint>
#include <cstdio>
#include <stdexcept>

#include "stream.h"

template <typename T>
class FStream : public virtual Stream<T> {
public:
  FStream() {
    m_file = NULL;
    remaining = 0;
  }
  
  void open(string filename, uint64_t start, uint64_t end) {
    counter++;
    
    elements = end - start;
    remaining = elements;
    
#ifndef _WINDOWS
    if (fseek(m_file, sizeof(T) * start, SEEK_SET) != 0)
      throw runtime_error("Failed to set position in file!");
#else
    if (_fseeki64(m_file, sizeof(T) * start, SEEK_SET) != 0)
      throw runtime_error("Failed to set position in file!");
#endif
  }
  
  bool end_of_stream() const {
    return remaining == 0;
  }
  
  void close() {
    if (m_file != NULL) {
      fclose(m_file);
      counter--;
    }
  }
  
  uint64_t size() const {
    return elements;
  }
  
protected:
  FILE* m_file;
  uint64_t remaining;
  uint64_t elements;
};
