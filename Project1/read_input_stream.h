#pragma once

#include "input_stream.h"
#include "rw_stream.h"
#include <stdexcept>

template <typename T>
class ReadInputStream : public RWStream<T>, public InputStream<T> {
public:
	void open(string filename, uint64_t start, uint64_t end) {
		RWStream<T>::open(filename, start, end, Stream<T>::IN);
	}

	T read_next() {
    size_t size = sizeof(T);
    T res;
    int r = ::read(this->fd, &res, size);
    if (r != size)
			throw runtime_error("Failed to read next entry from file (EOF or error).");
    this->rem--;
    return res;
	}

  bool end_of_stream() {
     return this->rem <= 0;
  }
};
