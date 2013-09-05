#pragma once

#include <string>
#include <cstdint>
#include <cstdio>

#include "input_stream.h"

template <class T>
class FREADInputStream : public InputStream<T> {
public:
  void open(string filename, uint32_t position) {
    m_file = fopen(filename, "rb");
    if (m_file == NULL)
      throw runtime_error("Failed to open file: " + filename);
    
    if (fseek(m_file, sizeof(T) * position, SEEK_SET) != 0)
      throw runtime_error("Failed to set position in file!");
  }
  
  T read_next() {
    const int count = 1;
    
    T result;
    size_t read = fread(&result, sizeof(T), count, m_file);
    if (read != count)
      throw runtime_error("Failed to read next entry from file (EOF or error).");
    
    return result;
  }
  
  bool end_of_stream() const {
    return feof(m_file) != 0;
  }
  
  void close() {
    if (m_file != NULL)
      fclose(m_file);
  }
  
private:
  FILE* m_file;
};
