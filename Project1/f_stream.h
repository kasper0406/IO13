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
  
  enum Direction { IN, OUT };
  
  void open(string filename, uint64_t start, uint64_t end, Direction direction) {
    remaining = end - start;
    
    string dir;
    if (direction == IN)
      dir = "r";
    else
      dir = "r+";
    
    m_file = fopen(filename.c_str(), (dir + "b").c_str());
    if (m_file == NULL)
      throw runtime_error("Failed to open file: " + filename);
    
#ifndef _WINDOWS
    if (fseek64(m_file, sizeof(T) * start, SEEK_SET) != 0)
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
    if (m_file != NULL)
      fclose(m_file);
  }
  
protected:
  FILE* m_file;
  uint64_t remaining;
};