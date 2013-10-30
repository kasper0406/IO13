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
  CachedStream() : stream_(nullptr), cache_(nullptr) { }
  
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
    
    other.cache_ = nullptr;
    other.stream_ = nullptr;
  }
  
  CachedStream& operator=(CachedStream&& other) NOEXCEPT {
    if (this != &other) {      
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
  
  void close() {
    close_stream();
  }
  
  void seek(uint64_t position) {
    assert(position >= stream_info_.start);
    assert(position < stream_info_.end);
    position_ = position - stream_info_.start;
    seek_required_ = true;
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
    prepare_stream();
    
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
  
  /**
   * Initializes and opens the stream if the stream is not open.
   * Seek to the correct position in the stream.
   */
  void prepare_stream() {
    if (stream_ == nullptr) {
      stream_ = new S();
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
  uint64_t stream_pos(uint64_t offset) {
    return stream_info_.start + offset;
  }
  
  S* stream_;
  uint64_t position_; // The location seeked to in the stream.
  uint64_t cache_pos_; // The location the cache begins.
  bool seek_required_; // Indicates if seek should be called on the stream before calling any operations on it.
  
  struct StreamInfo {
    string filename;
    uint64_t start;
    uint64_t end;
    size_t buffer_size;
  } stream_info_;
  
  I* cache_; // The read cache
};
