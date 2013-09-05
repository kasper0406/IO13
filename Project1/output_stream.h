#pragma once

#include <string>
#include <cstdint>

#include "stream.h"

using namespace std;

template <typename T>
class OutputStream : public virtual Stream<T> {
public:
  void write(T value);
};