#pragma once

#include "output_stream.h"
#include "f_stream.h"

template<typename T>
class FWRITEOutputStream : public FStream<T>, public OutputStream<T> {
  
};
