#pragma once

#include <cassert>
#include <cmath>
#include <vector>
#include <queue>
#include <stdexcept>

#include "cached_stream.hpp"

using namespace std;

#ifdef WIN32
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

template <class S, typename I>
class ExternalHeap;

template <class S, typename I>
class Block {
public:
  typedef uint64_t Child;
  
  Block(size_t start, size_t end, ExternalHeap<S, I>* heap)
    : heap_(heap), stream_(S()), element_count_(0), start_(start), end_(end) {
      // Avoid calling methods on 'heap' in the constructor
      // because it may not be initialized properly at this point
  }

  // Move constructor.
  Block(Block&& other) NOEXCEPT {
    // assert(other.stream_ == nullptr);
    element_count_ = other.element_count_;
    cached_minimum_ = other.cached_minimum_;
    start_ = other.start_;
    end_ = other.end_;
    stream_ = move(other.stream_);
    heap_ = other.heap_;
  }

  Block& operator=(Block&& other) NOEXCEPT {
    assert(heap_ == other.heap_);
    // assert(stream_ == nullptr);
    if (this != &other)
    {
      element_count_ = other.element_count_;
      cached_minimum_ = other.cached_minimum_;
      start_ = other.start_;
      end_ = other.end_;
    }
    return *this;
  }

  // Copy constructor.
  Block(Block& other) {
    assert(false);
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

  void sift(bool recursive = true) {
    sift_memory_efficient(recursive);
    // sift_memory_wasting(recursive);
  }
  
  // Sift up to parent (optionally recursively)
  // This version writes backwards and use the cached minimum element in parent,
  // but only uses a block of memory.
  void sift_memory_efficient(bool recursive) {
    if (root())
      return;
    
    const uint64_t r = this->element_count() + parent()->element_count();
    
    vector<I> buffer;
    buffer.reserve(end_ - start_); // Reserve 1 block of buffer space
    
    // Read the elements of the child (this) into the buffer
    uint64_t elements_in_child_less_than_min_in_parent = 0;
    this->open_at_first_element();
    while (!this->empty()) {
      if (this->peek() < parent()->minimum_element())
        elements_in_child_less_than_min_in_parent++;
      
      buffer.push_back(this->read_dec());
    }
    
    assert(this->element_count() == 0);
    
    // Find out how many elements goes to the child (k), and how many to the parent (r - k).
    const uint64_t minimum_block_size = ceil((double)(end_ - start_) / 2);
    const uint64_t maximum_block_size = end_ - start_;
    uint64_t k = max(elements_in_child_less_than_min_in_parent, minimum_block_size);
    if (r - k > maximum_block_size) {
      // Too many elements assigned to the parent.
      // Transfer some of them to the child.
      k += (r - k) - maximum_block_size;
    }
    
    // Write elements to the child
    parent()->open_at_last_element();
    this->seek_back(1);
    while (!parent()->empty() && !buffer.empty() && this->element_count() < k) {
      if (parent()->peek() < buffer.back()) {
        I element = parent()->backward_read_dec();
        this->backward_write_inc(element);
      } else {
        this->backward_write_inc(buffer.back());
        buffer.pop_back();
      }
    }
    while (!buffer.empty() && this->element_count() < k) {
      this->backward_write_inc(buffer.back());
      buffer.pop_back();
    }
    while (!parent()->empty() && this->element_count() < k) {
      I element = parent()->backward_read_dec();
      this->backward_write_inc(element);
    }
    
    assert(this->element_count() == k);
    this->close();
    
    // Move the rest of the parent elements into the buffer and do internal memory merging,
    // writing the result to parent.
    const int64_t buffer_split = buffer.size(); // Note signed, because of index comparisons.
    while (!parent()->empty())
      buffer.push_back(parent()->backward_read_dec());
    
    assert(buffer.size() == r - k); // The buffer should contain exactly the elements going to the parent
    assert(parent()->element_count() == 0);
    
    // Merge two piles of the buffer into the parent node
    parent()->seek_back(r - k);
    int64_t pile1_index = 0;                 // Indexes for the two piles to be merged in the buffer
    int64_t pile2_index = buffer.size() - 1; // pile1 grows to the right, pile2 grows to the left.
                                             // Note that these needs to be signed, as buffer_split could be 0.
    while (pile1_index < buffer_split && pile2_index >= buffer_split) {
      if (buffer[pile1_index] > buffer[pile2_index]) {
        parent()->write_inc(buffer[pile1_index++]);
      } else {
        parent()->write_inc(buffer[pile2_index--]);
      }
    }
    while (pile1_index < buffer_split)
      parent()->write_inc(buffer[pile1_index++]);
    while (pile2_index >= buffer_split)
      parent()->write_inc(buffer[pile2_index--]);
    
    assert(parent()->element_count() == r - k);
    
    parent()->close();
    
    // If buffer_split != 0, then some elements from the child
    // was moved to the parent node.
    if (buffer_split != 0 && recursive)
      parent()->sift(true);
  }
  
  // Sift up to parent (optionally recursively)
  // This version is using 2x memory, but is writing forward,
  // and not using the cached minimum element in parent.
  void sift_memory_wasting(bool recursive) {
    if (root())
      return;
    
    const uint64_t r = this->element_count() + parent()->element_count();
    
    vector<I> buffer;
    buffer.reserve(r);
    
    this->open_at_first_element();
    parent()->open_at_first_element();
    
    // Merge into internal memory
    uint64_t child_elements_greater_than_min_in_parent = 0;
    
    while (!this->empty() && !parent()->empty()) {
      if (this->peek() > parent()->peek()) {
        child_elements_greater_than_min_in_parent++;
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
    // TODO(knielsen): Prettyfy this?
    const uint64_t minimum_block_size = ceil((double)(end_ - start_) / 2);
    const uint64_t maximum_block_size = end_ - start_;
    uint64_t k = max(elements_in_child_less_than_min_in_parent, minimum_block_size);
    if (r - k > maximum_block_size) {
      // Too many elements assigned to the parent.
      // Transfer some of them to the child.
      k += (r - k) - maximum_block_size;
    }
    
    // The paper wrote the following, but it is clearly wrong because all the
    // elements might end up in the child.
    // Ex, assume block size of 1024, r = 2048, h = 0 => k = 2048 > 1024 <-!!!
    // const uint64_t k = max(r - elements_in_child_less_than_min_in_parent, (uint64_t)ceil(((double)end_ - (double)start_)/2));

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
    
    if (elements_in_child_less_than_min_in_parent && recursive)
      parent()->sift(true);
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
      sift(true);
    } else if (half <= s && s <= end_ - start_) {
      // Case 2
      move_records(this, heap_->last_block());
      assert(s == this->element_count());
      assert(0 == heap_->last_block()->element_count());
      
      heap_->blocks().pop_back();
      sift(true);
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
    // assert(stream_ == nullptr);
    // stream_ = new S();
    stream_.open(heap_->filename(), start_, end_, heap_->stream_buffer_size());
  }
  
  void open_at_last_element() {
    open_front();
    seek_back(1);
  }

  // Opens the block for reading/writing from the first element (descending)
  void open_at_first_element() {
    // assert(stream_ == nullptr);
    open_front();
    stream_.seek(end_ - element_count_);
  }

  // Writes and increments the element counter
  void write_inc(I element) {
    /*
    if (stream_.position() == end_ - 1) {
      // We are about to write to the last element
      // Hance we are updating the minimum
      cached_minimum_ = element;
    }
     */
    
    // assert(stream_ != nullptr);
    stream_.write(element);
    element_count_++;
    
    if (!stream_.has_next()) {
      // We are about to write to the last element
      // Hance we are updating the minimum
      cached_minimum_ = element;
    }
    
    assert(element_count_ <= end_ - start_);
  }
  
  // Writes backward and increments the element counter.
  void backward_write_inc(I element) {
    if (stream_.position() == end_ - 1) {
      // We are about to write to the last element
      // Hance we are updating the minimum
      cached_minimum_ = element;
    }
    
    stream_.backward_write(element);
    element_count_++;
    
    assert(element_count_ <= end_ - start_);
  }
  
  // Reads an element without changing stream or element counter
  I peek() {
    // assert(stream_ != nullptr);
    return stream_.peek();
  }

  // Seek from the back (end - 'elements')
  void seek_back(size_t elements) {
    // assert(stream_ != nullptr);
    assert(elements <= end_ - start_);
    stream_.seek(end_ - elements);
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
    // assert(stream_ != nullptr);
    I element = stream_.read_next();
    assert(element_count_ != 0);
    element_count_--;
    return element;
  }
  
  // Reads an elements backward from the stream and decrements the element counter
  I backward_read_dec() {
    I element = stream_.read_prev();
    assert(element_count_ != 0);
    element_count_--;
    return element;
  }

  // Close block
  void close() {
    // assert(stream_ != nullptr);
    stream_.close();
    // delete stream_;
    // stream_ = nullptr;
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
    return &heap_->blocks()[(heap_->pos(this) - 1) / heap_->d()];
  }
  
  Block* child(uint64_t child) {
    return &heap_->blocks()[heap_->d() * heap_->pos(this) + 1 + child];
  }
  
  uint64_t children() {
    return min(max((int64_t)0, (int64_t)((int64_t)heap_->blocks().size() - (int64_t)(heap_->d() * heap_->pos(this) + 1))), (int64_t)heap_->d());
  }
  
  void to_dot(stringstream& ss) {
    stringstream label;
    open_at_first_element();
    while (stream_.has_next()) {
      label << stream_.read_next();
      if (stream_.has_next())
        label << " ";
    }
    label << " | " << cached_minimum_;
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
    
    // A block must not be over loaded.
    assert(element_count() <= end_ - start_);
    
    // Check that elements are sorted
    this->open_at_first_element();
    I before = stream_.read_next();
    while (stream_.has_next()) {
      I next = stream_.read_next();
      assert(before >= next);
      before = next;
    }
    this->close();
    assert(before == cached_minimum_); // Ensure that the cached_minimum_ is up to date.
    
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
    
    for (Child c = 0; c < children(); c++)
      child(c)->consistency_check();
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
  
  I minimum_element() const {
    return cached_minimum_;
  }
  
  // Not owned
  ExternalHeap<S, I>* heap_;
  // Owned (it's a pointer to make sure it doesn't use any memory when not used.)
  S stream_;
  size_t element_count_;
  size_t start_;
  size_t end_;
  
  I cached_minimum_;
};