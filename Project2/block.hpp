#pragma once

#include <cassert>

template <class S, typename I>
class ExternalHeap;

template <class S, typename I>
class Block {
public:
  Block(size_t start, size_t end, ExternalHeap<S, I>* heap)
    : start_(start), end_(end), heap_(heap), element_count_(0), stream_(nullptr) {
      // Avoid calling methods on 'heap' in the constructor
      // because it may not be initialized properly at this point
  }

  // Grab from all children
  void refill();
  // Sift up to parent (recursively)
  void recursive_sift() {
    // TODO(lespeholt): Make sifting
  }
  void steal_from_last();
  Block* parent();

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

  // Reads an element and decrements the element counter
  I read_dec() {
    stream_->read_next();
    assert(element_count_ != 0);
    element_count_--;
  }

  // Close block
  void close() {
    assert(stream_ != nullptr);
    stream_->close();
    delete stream_;
    stream_ = nullptr;
  }

  bool imperfect() {
    size_t potential_elements = end_ - start_;

    return element_count_ < (size_t)ceil((double)potential_elements / 2);
  }

  bool root() {
    // start_ == 0 is not valid because we may swap some blocks
    return &heap_->blocks()[0] == this;
  }

private:
  // Not owned
  ExternalHeap<S, I>* heap_;
  // Owned (it's a pointer to make sure it doesn't use any memory when not used.)
  S* stream_;
  size_t element_count_;
  size_t start_;
  size_t end_;
};