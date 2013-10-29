#include <iostream>
#include <ctime>
#include <fstream>

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"
#include "sys_stream.hpp"
#include "mmap_stream.hpp"
#include "cached_stream.hpp"

using namespace std;

// typedef DummyStream<int> TestStream;
typedef MMapStream<int> TestStream;

void resize_test() {
  ExternalHeap<TestStream, int, 3> foo("resize_heap", 5);
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

void kasper_test() {
  ExternalHeap<TestStream, int, 1024> foo("heap2", 1024);

  const uint64_t N = 1000000;
  for (int i = 0; i < N; ++i) {
    foo.insert(i * 977 % N); // rand());
  }
  
  /*
  ofstream before("heap_before.dot");
  before << foo.to_dot();
  before.close();
   */
  
  uint64_t prev = numeric_limits<uint64_t>::max();
  for (int i = 0; i < N; i++) {
    if (prev < foo.peek_max())
      throw runtime_error("ERROR!");
    
    assert(prev >= foo.peek_max());
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

void cached_stream_test() {
  const uint64_t N = 100;
  CachedStream<uint64_t, MMapStream<uint64_t>, 7> stream;
  
  // Write some content
  stream.open("test.bin", 0, N, 0);
  for (int i = 0; i < N; i++)
    stream.write(i);
  stream.close();
  
  // Read from the stream, closing it after every read
  for (int i = 0; i < N; i++) {
    stream.open("test.bin", 0, N, 0);
    stream.seek(i);
    cout << stream.read_next() << endl;
    stream.close();
  }
}

int main(int argc, char *argv[]) {
#ifdef NDEBUG
  cout << "Release mode" << endl;
#else
  cout << "Debug mode" << endl;
#endif
  
  srand(time(NULL));

  // resize_test();
  // kasper_test();
  cached_stream_test();
  // test_swap_sift_test();
  // mmap_stream_test();
  
  return 0;
}
