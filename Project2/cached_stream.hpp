#pragma once

#ifdef WIN32
#define NOEXCEPT
#else
#define NOEXCEPT noexcept
#endif

#include <string>

using namespace std;

// TODO(knielsen): Refactor s.t. I is found from the supplied stream.
//                 or such that the stream is instiansiated with type I.
template<class I, class S, uint64_t cache_size>
class CachedStream {
public:
  CachedStream() : cache_(nullptr) { }
  
  ~CachedStream() {
    if (cache_ != nullptr)
      delete[] cache_;
    close();
  }
  
  // Move constructor
  CachedStream(const CachedStream&& other) NOEXCEPT {
    assert(false);
  }
  
  CachedStream& operator=(CachedStream&& other) NOEXCEPT {
    assert(false);
  }
  
  // Copy constructor.
  CachedStream(const CachedStream& other) {
    assert(false);
  }
  
  CachedStream& operator=(CachedStream& other) {
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
    return element;
  }
  
  void write(I value) {
    assert(position_ < stream_info_.end - stream_info_.start);
    if (stream_ == nullptr)
      open_stream();
    stream_->write(value);
  }
  
  void close() {
    close_stream();
  }
  
  void seek(uint64_t position) {
    assert(position >= stream_info_.start);
    assert(position < stream_info_.end);
    position_ = position - stream_info_.start;
  }
  
  bool has_next() {
    return position_ < stream_info_.end - stream_info_.start;
  }
  
private:
  bool is_cached() {
    return cache_ != nullptr && position_ >= cache_pos_ && position_ < cache_pos_ + cache_size;
  }
  
  I get_from_cache() {
    return cache_[position_ - cache_pos_];
  }
  
  I get_from_stream() {
    open_stream();
    
    I res = stream_->read_next();
    
    // Fill up the cache
    cache_pos_ = position_ + 1;
    if (cache_ == nullptr)
      cache_ = new I[cache_size];
    
    for (uint64_t i = 0; i < cache_size && stream_->has_next(); i++)
      cache_[i] = stream_->read_next();
    stream_->seek(stream_pos(position_));
    
    return res;
  }
  
  void open_stream() {
    stream_ = new S();
    stream_->open(stream_info_.filename, stream_info_.start, stream_info_.end, stream_info_.buffer_size);
    stream_->seek(stream_pos(position_));
  }
  
  void close_stream() {
    if (stream_ != nullptr) {
      stream_->close();
      delete stream_;
      stream_ = nullptr;
    }
  }
  
  // Returns the position in the stream when seeking to a specific offset in the block.
  uint64_t stream_pos(uint64_t offset) {
    return stream_info_.start + offset;
  }
  
  S* stream_;
  uint64_t position_; // The location seeked to in the stream.
  uint64_t cache_pos_; // The location the cache begins.
  
  struct StreamInfo {
    string filename;
    uint64_t start;
    uint64_t end;
    size_t buffer_size;
  } stream_info_;
  
  I* cache_; // The read cache
};
