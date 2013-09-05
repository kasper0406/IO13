#pragma once

#include <cstdint>
#include <vector>

#include "input_stream.h"
#include "output_stream.h"

void test_reads(uint32_t k, uint32_t n) {
  vector<InputStream<int32_t>> streams(k);

  for (int i = 0; i < k; ++i) {
    streams[i].open(
  }
}