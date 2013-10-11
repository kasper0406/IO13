#pragma once

#include <array>
#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <sstream>

#include "block.hpp"

using namespace std;

const char* filename = "heap";

template <class S, typename I, uint64_t d>
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

      blocks_.push_back(Block<S,I,d>(buffer_size_ * blocks_.size(), buffer_size_ * (blocks_.size() + 1), this));
      blocks_.back().open_front();
      
      while (!insert_buffer_.empty()) {
        // Pop element in insert buffer
        pop_heap(insert_buffer_.begin(), insert_buffer_.end());
        I element = insert_buffer_.back();
        insert_buffer_.pop_back();
        blocks_.back().write_inc(element);
      }

      blocks_.back().close();

      // Special case: Former last leaf imperfect?
      if (!blocks_.back().root() && blocks_[blocks_.size() - 2].imperfect()) {
        // TODO(lespeholt): Swap and sift. Does not make sense to make until 'extract_max' is done.
        assert(false);
      } else {
        blocks_.back().recursive_sift();
      }
    }

    // Push element in insert buffer
    insert_buffer_.push_back(element);
    push_heap(insert_buffer_.begin(), insert_buffer_.end());
  }

  I peek_max();
  
  void extract_max() {
    // TODO(knielsen): Keep some elements in the root buffered.
    
    bool biggest_in_insert_buffer = true;
    if (!insert_buffer_.empty() && !blocks_.empty()) {
      blocks_[0].open_at_first_element();
      biggest_in_insert_buffer = insert_buffer_.front() >= blocks_[0].peek();
      blocks_[0].close();
    }
    
    if (!biggest_in_insert_buffer) {
      // Biggest is in root element
      blocks_[0].open_at_first_element();
      blocks_[0].read_dec();
      blocks_[0].close();
      
      if (blocks_[0].imperfect()) {
        if (blocks_[0].element_count() == 0) {
          // The heap is empty
          blocks_.pop_back();
        } else {
          // Refill
          blocks_[0].refill();
        }
      }
    } else if (!insert_buffer_.empty()) {
      // Biggest is in insert buffer
      pop_heap(insert_buffer_.begin(), insert_buffer_.end());
      insert_buffer_.pop_back();
    } else {
      // The heap is empty!
      throw logic_error("Trying to extract from empty heap!");
    }
  }
  
  size_t stream_buffer_size() const {
    // TODO(lespeholt): We probably want a separate stream buffer size 
    return buffer_size_;
  }

  vector<Block<S, I, d>>& blocks() {
    return blocks_;
  }
  
  uint64_t pos(Block<S,I,d>* block) {
    return block - &blocks_[0];
  }
  
  string to_dot() {
    stringstream ss;
    ss << "digraph foo {" << endl;
    
    if (!blocks_.empty())
      blocks_[0].to_dot(ss);
    
    ss << "}" << endl;
    return ss.str();
  }

private:
  size_t buffer_size_;
  vector<I> insert_buffer_;
  vector<Block<S, I, d>> blocks_;
};