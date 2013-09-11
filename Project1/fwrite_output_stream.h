#pragma once

#include <stdexcept>

#include "output_stream.h"
#include "f_stream.h"

template<typename T>
class FWRITEOutputStream : public FStream<T>, public OutputStream<T> {
public:
  void open(string filename, uint64_t start, uint64_t end) {
    // Create the file if it doesn't exist
    this->m_file = fopen(filename.c_str(), "a+b");
    if (this->m_file == NULL)
      throw runtime_error("Failed to create file " + filename);
    
    // Open the file for writing
    this->m_file = freopen(NULL, "r+b", this->m_file);
    if (this->m_file == NULL)
      throw runtime_error("Failed to change file mode to writing!");
    
    // Make sure the file is large enough
    if (fseek(this->m_file, 0, SEEK_END) != 0)
      throw runtime_error("Failed to go to end of file!");
    if (ftell(this->m_file) < sizeof(T) * end) {
      const int val = 0;
      
      if (fseek(this->m_file, sizeof(T) * end - sizeof(int), SEEK_SET) != 0)
        throw runtime_error("Failed to seek to new end of file!");
      
      if (fwrite(&val, sizeof(int), 1, this->m_file) != 1)
        throw runtime_error("Failed to extend file size!");
    }
    
    FStream<T>::open(filename, start, end, Stream<T>::OUT);
  }
  
  void write(T value) {
    const int count = 1;
    
    if (this->remaining == 0)
      throw runtime_error("Tried to write to full output stream!");
    this->remaining -= count;
    
    if (fwrite(&value, sizeof(T), count, this->m_file) != count)
      throw runtime_error("Failed to write element to output stream!");
  }
};
