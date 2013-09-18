#pragma once

#include <string>
#include <cstdint>

using namespace std;

uint64_t counter = 0;

template <typename T>
class Stream
{
public:
  enum Direction { IN, OUT };
  
  /**
   * Opens file 'filename' and reads entries in the interval [start, end[.
   */
  void open(string filename, uint64_t start, uint64_t end);
  bool end_of_stream();
  void close() const;

  typedef T type;
};
