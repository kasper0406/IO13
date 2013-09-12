#pragma once

#include "input_stream.h"
#include "rw_stream.h"

template <typename T>
class WriteOutputStream : public RWStream<T>, public InputStream<T> {
public:
	void open(string filename, uint64_t start, uint64_t end) {
		RWStream<T>::open(filename, start, end, Stream<T>::OUT);
	}

	void write(T value) {
    size_t size = sizeof(T);

		if (this->rem <= 0) throw runtime_error("Tried to write to full output stream!");

    if (::write(this->fd, &value, size) != size)
			throw runtime_error("Failed to write element to output stream!");

		this->rem--;
  }
};
