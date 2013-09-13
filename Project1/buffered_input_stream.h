#pragma once

#include "input_stream.h"
#include "rw_stream.h"
#include <stdexcept>
#include <algorithm>

template <uint64_t B, typename T>
class BufferedInputStream : public RWStream<T>, public InputStream<T> {
public:
	BufferedInputStream() {
		this->buffer = new T[B];
		index = length = B;
	}

	void open(string filename, uint64_t start, uint64_t end) {
		RWStream<T>::open(filename, start, end, Stream<T>::IN);
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
private:
	T* buffer;
	uint64_t index;
	uint64_t length;
};