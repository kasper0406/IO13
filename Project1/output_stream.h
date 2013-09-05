#pragma once

#include <string>
#include <cstdint>

using namespace std;

template <typename T>
class OutputStream {
public:
  void open(string filename, uint64_t position);
  void write(T value);
  void close();
};