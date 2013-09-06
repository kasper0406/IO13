#include <iostream>

#include "test.h"

#include "fread_input_stream.h"
#include "fwrite_output_stream.h"

#ifndef _WINDOWS
#include "mmap_input_stream.h"
#include "mmap_output_stream.h"

template <typename T> using MMapIStream = MMapInputStream<1024, T>;
template <typename T> using MMapOStream = MMapOutputStream<1024, T>;
#endif

using namespace std;

int main(int argc, char *argv[]) {
  sanity_test<FREADInputStream, FWRITEOutputStream>();
  
  const uint64_t elements = 8 * 1024 * 1024;
  
  test_reads<FREADInputStream>(elements);
  test_writes<FWRITEOutputStream>(elements);
#ifndef _WINDOWS
  test_reads<MMapIStream>(elements);
  test_writes<MMapOStream>(elements);
#endif
    
  return 0;
}
