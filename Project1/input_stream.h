#pragma once

#include <string>
#include <cstdint>

using namespace std;

template <typename T>
class InputStream {
public:
  /**
   * Opens file 'filename' and reads entries in the interval [start, end[.
   */
  void open(string filename, uint64_t start, uint64_t end);
  
  T read_next();
  bool end_of_stream();
  void close();
};