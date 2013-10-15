#pragma once

#include <cassert>
#include <cmath>
#include <vector>
#include <queue>
#include <stdexcept>

using namespace std;

template <class S, typename I, uint64_t d>
class ExternalHeap;

template <class S, typename I, uint64_t d>
class Block {
public:
  typedef uint64_t Child;
  
  Block(size_t start, size_t end, ExternalHeap<S, I, d>* heap)
    : heap_(heap), stream_(nullptr), element_count_(0), start_(start), end_(end) {
      // Avoid calling methods on 'heap' in the constructor
      // because it may not be initialized properly at this point
  }
  
  // Grab from all children
  void refill() {
    assert(imperfect());
    
    if (children() == 0) {
      steal_from_last();
      return;
    }
    
    priority_queue<ElementChild> Q;
    for (Child c = 0; c < children(); c++) {
      assert(!child(c)->empty());
      
      child(c)->open_at_first_element();
      Q.push(ElementChild(child(c)->peek(), c));
    }
    
    const uint64_t elements_to_refill = min(elements_in_children(), (uint64_t)ceil((double)(end_ - start_) / 2));
    // const uint64_t elements_to_refill = min(elements_in_children(), (uint64_t)(end_ - start_ - element_count()));
    
    // TODO(knielsen): Maybe change memory layout to growing the other way. I don't think easy elimination of data movement is possible.
    this->move_back(elements_to_refill);
    
    this->open_front();
    this->seek_back(elements_to_refill);
    
    for (uint64_t i = 0; i < elements_to_refill; i++) {
      auto e = Q.top(); Q.pop();
      
      child(e.child)->read_dec();
      if (!child(e.child)->empty())
        Q.push(ElementChild(child(e.child)->peek(), e.child));
      
      this->write_inc(e.element);
    }
    
    this->close();
    
    for (Child c = 0; c < children(); c++)
      child(c)->close();
    
    const uint64_t children_before = children(); // Needed since children() may change while refilling recursively!
    for (Child c = 0; c < children_before; c++) {
      Child reverse = children_before - c - 1;
      if (child(reverse)->imperfect())
        child(reverse)->refill();
    }
    
    if (imperfect()) {
      assert(children() == 0);
      steal_from_last();
    }
  }
  
  // Sift up to parent (recursively)
  void recursive_sift() {
    if (root())
      return;
    
    const uint64_t elements_before = heap_->elements_in_heap();
    
    // TODO(knielsen): Maybe reduce space consumption.
    const uint64_t r = this->element_count() + parent()->element_count();
    const uint64_t elements_in_parent_before = parent()->element_count();
    vector<I> buffer;
    buffer.reserve(r);
    
    this->open_at_first_element();
    parent()->open_at_first_element();
    
    // Merge into internal memory
    bool element_in_child_moved_to_parent = false;
    
    while (!this->empty() && !parent()->empty()) {
      if (this->peek() > parent()->peek()) {
        element_in_child_moved_to_parent = true;
        buffer.push_back(this->read_dec());
      } else {
        buffer.push_back(parent()->read_dec());
      }
    }
    const uint64_t elements_in_child_less_than_min_in_parent = this->element_count();
    while (!this->empty()) {
      buffer.push_back(this->read_dec());
    }
    while (!parent()->empty())
      buffer.push_back(parent()->read_dec());
    
    // Distribute result to disk
    // TODO(knielsen): Undersøg om det her er korrekt.
    //                 Forstår ikke hvad de gør i paperet.
    // const uint64_t k = min((uint64_t)buffer.size() / 2, elements_in_parent_before + elements_in_child_less_than_min_in_parent);
    const uint64_t k = r - elements_in_parent_before;
    
    // To parent
    parent()->seek_back(r - k);
    for (uint64_t i = 0; i < r - k; i++)
      parent()->write_inc(buffer[i]);
    
    // To child
    this->seek_back(k);
    uint64_t counter = 0;
    for (uint64_t i = 0; i < k; i++) {
      this->write_inc(buffer[i + (r - k)]);
      counter++;
    }
    
    this->close();
    parent()->close();
    
    assert(elements_before == heap_->elements_in_heap());
    
    if (element_in_child_moved_to_parent)
      parent()->recursive_sift();
  }
  
  void steal_from_last() {
    assert(imperfect());
    
    if (last()) {
      if (element_count() == 0)
        heap_->blocks().pop_back();
      return ;
    }
    
    const uint64_t half = ceil(((double)(end_ - start_)) / 2);
    uint64_t s = heap_->last_block()->element_count() + this->element_count();
    if (s > end_ - start_) {
      // Case 1
      move_records(this, heap_->last_block());
      assert(s == this->element_count() + heap_->last_block()->element_count());
      recursive_sift();
    } else if (half <= s && s <= end_ - start_) {
      // Case 2
      move_records(this, heap_->last_block());
      assert(s == this->element_count());
      assert(0 == heap_->last_block()->element_count());
      
      heap_->blocks().pop_back();
      recursive_sift();
    } else {
      // Case 3
      move_records(this, heap_->last_block());
      assert(s == this->element_count());
      assert(0 == heap_->last_block()->element_count());
      assert(imperfect());
      
      heap_->blocks().pop_back();
      steal_from_last(); // Ensured no to happen twice
    }
  }
  
  // Opens the block for reading/writing in the beginning of the block
  void open_front() {
    assert(stream_ == nullptr);
    stream_ = new S();
    stream_->open(start_, end_, heap_->stream_buffer_size());
  }

  // Opens the block for reading/writing from the first element (descending)
  void open_at_first_element() {
    assert(stream_ == nullptr);
    open_front();
    stream_->seek(end_ - element_count_);
  }

  // Writes and increments the element counter
  void write_inc(I element) {
    assert(stream_ != nullptr);
    stream_->write(element);
    element_count_++;
    assert(element_count_ <= end_ - start_);
  }
  
  // Reads an element without changing stream or element counter
  I peek() {
    assert(stream_ != nullptr);
    return stream_->peek();
  }

  // Seek from the back (end - 'elements')
  void seek_back(size_t elements) {
    assert(stream_ != nullptr);
    assert(elements <= end_ - start_);
    stream_->seek(end_ - elements);
  }
  
  // Moves the contents of the stream back 'elements' positions.
  void move_back(size_t elements) {
    assert(element_count() + elements <= end_ - start_);
    
    const uint64_t len = element_count();
    I* buffer = new I[len];
    open_at_first_element();
    for (uint64_t i = 0; i < len; i++)
      buffer[i] = read_dec();
    
    seek_back(elements + len);
    for (uint64_t i = 0; i < len; i++)
      write_inc(buffer[i]);
    
    close();
    delete[] buffer;
  }

  bool empty() {
    return element_count() == 0;
  }

  // Reads an element and decrements the element counter
  I read_dec() {
    assert(stream_ != nullptr);
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
  
  bool last() {
    return &heap_->blocks()[heap_->blocks().size() - 1] == this;
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
    
    for (Child c = 0; c < children(); c++)
      child(c)->to_dot(ss);
  }

  void consistency_check() {
#ifndef NDEBUG
    if (!last())
      assert(!imperfect());
    
    if (!root()) {
      // Check heap invariant
      this->open_at_first_element();
      parent()->open_front();
      parent()->seek_back(1);

      I min_in_parent = parent()->peek();
      I max_in_this = this->peek();
      assert(max_in_this <= min_in_parent);
      
      parent()->close();
      this->close();
    }
    
    // Check that elements are sorted
    this->open_at_first_element();
    I before = stream_->read_next();
    while (stream_->has_next()) {
      I next = stream_->read_next();
      assert(before >= next);
      before = next;
    }
    this->close();
    
    for (Child c = 0; c < children(); c++)
      child(c)->consistency_check();
    
    bool found = false;
    for (uint64_t i = 0; i < heap_->blocks().size(); i++) {
      if (&heap_->blocks()[i] == this) {
        found = true;
        break;
      }
    }
    assert(found);
#endif
  }
  
private:
  uint64_t elements_in_children() {
    uint64_t count = 0;
    for (Child c = 0; c < children(); c++)
      count += child(c)->element_count();
    return count;
  }
  
  struct ElementChild {
    ElementChild(I element, Child child) : element(element), child(child) { }
    
    I element;
    Child child;
    
    bool operator<(const ElementChild& o) const {
      return element < o.element;
    }
  };
  
  void move_records(Block* to, Block* from) {
    const uint64_t block_capacity = to->end_ - to->start_;
    const uint64_t elements_in_to = to->element_count();
    
    vector<I> buffer;
    buffer.reserve(block_capacity);
    
    to->open_at_first_element();
    from->open_at_first_element();
    
    // Merge into internal memory
    uint64_t elements_from_from = 0;
    while (!to->empty() && !from->empty() && elements_from_from < block_capacity - elements_in_to) {
      if (to->peek() >= from->peek())
        buffer.push_back(to->read_dec());
      else {
        elements_from_from++;
        buffer.push_back(from->read_dec());
      }
    }
    while (!to->empty()) {
      assert(buffer.size() < block_capacity);
      buffer.push_back(to->read_dec());
    }
    while (!from->empty() && elements_from_from < block_capacity - elements_in_to) {
      elements_from_from++;
      buffer.push_back(from->read_dec());
    }
    
    from->close();
    assert(buffer.size() <= block_capacity);
    
    to->seek_back(buffer.size());
    for (uint64_t i = 0; i < buffer.size(); i++)
      to->write_inc(buffer[i]);
    
    to->close();
  }
  
  // Not owned
  ExternalHeap<S, I, d>* heap_;
  // Owned (it's a pointer to make sure it doesn't use any memory when not used.)
  S* stream_;
  size_t element_count_;
  size_t start_;
  size_t end_;
};