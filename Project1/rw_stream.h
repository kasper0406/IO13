#pragma once

#include "stream.h"
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>
#include <cerrno>

template<typename T>
class RWStream : public virtual Stream<T> {
public:
	void open(string filename, uint64_t start, uint64_t end, typename Stream<T>::Direction direction) {
		elements = end-start;
    rem = elements;
    
		fd = ::open(filename.c_str(),
								direction == Stream<T>::Direction::IN ? O_RDONLY : O_WRONLY | O_CREAT,
								S_IRUSR | S_IWUSR);

    if (fd == -1) {
      throw runtime_error("Could not open stream." + to_string(errno));

    }
		if (::lseek(fd, sizeof(T)*start, SEEK_SET) == -1) {
      throw runtime_error("Seek failed");
    }
	}

	virtual void close() {
		if (::close(fd) == -1) {
      throw runtime_error("Could not close stream");
    }
	}
  
  uint64_t size() const {
    return elements;
  }

protected:
	int fd;
	uint64_t rem;
  uint64_t elements;
};
