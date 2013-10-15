#include <iostream>
#include <ctime>
#include <fstream>

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"

using namespace std;

void test_swap_sift_test() {
  ExternalHeap<DummyStream<int>, int, 3> foo(5);

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
  ExternalHeap<DummyStream<int>, int, 100> foo(100);

  const uint64_t N = 10000000;
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

int main(int argc, char *argv[]) {
#ifdef NDEBUG
  cout << "Release mode" << endl;
#else
  cout << "Debug mode" << endl;
#endif
  
  srand(time(NULL));

  kasper_test();
  test_swap_sift_test();

  return 0;
}
