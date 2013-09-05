#pragma once

#include <string>
#include <cstdint>

using namespace std;

template <typename T>
class InputStream {
  void open(string filename, uint64_t position);
  T read_next();
  bool end_of_stream();
  void close();
};