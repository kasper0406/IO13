#include <iostream>

#include "test.h"

#include "fread_input_stream.h"
#include "fwrite_output_stream.h"
#include "merge_sort.h"

#ifndef _WINDOWS
#include "mmap_input_stream.h"
#include "mmap_output_stream.h"
#include "read_input_stream.h"
#include "write_output_stream.h"
#include "buffered_input_stream.h"
#include "buffered_output_stream.h"

constexpr uint64_t B = 1 << 21;
template <typename T> using MMapIStream = MMapInputStream<B, T>;
template <typename T> using MMapOStream = MMapOutputStream<B, T>;

template <typename T> using BufferedIStream = BufferedInputStream<B, T>;
template <typename T> using BufferedOStream = BufferedOutputStream<B, T>;
#endif

using namespace std;

// TODO(lespeholt): Husk at bruge en ordentlig random generator. Standard
// versionen har en kort periode!

// TODO(knielsen): Overskriv fseek / lseek etc. med alternativer så alle
// systemer bruger 64 bit pointers. OS X gør dette auto, men win og linux
// gør det så vidt jeg kan læse kun med 32 bits.

/*
void kasper_test() {
  const uint64_t N = 3517;
  const uint64_t M = 17;
  const uint64_t d = 5;
  
  generate_file<uint32_t>("input", random_uint32, N);
  
  cout << "Before sort:" << endl;
  FREADInputStream<uint32_t> reader;
  reader.open("input", 0, N);
  for (int i = 0; !reader.end_of_stream(); i++) {
    cout << reader.read_next() << "\t";
    if (i % M == M - 1)
      cout << endl;
  }
  cout << endl;
  reader.close();
  
  // IO13::sort<FREADInputStream, FWRITEOutputStream, uint32_t, M, d>(N, "input");
  IO13::sort<MMapIStream, MMapOStream, uint32_t, M, d>(N, "input");
  // IO13::sort<ReadInputStream, WriteOutputStream, uint32_t, M, d>(N, "input");
  // IO13::sort<BufferedIStream, BufferedOStream, uint32_t, M, d>(N, "input");

  cout << endl << "After sort:" << endl;
  reader.open("input", 0, N);
  uint32_t cur = 0;
  for (int i = 0; !reader.end_of_stream(); i++) {
    uint32_t val  =reader.read_next();
    if (val < cur)
      cerr << "ERROR!" << endl;
    cur = val;
    
    cout << val << "\t";
    if (i % M == M - 1)
      cout << endl;
  }
  cout << endl;
  reader.close();
  
  cout << "File counter: " << counter << endl;
}*/

template <size_t MinB, size_t MaxB, size_t elements> // Input skal vaere paa formen 2^b
class BufferTest {
public:
  static void run() {
    run<MaxB>();
  }

private:
  template <size_t B>
  static void run() {
    if (B / 2 >= MinB) {
      run<B / 2>();
    }

    // test_reads<MMapInputStream<B, uint32_t>>(elements);
    // test_writes<MMapOutputStream<B, uint32_t>>(elements);

    test_reads<BufferedInputStream<B, uint32_t>>(elements);
    test_writes<BufferedOutputStream<B, uint32_t>>(elements);
  }
};

int main(int argc, char *argv[]) {
  sanity_test<FREADInputStream, FWRITEOutputStream>();
  sanity_test<ReadInputStream, WriteOutputStream>();
  sanity_test<BufferedIStream, BufferedOStream>();
  sanity_test<MMapIStream, MMapOStream>();
  
  const uint64_t elements = 1024 * 1024 * 32 / 4;

  // kasper_test();

  // Buffer test

  // BufferTest<1024 * 1024, elements / 2, elements>::run();

  // Read/write test

  // test_reads<ReadInputStream<uint32_t>>(elements);
  test_writes<WriteOutputStream<uint32_t>>(elements);

  // test_reads<FREADInputStream<uint32_t>>(elements);
  // test_writes<FWRITEOutputStream<uint32_t>>(elements);
    
  // test_sort<BufferedIStream, BufferedOStream>();

  cout << "File counter: " << counter << endl;

  return 0;
}
