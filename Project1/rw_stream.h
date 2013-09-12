#pragma once

#include "stream.h"
#include <fcntl.h>
#include <cstdio>
#include <unistd.h>

template<typename T>
class RWStream : public virtual Stream<T> {
public:
	void open(string filename, uint64_t start, uint64_t end, typename Stream<T>::Direction direction) {
		elements = end-start;
    rem = elements;
    
		fd = ::open(filename.c_str(),
								direction == Stream<T>::Direction::IN ? O_RDONLY : O_WRONLY | O_CREAT,
								S_IRUSR | S_IWUSR);
		::lseek(fd, sizeof(T)*start, SEEK_SET);
	}

	virtual void close() {
		::close(fd);
	}
  
  uint64_t size() const {
    return elements;
  }

protected:
	int fd;
	uint64_t rem;
  uint64_t elements;
};
