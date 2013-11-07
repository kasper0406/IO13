#pragma once

#include "buffered_stream.hpp"
#include "f_stream.hpp"

#ifdef WIN32
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

#include <string>

using namespace std;

template<class I, template<typename> class S>
class CachedStream {
public:
  CachedStream(uint64_t cache_size) : cache_size_(cache_size), stream_(nullptr), cache_(nullptr) { }
  
  ~CachedStream() {
    if (cache_ != nullptr)
      delete[] cache_;
    close();
  }
  
  // Move constructor
  CachedStream(CachedStream&& other) NOEXCEPT {
    stream_ = other.stream_;
    position_ = other.position_;
    cache_pos_ = other.cache_pos_;
    cache_ = other.cache_;
    stream_info_ = other.stream_info_;
    cache_size_ = other.cache_size_;
    
    other.cache_ = nullptr;
    other.stream_ = nullptr;
  }
  
  CachedStream& operator=(CachedStream&& other) NOEXCEPT {
    if (this != &other) {
      cache_size_ = other.cache_size_;
      stream_ = other.stream_;
      position_ = other.position_;
      cache_pos_ = other.cache_pos_;
      cache_ = other.cache_;
      stream_info_ = other.stream_info_;
      
      other.cache_ = nullptr;
      other.stream_ = nullptr;
    }
    return *this;
  }
  
  // Copy constructor.
  CachedStream(const CachedStream& other) {
    assert(false);
  }
  
  CachedStream& operator=(const CachedStream& other) {
    assert(false);
  }
  
  void open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
    stream_info_.filename = filename;
    stream_info_.start = start;
    stream_info_.end = end;
    stream_info_.buffer_size = buffer_size;
    
    position_ = 0;
  }
  
  I peek() {
    assert(position_ < stream_info_.end - stream_info_.start);
    if (is_cached())
      return get_from_cache();
    else
      return get_from_stream();
  }
  
  I read_next() {
    I element = peek();
    position_++;
    seek_required_ = true;
    return element;
  }
  
  I read_prev() {
    assert(position_ < stream_info_.end - stream_info_.start);
    assert(position_ >= 0);
    I result;
    if (is_cached())
      result = get_from_cache();
    else {
      result = get_from_stream_backwards();
    }
    seek_required_ = true;
    position_--;
    return result;
  }
  
  void write(I value) {
    assert(position_ < stream_info_.end - stream_info_.start);
    if (is_cached()) {
      // Update the cache to the new value
      cache_[position_ - cache_pos_] = value;
    }
    
    prepare_stream();
    stream_->write(value);
    position_++;
  }
  
  void backward_write(I value) {
    assert(position_ < stream_info_.end - stream_info_.start);
    if (is_cached()) {
      // Update the cache to the new value
      cache_[position_ - cache_pos_] = value;
    }
    
    prepare_stream();
    stream_->backward_write(value);
    position_--;
  }
  
  void close() {
    close_stream();
  }
  
  void seek(int64_t position) {
    assert(position >= stream_info_.start);
    assert(position <= stream_info_.end); // Seek is allowed to be on the non-existing end element. Hence the <=.
    position_ = position - stream_info_.start;
    seek_required_ = true;
  }
  
  bool has_next() {
    return position_ < stream_info_.end - stream_info_.start;
  }
  
  int64_t position() const {
    return stream_info_.start + position_;
  }
  
  static void cleanup() { }
  
private:
  bool is_cached() {
    return cache_ != nullptr && position_ >= cache_pos_ && position_ < cache_pos_ + cache_size_;
  }
  
  I get_from_cache() {
    return cache_[position_ - cache_pos_];
  }
  
  I get_from_stream() {
    prepare_stream();
    
    I res = stream_->read_next();
    
    // Fill up the cache
    cache_pos_ = position_ + 1;
    if (cache_ == nullptr)
      cache_ = new I[cache_size_];
    
    for (uint64_t i = 0; i < cache_size_ && stream_->has_next(); i++)
      cache_[i] = stream_->read_next();
    stream_->seek(stream_pos(position_));
    
    return res;
  }
  
  /*
   * Rewind position such that the cached is filled up reading from left to right,
   * and return the element requested.
   */
  I get_from_stream_backwards() {
    const int64_t position_before = position_;
    position_ = max((int64_t)0, position_ - cache_size_);
    seek_required_ = true;
    prepare_stream();
    
    // Fill up the cache
    cache_pos_ = position_;
    if (cache_ == nullptr)
      cache_ = new I[cache_size_];
    
    for (uint64_t i = 0; i < cache_size_ && stream_->has_next(); i++)
      cache_[i] = stream_->read_next();
    
    position_ = position_before;
    I result;
    if (is_cached())
      result = get_from_cache();
    else {
      assert(stream_->has_next());
      result = stream_->read_next();
    }
    return result;
  }
  
  /**
   * Initializes and opens the stream if the stream is not open.
   * Seek to the correct position in the stream.
   */
  void prepare_stream() {
    if (stream_ == nullptr) {
      stream_ = new S<I>(0); // Cache size doesn't matter
      stream_->open(stream_info_.filename, stream_info_.start, stream_info_.end, stream_info_.buffer_size);
    }
    if (seek_required_) {
      stream_->seek(stream_pos(position_));
      seek_required_ = false;
    }
  }
  
  void close_stream() {
    if (stream_ != nullptr) {
      stream_->close();
      delete stream_;
      stream_ = nullptr;
    }
  }
  
  // Returns the position in the stream when seeking to a specific offset in the block.
  int64_t stream_pos(int64_t offset) {
    return stream_info_.start + offset;
  }
  
  int64_t cache_size_;
  
  S<I>* stream_;
  int64_t position_; // The location seeked to in the stream.
  int64_t cache_pos_; // The location the cache begins.
  bool seek_required_; // Indicates if seek should be called on the stream before calling any operations on it.
  
  struct StreamInfo {
    string filename;
    uint64_t start;
    uint64_t end;
    size_t buffer_size;
  } stream_info_;
  
  I* cache_; // The read cache
};

template<>
void CachedStream<int, BufferedStream>::open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
  // HACK(lespeholt): If root, then we want a full buffer
  // Probably better to just keep it open in the root because when
  // we not use CachedStream
  if (start == 0) {
    cache_size_ = buffer_size;
  }

  stream_info_.filename = filename;
  stream_info_.start = start;
  stream_info_.end = end;
  stream_info_.buffer_size = buffer_size;
  
  position_ = 0;
}

template<>
void CachedStream<int, FStream>::open(string filename, uint64_t start, uint64_t end, size_t buffer_size) {
  // HACK(lespeholt): If root, then we want a full buffer
  // Probably better to just keep it open in the root because when
  // we not use CachedStream
  if (start == 0) {
    cache_size_ = 1024;
  }

  stream_info_.filename = filename;
  stream_info_.start = start;
  stream_info_.end = end;
  stream_info_.buffer_size = buffer_size;
  
  position_ = 0;
}