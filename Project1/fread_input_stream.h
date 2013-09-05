#pragma once

#include <string>
#include <cstdint>
#include <cstdio>

#include "input_stream.h"

template <typename T>
class FREADInputStream : public InputStream<T> {
public:
  void open(string filename, uint64_t start, uint64_t end) {
    remaining = end - start;
    
    m_file = fopen(filename.c_str(), "rb");
    if (m_file == NULL)
      throw runtime_error("Failed to open file: " + filename);
    
    if (_fseeki64(m_file, sizeof(T) * start, SEEK_SET) != 0)
      throw runtime_error("Failed to set position in file!");
  }
  
  T read_next() {
    const int count = 1;
    remaining -= count;
    
    T result;
    size_t read = fread(&result, sizeof(T), count, m_file);
    if (read != count)
      throw runtime_error("Failed to read next entry from file (EOF or error).");
    
    return result;
  }
  
  bool end_of_stream() const {
    return remaining == 0;
  }
  
  void close() {
    if (m_file != NULL)
      fclose(m_file);
  }
  
private:
  FILE* m_file;
  uint64_t remaining;
};
