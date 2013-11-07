#pragma once

#include <array>
#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <limits>
#include <sstream>

#include "block.hpp"
#include <string>

using namespace std;

#ifdef WIN32
  #define _fseeki64 _fseeki64
#else
  #define _fseeki64 fseek
#endif

template <class S, typename I>
class ExternalHeap {
public:
  ExternalHeap(string filename, size_t buffer_size, size_t stream_buffer_size, size_t d, size_t stream_cache_size)
      : size_(0), buffer_size_(buffer_size), stream_buffer_size_(stream_buffer_size), stream_cache_size_(stream_cache_size), d_(d) {
    // With this, the capacity changes to *at least* 'buffer_size'.
    // To make sure we use precisely a buffer of size 'buffer_size'
    // 'buffer_size_' is set (instead of using insert_buffer_.capacity().)
    insert_buffer_.reserve(buffer_size);
    filename_ = filename;
  }
  
  ~ExternalHeap() {
    S::cleanup(); // Required for MMapFileStream.
  }

  void insert(I element, bool sift = true) {
    size_++;
    
    if (insert_buffer_.size() >= buffer_size_) {
      // Inserts new block at the end, opens its stream and store the insert buffer in sorted order (descending)

      // Resize file
      add_block_to_file();

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

      // Special case: Former last leaf imperfect?
      if (!blocks_.back().root() && blocks_[blocks_.size() - 2].imperfect()) {
        swap(blocks_.back(), blocks_[blocks_.size() - 2]);
        if (sift) {
          blocks_.back().sift();
          blocks_[blocks_.size() - 2].sift();
        }
      } else {
        if (sift) {
          blocks_.back().sift();
        }
      }
      
      // consistency_check();
    }

    // Push element in insert buffer
    insert_buffer_.push_back(element);
    push_heap(insert_buffer_.begin(), insert_buffer_.end());
  }

  I peek_max() {
    I insert_buffer_candidate = numeric_limits<I>::min();
    I root_candidate = numeric_limits<I>::min();

    if (insert_buffer_.empty() && blocks_.empty()) {
      // The heap is empty!
      throw logic_error("Trying to peek in empty heap!");
    }

    if (!insert_buffer_.empty()) {
      insert_buffer_candidate = insert_buffer_.front();
    }

    if (!blocks_.empty()) {
      blocks_[0].open_at_first_element();
      root_candidate = blocks_[0].peek();
      blocks_[0].close();
    }

    return max(insert_buffer_candidate, root_candidate);
  }
  
  void extract_max() {
    if (insert_buffer_.empty() && blocks_.empty()) {
      // The heap is empty!
      throw logic_error("Trying to extract from empty heap!");
    }

    size_--;
    
    if (!insert_buffer_.empty() && insert_buffer_.front() == peek_max()) {
      // Biggest is in insert buffer
      pop_heap(insert_buffer_.begin(), insert_buffer_.end());
      insert_buffer_.pop_back();
    } else {
      // Biggest is in root element
      blocks_[0].just_dec();
      
      if (blocks_[0].imperfect()) {
        if (blocks_[0].element_count() == 0) {
          // The heap is empty
          blocks_.pop_back();
        } else {
          // Refill
          blocks_[0].refill();
        }
      }
    }
    
    consistency_check();
  }
  
  size_t stream_buffer_size() const {
    return stream_buffer_size_;
  }

  size_t stream_cache_size() const {
    return stream_cache_size_;
  }
  
  vector<Block<S, I>>& blocks() {
    return blocks_;
  }

  size_t d() const {
    return d_;
  }

  const string& filename() const { 
    return filename_;
  }
  
  Block<S, I>* last_block() {
    return &blocks_[blocks_.size() - 1];
  }
  
  uint64_t pos(Block<S,I>* block) {
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

  void consistency_check() {
    if (!blocks_.empty())
      blocks_[0].consistency_check();
  }
  
  size_t size() const {
    return size_;
  }
  
  bool empty() const {
    return size() == 0;
  }
  
private:
  void add_block_to_file() {
    FILE* pFile = fopen(filename().c_str(), blocks().size() == 0 ? "w+" : "r+");
    if (pFile == nullptr) {
      cout << "Could not add block to " << filename().c_str() << endl;
      exit(1);
    }

    if (_fseeki64(pFile, (blocks().size() + 1) * buffer_size_ * sizeof(I) - 1, SEEK_SET) != 0) {
      throw logic_error("Seek failed");
    }

    if (fputs("1", pFile) < 0) {
      throw logic_error("Extend file failed");
    }
    
    if (fclose(pFile) != 0) {
      throw logic_error("Failed to close file");
    }
  }

  size_t size_;
  size_t buffer_size_;
  size_t stream_buffer_size_;
  size_t stream_cache_size_;
  size_t d_;
  vector<I> insert_buffer_;
  vector<Block<S, I>> blocks_;
  string filename_;
};