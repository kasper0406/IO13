#pragma once

#include "input_stream.h"
#include "rw_stream.h"
#include <stdexcept>
#include <algorithm>

template <uint64_t B, typename T>
class BufferedInputStream : public RWStream<T>, public InputStream<T> {
public:
	BufferedInputStream() {
		buffer = new T[B];
		index = length = B;
	}

  ~BufferedInputStream() noexcept {
    if (this->buffer != nullptr)
      delete[] this->buffer;
  }
  
  BufferedInputStream(BufferedInputStream&& other) noexcept : RWStream<T>(move(other)) {
    buffer = other.buffer;
    index = other.index;
    length = other.length;
    filename_ = other.filename_;
    
    other.buffer = nullptr;
  }
  
  BufferedInputStream(const BufferedInputStream& other) {
    throw runtime_error("Streams should not be copied!");
  }
  
  BufferedInputStream& operator=(const BufferedInputStream& other) {
    throw runtime_error("Streams should not be copied!");
  }
  
  BufferedInputStream& operator=(BufferedInputStream&& other) {
    if (this != &other) {
      buffer = other.buffer;
      index = other.index;
      length = other.length;
      filename_ = other.filename_;
      
      other.buffer = nullptr;
    }
    
    return static_cast<BufferedInputStream&>(RWStream<T>::operator=(std::move(other)));
  }
  
	void open(string filename, uint64_t start, uint64_t end) {
		RWStream<T>::open(filename, start, end, Stream<T>::IN);
		this->filename_ = filename;
	}

	T read_next() {
		if (index==length) {
		  size_t size = sizeof(T)*length; //maybe min(length, rem)
		  int r = ::read(this->fd, buffer, size);
		  if (r == 0)
				throw runtime_error("Failed to read next entry from file (EOF or error).");
			index = 0;
		}
	  this->rem--;
		return buffer[index++];
	}

  bool end_of_stream() {
     return this->rem <= 0;
  }

void closeAndRemove() {
RWStream<T>::close();
if (filename_ != "tmp" && remove(this->filename_.c_str()) != 0) {
throw runtime_error("did not delete " + this->filename_ + " foo");
}
}
private:
        string filename_;
	T* buffer;
	uint64_t index;
	uint64_t length;
};
