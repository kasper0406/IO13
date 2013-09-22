#pragma once

#include <functional>
#include <random>
#include <cstdint>
#include <string>
#include <numeric>
#include <fstream>
#include <iostream>

using namespace std;

minstd_rand generator(100);
uniform_int_distribution<uint32_t> distribution(numeric_limits<uint32_t>::min(),
                                                100); // numeric_limits<uint32_t>::max());
auto random_uint32 = bind(distribution, generator);

template <typename T>
void generate_file(string filename, function<T()> generator, uint64_t n) {
  ofstream fs(filename);

  if (!fs) {
    cerr << "Failed to open file \"" << filename << "\" for write";
    exit(1);
  }

  const uint64_t bufferSize = 1024;
  vector<T> buffer(bufferSize);
  for (uint64_t i = 0; i < n; ++i) {
    buffer[i % bufferSize] = rand(); // i * 5 % 37 + n * i; // generator();
    if (i % bufferSize == bufferSize - 1)
      fs.write((char*)&buffer[0], sizeof(T) * bufferSize);
  }
  fs.write((char*)&buffer[0], sizeof(T) * (n % bufferSize));

  fs.close();
}