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
// typedef CachedStream<int, FStream<int>> TestStream;
typedef CachedStream<int, BufferedStream> TestStream;
// typedef CachedStream<int, FStream> TestStream;

void resize_test() {
  ExternalHeap<FStream<int>, int> foo("resize_heap", 5, 3, 3, 3);
  for (int i = 0; i < 11; ++i) {
    foo.insert(100);
  }
}

void test_swap_sift_test() {
  ExternalHeap<TestStream, int> foo("heap", 5, 3, 3, 3);
  
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
  
  ExternalHeap<SysStream<int>, int> foo("heap3", 4, 2, 2, 3);
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
  ExternalHeap<TestStream, int> foo("heap2", 1024, 2, 32, 4);
  const uint64_t N = 1000;
  
  for (int i = 0; i < N; ++i) {
    foo.insert(rand());
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
  MMapStream<uint64_t> stream(0);
  
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
  
  vector<BufferedStream<int>> streams(stream_count, BufferedStream<int>(0));
  
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
  
  BufferedStream<int> stream(0);
  
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
void simple_sanity_test(size_t buffer_size = 0, size_t cache_size = 128) {
  fstream create("monkey", fstream::out | fstream::binary);
  if (!create.is_open()) {
    cout << "Could not create file" << endl;
    exit(1);
  }
  int value = 1;
  create.write(reinterpret_cast<const char*>(&value), sizeof(int));
  create.close();
  
  S stream(cache_size);
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

  if (stream.position() != 1) {
    cout << "Error position" << endl;
    exit(1);
  }

  stream.seek(0);

  if (stream.read_prev() != 42) {
    cout << "Error read_back" << endl;
    exit(1);
  }

  if (stream.position() != -1) {
    cout << "Error position" << endl;
    exit(1);
  }

  stream.seek(0);

  stream.backward_write(41);

  if (stream.position() != -1) {
    cout << "Error position" << endl;
    exit(1);
  }

  stream.close();

  S stream2(cache_size);
  stream2.open("monkey", 0, 1, buffer_size);

  if (stream2.has_next() && stream2.read_next() != 41) {
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
  auto tell = filesize.tellg();
  if (tell != 4) {
    cout << "Wrong filesize" << endl;
    exit(1);
  };
  filesize.close();
  
  S stream3(cache_size);
  stream3.open("monkey", 0, 4, buffer_size);
  stream3.write(0);
  stream3.write(1);
  stream3.write(2);
  stream3.write(3);
  stream3.close();
  
  S stream4(cache_size);
  stream4.open("monkey", 0, 4, buffer_size);
  if (stream4.read_next() != 0
      || stream4.read_next() != 1
      || stream4.read_next() != 2
      || stream4.read_next() != 3) {
    cout << "Wrong result" << endl;
    exit(1);
  }
  stream4.close();
  
  S stream5(cache_size);
  stream5.open("monkey", 0, 3, buffer_size);
  stream5.write(42);
  stream5.seek(2);
  stream5.write(43);
  stream5.close();
  
  stream5.open("monkey", 2, 4, buffer_size);
  stream5.seek(3);
  stream5.write(44);
  stream5.close();
  
  S stream6(cache_size);
  stream6.open("monkey", 0, 4, buffer_size);
  if (stream6.read_next() != 42) {
    cout << "Wrong result" << endl;
    exit(1);
  }
  stream6.seek(3);
  if (stream6.read_next() != 44) {
    cout << "Wrong result" << endl;
    exit(1);
  }
  stream6.close();
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
  // simple_sanity_test<MMapStream<int>>();
  simple_sanity_test<SysStream<int>>();
  simple_sanity_test<BufferedStream<int>>(1);
  simple_sanity_test<BufferedStream<int>>(2);  
  simple_sanity_test<BufferedStream<int>>(3);
  simple_sanity_test<BufferedStream<int>>(4);
  // simple_sanity_test<CachedStream<int, DummyStream, 10>>();
  // simple_sanity_test<CachedStream<int, MMapStream, 10>>();
  simple_sanity_test<CachedStream<int, FStream>>();
  simple_sanity_test<CachedStream<int, BufferedStream>>(2);

  // resize_test();
  lasse_test();
  kasper_test();
  //mmap_file_stream_test();
  
  return 0;
}
