#pragma once

#include <string>
#include <cstdlib>

template <typename T>
class InputStream {
  void open(string filename, uint position);
  T read_next();
  bool end_of_stream();
  void close();
};