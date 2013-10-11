#pragma once

#include <cassert>
#include <cmath>
#include <vector>

using namespace std;

template <class S, typename I, uint64_t d>
class ExternalHeap;

template <class S, typename I, uint64_t d>
class Block {
public:
  Block(size_t start, size_t end, ExternalHeap<S, I, d>* heap)
    : heap_(heap), stream_(nullptr), element_count_(0), start_(start), end_(end) {
      // Avoid calling methods on 'heap' in the constructor
      // because it may not be initialized properly at this point
  }

  // Grab from all children
  void refill() {
    
  }
  
  // Sift up to parent (recursively)
  void recursive_sift() {
    if (root())
      return;
    
    // TODO(knielsen): Maybe reduce space consumption.
    const uint64_t r = this->element_count_ + parent()->element_count_;
    vector<I> buffer;
    buffer.reserve(r);
    
    this->open_at_first_element();
    parent()->open_at_first_element();
    
    // Merge into internal memory
    uint64_t h = 0;
    while (this->stream_->has_next() && parent()->stream_->has_next()) {
      if (this->stream_->peek() > parent()->stream_->peek()) {
        buffer.push_back(this->stream_->read_next());
        h++;
      } else {
        buffer.push_back(parent()->stream_->read_next());
      }
    }
    while (this->stream_->has_next())
      buffer.push_back(this->stream_->read_next());
    while (parent()->stream_->has_next())
      buffer.push_back(parent()->stream_->read_next());
    
    // Distribute result to disk
    const uint64_t k = max(r - h, (uint64_t)ceil((double)(end_ - start_) / 2));
    
    // To parent
    parent()->element_count_ = r - k;
    parent()->stream_->seek(parent()->end_ - parent()->element_count_);
    for (uint64_t i = 0; i < parent()->element_count_; i++)
      parent()->stream_->write(buffer[i]);
    
    // To child
    this->element_count_ = k;
    this->stream_->seek(this->end_ - this->element_count_);
    for (uint64_t i = 0; i < this->element_count_; i++)
      this->stream_->write(buffer[i + (r - k)]);
    
    this->close();
    parent()->close();
    
    if (h != 0)
      parent()->recursive_sift();
  }
  
  void steal_from_last();
  
  // Opens the block for reading/writing in the beginning of the block
  void open_front() {
    assert(stream_ == nullptr);
    stream_ = new S();
    stream_->open(start_, end_, heap_->stream_buffer_size());
  }

  // Opens the block for reading/writing from the first element (descending)
  void open_at_first_element() {
    open_front();
    stream_->seek(end_ - element_count_);
  }

  // Writes and increments the element counter
  void write_inc(I element) {
    stream_->write(element);
    element_count_++;
    assert(element_count_ <= end_ - start_);
  }
  
  // Reads an element without changing stream or element counter
  I peek() {
    return stream_->peek();
  }

  // Reads an element and decrements the element counter
  I read_dec() {
    I element = stream_->read_next();
    assert(element_count_ != 0);
    element_count_--;
    return element;
  }

  // Close block
  void close() {
    assert(stream_ != nullptr);
    stream_->close();
    delete stream_;
    stream_ = nullptr;
  }

  uint64_t element_count() {
    return element_count_;
  }
  
  bool imperfect() {
    size_t potential_elements = end_ - start_;

    return element_count_ < (size_t)ceil((double)potential_elements / 2);
  }

  bool root() {
    // start_ == 0 is not valid because we may swap some blocks
    return &heap_->blocks()[0] == this;
  }
  
  Block* parent() {
    return &heap_->blocks()[(heap_->pos(this) - 1) / d];
  }
  
  Block* child(uint64_t child) {
    return &heap_->blocks()[d * heap_->pos(this) + 1 + child];
  }
  
  uint64_t children() {
    return min(max((int64_t)0, (int64_t)((int64_t)heap_->blocks().size() - (int64_t)(d * heap_->pos(this) + 1))), (int64_t)d);
  }
  
  void to_dot(stringstream& ss) {
    stringstream label;
    open_at_first_element();
    while (stream_->has_next()) {
      label << stream_->read_next();
      if (stream_->has_next())
        label << " ";
    }
    close();
    
    ss << "n" << heap_->pos(this) << " [label=\"" << label.str() << "\",shape=box];" << endl;
    
    if (!root())
      ss << "n" << heap_->pos(this) << " -> " << "n" << heap_->pos(parent()) << ";";
    
    for (uint64_t c = 0; c < children(); c++)
      child(c)->to_dot(ss);
  }

private:
  // Not owned
  ExternalHeap<S, I, d>* heap_;
  // Owned (it's a pointer to make sure it doesn't use any memory when not used.)
  S* stream_;
  size_t element_count_;
  size_t start_;
  size_t end_;
};