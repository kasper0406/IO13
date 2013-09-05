#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include "utils.h"
#include "input_stream.h"
#include "output_stream.h"

void test_reads(uint32_t k, uint32_t n) {
  string filename = "test_file";
  uint64_t elements = 1024 * 1024;

  generate_file<uint32_t>(filename, random_uint32, elements);

  vector<InputStream<int32_t>> streams(k);

  for (int i = 0; i < k; ++i) {
    streams[i].open(filename, i * (elements / k));
  }
}