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

constexpr uint64_t B = 1024 * 1024 * 8 / 4;//(uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)4;
template <typename T> using MMapIStream = MMapInputStream<B, T>;
template <typename T> using MMapOStream = MMapOutputStream<B, T>;

template <typename T> using BufferedIStream = BufferedInputStream<B, T>;
template <typename T> using BufferedOStream = BufferedOutputStream<B, T>;
#endif

using namespace std;

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
    if (B / 4 >= MinB) {
      run<B / 4>();
    }

    //test_reads<MMapInputStream<B, uint32_t>>(elements);
    test_writes<MMapOutputStream<B, uint32_t>>(elements);

    //test_reads<BufferedInputStream<B, uint32_t>>(elements);
    test_writes<BufferedOutputStream<B, uint32_t>>(elements);
  }
};

void lasse_mmap_test() {
  string filename = "mmap_test_file";
  const uint64_t elements = 1024 * 1024 * 64;
  const int times = 1000000;
  const int trials = 5;

  generate_file<uint32_t>(filename, random_uint32, elements);

  auto small_test = [&]() {
    for (int i = 0; i < times; ++i) {
    MMapInputStream<1024*4, uint32_t> s;
    s.open(filename, 0, 1024*4);
    s.remap();
    s.close();
    }
  };

  auto large_test = [&]() {
    for (int i = 0; i < times; ++i) {
    MMapInputStream<elements, uint32_t> s;
    s.open(filename, 0, elements);
    s.remap();
    s.close();
    }
  };

  measure(cout, "Small area", trials, small_test);
  measure(cout, "Large area", trials, large_test);
}

void lasse_mmap_test2() {
  const string filename = "mmap_test_file";
  const uint64_t elements = B;
  
  generate_file<uint32_t>(filename, []() { return 0; }, elements);
  
  cout << "Generated file" << endl;
  
  uint64_t block_size = B / 128;
  int i = 0;
  for (int block = 0; block < (B / block_size); block++) {
    measure(cout, "MMAP write test", 1, [filename, block_size, &i]() {
      MMapOutputStream<B, uint32_t> s;
      s.open(filename, 0, elements);
      
      for (int j = 0; j < block_size; j++) {
        i++;
        s.write(j % 13);
      }
      
      s.close();
    });
  }
}

int main(int argc, char *argv[]) {
  /*sanity_test<FREADInputStream, FWRITEOutputStream>();
  sanity_test<ReadInputStream, WriteOutputStream>();
  sanity_test<BufferedIStream, BufferedOStream>();*/
  sanity_test<MMapIStream, MMapOStream>();
  //lasse_mmap_test2();
  
  // const uint64_t elements = 1024 * 1024 * 1024 / 128;  // 1 GB
  //const uint64_t elements = 1024 * 1024 * 1024 / 16;

  const uint64_t elements = (uint64_t)(1024 * 1024 * 1024) *  (uint64_t)4; // 1 GB

  /*for (int k = 1; k <= 256; k *= 4) {
    // if (k == 16) continue;
    // test_reads_multiple_files<MMapIStream<uint32_t>>(elements, k);
    // test_reads_multiple_files<BufferedIStream<uint32_t>>(elements, k);

    // test_writes_multiple_files<MMapOStream<uint32_t>>(elements, k);
    test_writes_multiple_files<BufferedOStream<uint32_t>>(elements, k);
  }*/
  
  test_writes<MMapOStream<uint32_t>>(elements);

  //lasse_mmap_test();

  // kasper_test();

  // Buffer test

  //BufferTest<2048, 8 * 1024 * 1024, elements>::run();

  // Read/write test

  //test_reads<ReadInputStream<uint32_t>>(elements);
  //test_writes<WriteOutputStream<uint32_t>>(elements);

  //test_reads<FREADInputStream<uint32_t>>(elements);
  //test_writes<FWRITEOutputStream<uint32_t>>(elements);
    
  // test_sort<MMapIStream, MMapOStream>();

  //test_heapsort();

  cout << "File counter: " << counter << endl;

  return 0;
}
