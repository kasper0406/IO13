#pragma once

#include "input_stream.h"
#include "rw_stream.h"

template <uint64_t B, typename T>
class BufferedOutputStream : public RWStream<T>, public InputStream<T> {
public:
	BufferedOutputStream() {
		this->buffer = new T[B];
		index = 0;
		length = B;
	}

	void open(string filename, uint64_t start, uint64_t end) {
		RWStream<T>::open(filename, start, end, Stream<T>::OUT);
	}

	void write(T value) {
		buffer[index++] = value;
		if (this->rem <= 0) throw runtime_error("Tried to write to full output stream!");
		
		if (index==length) {
		  size_t size = sizeof(T)*length;
		  if (::write(this->fd, buffer, size) != size)
				throw runtime_error("Failed to write element to output stream!");
			index = 0;
		}
		this->rem--;
  }

	void close() {
		if (index>0) {
			size_t size = sizeof(T)*index;
		  if (::write(this->fd, buffer, size) != size)
				throw runtime_error("Failed to close output stream!");
		}

    RWStream<T>::close();
	}

private:
	T* buffer;
	uint64_t index;
	uint64_t length;
};
