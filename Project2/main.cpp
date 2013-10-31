#include <iostream>
#include <ctime>
#include <fstream>

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"
#include "sys_stream.hpp"
#include "mmap_stream.hpp"
#include "cached_stream.hpp"
#include "f_stream.hpp"
#include "mmap_file_stream.hpp"
#include "buffered_stream.hpp"

using namespace std;

// typedef DummyStream<int> TestStream;
// typedef FStream<int> TestStream;
// typedef MMapStream<int> TestStream;
// typedef MMapFileStream<int> TestStream;
// typedef CachedStream<int, FStream<int>, 128> TestStream;
// typedef CachedStream<int, MMapStream, 128> TestStream;
typedef FStream<int> TestStream;

void resize_test() {
  ExternalHeap<FStream<int>, int, 3> foo("resize_heap", 5);
  for (int i = 0; i < 11; ++i) {
    foo.insert(100);
  }
}

void test_swap_sift_test() {
  ExternalHeap<TestStream, int, 3> foo("heap", 5);
  
  // Full tree
  for (int i = 0; i < 5; ++i) {
    foo.insert(100);
  }
  for (int i = 0; i < 16; ++i) {
    foo.insert(20 - i);
  }
  {
    ofstream before("heap0.dot");
    before << foo.to_dot();
    before.close();
  }
  
  for (int i = 0; i < 4; ++i) {
    foo.extract_max();
  }
  {
    ofstream before("heap1.dot");
    before << foo.to_dot();
    before.close();
  }
  
  for (int i = 0; i < 5; ++i) {
    foo.insert(0);
  }
  {
    ofstream before("heap2.dot");
    before << foo.to_dot();
    before.close();
  }
}

void lasse_test() {
  cout << "Lasse test" << endl;
  
  ExternalHeap<SysStream<int>, int, 2> foo("heap3", 4);
  foo.insert(10);
  foo.insert(10);
  foo.insert(8);
  foo.insert(8);
  foo.insert(5);
  
  assert(foo.peek_max() == 10);
  foo.extract_max();
  
  {
    ofstream before("heap3_before.dot");
    before << foo.to_dot();
    before.close();
  }
  
  foo.insert(5);
  foo.insert(5);
  
  {
    ofstream before("heap3_after.dot");
    before << foo.to_dot();
    before.close();
  }

  assert(foo.peek_max() == 10);
  foo.extract_max();
  assert(foo.peek_max() == 8);
  foo.extract_max();
  assert(foo.peek_max() == 8);
  foo.extract_max();
  assert(foo.peek_max() == 5);
  foo.extract_max();
  assert(foo.peek_max() == 5);
  foo.extract_max();
  assert(foo.peek_max() == 5);
  foo.extract_max();
}

void kasper_test() {
  cout << "Kasper test" << endl;
  ExternalHeap<TestStream, int, 8> foo("heap2", 8);
  const uint64_t N = 1000;
  
  for (int i = 0; i < N; ++i) {
    foo.insert(N - i);
  }
  
  /*
  ofstream before("heap_before.dot");
  before << foo.to_dot();
  before.close();
   */
  
  uint64_t prev = numeric_limits<uint64_t>::max();
  for (int i = 0; i < N; i++) {
    if (prev < foo.peek_max()) {
      cerr << "Wrong value! prev: " << prev << "\tcur: " << foo.peek_max() << endl;
      // throw runtime_error("ERROR!");
    }
    
    prev = foo.peek_max();
    foo.extract_max();
  }
  
  /*
  ofstream after("heap_after.dot");
  after << foo.to_dot();
  after.close();
   */
}

void mmap_stream_test() {
  // MMap stream test
  MMapStream<uint64_t> stream;
  
  // Write some content
  stream.open("test.bin", 0, 10, 0);
  for (int i = 0; i < 10; i++)
    stream.write(i + 1);
  stream.close();
  
  // Read the same content
  stream.open("test.bin", 0, 10, 0);
  while (stream.has_next()) {
    uint64_t peek = stream.peek();
    uint64_t read = stream.read_next();
    assert(peek == read);
    cout << read << endl;
  }
  stream.close();
}

void mmap_file_stream_test() {
  const int stream_count = 3;
  const string filename = "test.bin";
  const int B = 8;
  
  vector<BufferedStream<int>> streams(stream_count, BufferedStream<int>());
  
  // Write test
  for (int i = 0; i < stream_count; i++) {
    streams[i].open(filename, i * B, (i + 1) * B, 4);
  }
  
  for (int k = 0; k < B; k++) {
    for (int i = 0; i < stream_count; i++) {
      streams[i].write(k + i * B);
    }
  }
  
  for (int i = 0; i < stream_count; i++) {
    streams[i].close();
  }
  
  // Read test
  for (int i = 0; i < stream_count; i++) {
    streams[i].open(filename, i * B, (i + 1) * B, 4);
  }
  
  for (int k = 0; k < B; k++) {
    for (int i = 0; i < stream_count; i++) {
      cout << streams[i].read_next() << "\t";
    }
    cout << endl;
  }
  
  for (int i = 0; i < stream_count; i++) {
    streams[i].close();
  }
  
  MMapFileStream<int>::cleanup();
}

void cached_stream_test() {
  const uint64_t N = 4096;
  const uint64_t start = 2 * N;
  
  BufferedStream<int> stream;
  
  // Write some content
  stream.open("test.bin", start, start + N, 100);
  for (int i = 0; i < N; i++)
    stream.write(i);
  stream.close();
  
  // Read from the stream, closing it after every read
  for (int i = 0; i < N; i++) {
    stream.open("test.bin", start, start + N, 100);
    stream.seek(i + start);
    cout << stream.read_next() << endl;
    stream.close();
  }
}

template <typename S>
void simple_sanity_test(size_t buffer_size = 0) {
  fstream create("monkey", fstream::out | fstream::binary);
  if (!create.is_open()) {
    cout << "Could not create file" << endl;
    exit(1);
  }
  int value = 1;
  create.write(reinterpret_cast<const char*>(&value), sizeof(int));
  create.close();
  
  S stream;
  stream.open("monkey", 0, 1, buffer_size);
  
  if (!stream.has_next()) {
    cout << "Error has_next" << endl;
    exit(1);
  }

  int read = stream.read_next();
  if (read != 1) {
    cout << "Error read_next" << endl;
    exit(1);
  }

  stream.seek(0);
  
  stream.write(42);
  
  if (stream.has_next()) {
    cout << "Error has_next/write" << endl;
    exit(1);
  }
  
  stream.seek(0);
  
  if (!stream.has_next()) {
    cout << "Error has_next/seek" << endl;
    exit(1);
  }
  
  if (stream.peek() != 42) {
    cout << "Error peek" << endl;
    exit(1);
  }
  
  if (stream.read_next() != 42) {
    cout << "Error read_next" << endl;
    exit(1);
  }
  
  if (stream.has_next()) {
    cout << "Error has_next/read_next " << endl;
    exit(1);
  }
  
  stream.close();

  S stream2;
  stream2.open("monkey", 0, 1, buffer_size);

  if (stream2.has_next() && stream2.read_next() != 42) {
    cout << "Error, buffer not flushed?" << endl;
    exit(1);
  }

  stream2.close();
  
  fstream filesize("monkey", fstream::in | fstream::binary);
  if (!filesize.is_open()) {
    cout << "Could not open file" << endl;
    exit(1);
  }
  filesize.seekg(0, ifstream::end);
  if (filesize.tellg() != 4) {
    cout << "Wrong filesize" << endl;
    exit(1);
  };
  filesize.close();
}

int main(int argc, char *argv[]) {
#ifdef NDEBUG
  cout << "Release mode" << endl;
#else
  cout << "Debug mode" << endl;
#endif
  
  srand(time(NULL));
  
//cached_stream_test();

  // simple_sanity_test<DummyStream<int>>(); Fails sanity check because it doesn't read a real file
  simple_sanity_test<FStream<int>>();
  simple_sanity_test<MMapStream<int>>();
  simple_sanity_test<SysStream<int>>();
  simple_sanity_test<BufferedStream<int>>(2);  
  simple_sanity_test<BufferedStream<int>>(3);  
  simple_sanity_test<BufferedStream<int>>(4);  
  // simple_sanity_test<CachedStream<int, DummyStream, 10>>();
  simple_sanity_test<CachedStream<int, MMapStream, 10>>();
  simple_sanity_test<CachedStream<int, FStream, 10>>();
  simple_sanity_test<CachedStream<int, BufferedStream, 10>>(2);

  // resize_test();
  lasse_test();
  kasper_test();
  //mmap_file_stream_test();
  
  return 0;
}
