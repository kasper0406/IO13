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

template <typename T> using MMapIStream = MMapInputStream<4, T>;
template <typename T> using MMapOStream = MMapOutputStream<4, T>;

template <typename T> using BufferedIStream = BufferedInputStream<4, T>;
template <typename T> using BufferedOStream = BufferedOutputStream<4, T>;
#endif

using namespace std;

// TODO(lespeholt): Husk at bruge en ordentlig random generator. Standard
// versionen har en kort periode!

// TODO(knielsen): Overskriv fseek / lseek etc. med alternativer så alle
// systemer bruger 64 bit pointers. OS X gør dette auto, men win og linux
// gør det så vidt jeg kan læse kun med 32 bits.

int main(int argc, char *argv[]) {
  sanity_test<FREADInputStream, FWRITEOutputStream>();
#ifndef _WINDOWS
  sanity_test<ReadInputStream, WriteOutputStream>();
  sanity_test<BufferedIStream, BufferedOStream>();
  sanity_test<MMapIStream, MMapOStream>();
#endif
  
  const uint64_t elements = 8 * 1024 * 1024;
  
  /*
  {
    int n = 8;
    int k = 4;
    int cur = 0;

    generate_file<int>("input", [&]() {
       return cur++ % (n / k);
    }, n);

    FWRITEOutputStream<int> out;
    out.open("merged", 0, n);

    vector<FREADInputStream<int>> ins(k);
    for (int i = 0; i < k; ++i) {
      ins[i].open("input", i * (n / k), (i + 1) * (n / k));
    }

    IO13::merge<int, FWRITEOutputStream, FREADInputStream>(out, ins);

    for (auto& in : ins) {
      in.close();
    }

    out.close();
  }
   */
  
  const uint64_t N = 3517;
  const uint64_t M = 17;
  const uint64_t d = 5;
  
  /*
  int cur = 0;
  generate_file<int>("input", [&]() {
    return cur++;
  }, N);
   */
  
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
  
  /*test_reads<FREADInputStream>(elements);
  test_writes<FWRITEOutputStream>(elements);
#ifndef _WINDOWS
  test_reads<MMapIStream>(elements);
  test_writes<MMapOStream>(elements);
#endif*/
  
  cout << "File counter: " << counter << endl;
  
  return 0;
}
