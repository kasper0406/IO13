#pragma once

#include <functional>
#include <random>
#include <cstdint>
#include <string>
#include <numeric>
#include <fstream>
#include <iostream>

using namespace std;

mt19937 generator(100);
uniform_int_distribution<uint32_t> distribution(numeric_limits<uint32_t>::min(),
                                                numeric_limits<uint32_t>::max());
auto random_uint32 = bind(distribution, generator);

template <typename T>
void generate_file(string filename, function<T()> generator, uint64_t n) {
  ofstream fs(filename);

  if (!fs) {
    cerr << "Failed to open file \"" << filename << "\" for write";
    exit(1);
  }

  for (uint64_t i = 0; i < n; ++i) {
    T value = generator();
    fs.write((char*)&value, sizeof(T));
  }

  fs.close();
}