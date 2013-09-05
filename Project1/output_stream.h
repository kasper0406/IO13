#pragma once

#include <string>
#include <cstdint>

using namespace std;

template <typename T>
class OutputStream {
public:
  void open(string filename, uint64_t start, uint64_t end);
  void write(T value);
  void close();
};