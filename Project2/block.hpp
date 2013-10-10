#pragma once

template <class S, typename I>
class ExternalHeap;

template <class S, typename I>
class Block {
public:
  Block(size_t start, size_t end, const ExternalHeap<S, I>* heap)
    : start_(start), end_(end), heap_(heap), element_count_(0) {
  }

  // Grab from all children
  void refill();
  // Sift up to parent
  void sift();
  void stealFromLast();
  Block* parent();

private:
  // Not owned
  const ExternalHeap<S, I>* heap_;
  size_t element_count_;
  size_t start_;
  size_t end_;
};