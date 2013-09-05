#pragma once

#include <string>
#include <cstdlib>

template <typename T>
class OutputStream {
  void open(string filename, uint position);
  write(T value);
  void close();
};