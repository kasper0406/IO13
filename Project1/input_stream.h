#pragma once

#include <string>
#include <cstdint>

#include "stream.h"

using namespace std;

template <typename T>
class InputStream : public virtual Stream<T> {
public:
  T read_next();
};