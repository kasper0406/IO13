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
  ExternalHeap(size_t buffer_size) : buffer_size_(buffer_size) {
    // With this, the capacity changes to *at least* 'buffer_size'.
    // To make sure we use precisely a buffer of size 'buffer_size'
    // 'buffer_size_' is set (instead of using insert_buffer_.capacity().)
    insert_buffer_.reserve(buffer_size);
  }

  void insert(I element) {
    if (insert_buffer_.size() >= buffer_size_) {
      // Inserts new block at the end, opens its stream and store the insert buffer in sorted order (descending)

      blocks_.push_back(Block<S,I>(buffer_size_ * blocks_.size(), buffer_size_ * (blocks_.size() + 1), this));
      blocks_.back().open_front();
      
      while (!insert_buffer_.empty()) {
        // Pop element in insert buffer
        pop_heap(insert_buffer_.begin(), insert_buffer_.end());
        I element = insert_buffer_.back();
        insert_buffer_.pop_back();
        blocks_.back().write_inc(element);
      }

      blocks_.back().close();

      // TODO(lespeholt): Sift, etc.
    }

    // Push element in insert buffer
    insert_buffer_.push_back(element);
    push_heap(insert_buffer_.begin(), insert_buffer_.end());
  }

  I peek_max();
  void extract_max();
  size_t stream_buffer_size() const {
    // TODO(lespeholt): We probably want a separate stream buffer size 
    return buffer_size_;
  }

private:
  size_t buffer_size_;
  vector<I> insert_buffer_;
  vector<Block<S, I>> blocks_;
};