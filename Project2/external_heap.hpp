#pragma once

#include <array>
#include <string>
#include <vector>
#include <queue>

#include "block.hpp"

using namespace std;

const char* filename = "heap";

template <class S, typename I>
class ExternalHeap {
public:
  ExternalHeap(size_t buffer_size) : insert_buffer_(buffer_size) {
  }

  void insert(I element);
  I peek_max();
  void extract_max();

private:
  vector<I> insert_buffer_;
  vector<Block<S, I>> blocks_;
};