#pragma once

#include <array>
#include <string>
#include <vector>
#include <queue>
#include <cstdint>
#include <sstream>

#include "block.hpp"
#include <string>

using namespace std;

template <class S, typename I, uint64_t d>
class ExternalHeap {
public:
  ExternalHeap(string filename, size_t buffer_size) : size_(0), buffer_size_(buffer_size) {
    // With this, the capacity changes to *at least* 'buffer_size'.
    // To make sure we use precisely a buffer of size 'buffer_size'
    // 'buffer_size_' is set (instead of using insert_buffer_.capacity().)
    insert_buffer_.reserve(buffer_size);
    filename_ = filename;
  }

  void insert(I element) {
    size_++;
    
    if (insert_buffer_.size() >= buffer_size_) {
      // Inserts new block at the end, opens its stream and store the insert buffer in sorted order (descending)

      // Resize file
      add_block_to_file();

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
        swap(blocks_.back(), blocks_[blocks_.size() - 2]);
        // TODO(lespeholt): Muligvis lidt optimering her, hvor der kan undgaas kald
        // TODO(lespeholt): Raekkefoelge?
        blocks_.back().recursive_sift();
        blocks_[blocks_.size() - 2].recursive_sift();
      } else {
        blocks_.back().recursive_sift();
      }
      
      consistency_check();
    }

    // Push element in insert buffer
    insert_buffer_.push_back(element);
    push_heap(insert_buffer_.begin(), insert_buffer_.end());
  }

  I peek_max() {
    if (!insert_buffer_.empty() && !blocks_.empty()) {
      blocks_[0].open_at_first_element();
      I res;
      if (insert_buffer_.front() >= blocks_[0].peek())
        res = insert_buffer_.front();
      else
        res = blocks_[0].peek();
      blocks_[0].close();
      return res;
    } else if (insert_buffer_.empty() && !blocks_.empty()) {
      blocks_[0].open_at_first_element();
      I res = blocks_[0].peek();
      blocks_[0].close();
      return res;
    } else if (!insert_buffer_.empty() && blocks_.empty()) {
      return insert_buffer_.front();
    } else {
      // The heap is empty!
      throw logic_error("Trying to extract from empty heap!");
    }
  }
  
  void extract_max() {
    size_--;
    
    // TODO(knielsen): Keep some elements in the root buffered.
    
    bool biggest_in_insert_buffer;
    if (!insert_buffer_.empty() && !blocks_.empty()) {
      blocks_[0].open_at_first_element();
      biggest_in_insert_buffer = insert_buffer_.front() >= blocks_[0].peek();
      blocks_[0].close();
    } else if (insert_buffer_.empty() && !blocks_.empty()) {
      biggest_in_insert_buffer = false;
    } else if (!insert_buffer_.empty() && blocks_.empty()) {
      biggest_in_insert_buffer = true;
    } else {
      // The heap is empty!
      throw logic_error("Trying to extract from empty heap!");
    }
    
    if (biggest_in_insert_buffer) {
      // Biggest is in insert buffer
      pop_heap(insert_buffer_.begin(), insert_buffer_.end());
      insert_buffer_.pop_back();
    } else {
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
    }
    
    consistency_check();
  }
  
  size_t stream_buffer_size() const {
    // TODO(lespeholt): We probably want a separate stream buffer size 
    return buffer_size_;
  }

  vector<Block<S, I, d>>& blocks() {
    return blocks_;
  }

  const string& filename() const { 
    return filename_;
  }
  
  Block<S, I, d>* last_block() {
    return &blocks_[blocks_.size() - 1];
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
    FILE* pFile = fopen(filename().c_str(), "w+");
    assert(pFile != nullptr);

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
  vector<I> insert_buffer_;
  vector<Block<S, I, d>> blocks_;
  string filename_;
};