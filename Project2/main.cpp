#include <iostream>

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  ExternalHeap<DummyStream<int>, int> foo(3);

  for (int i = 0; i < 10; ++i) {
    foo.insert(rand());
  }

  for (int i = 0; i < 10; ++i) {
    cout << DummyStream<int>::buffer_[i] << endl;
  }

  return 0;
}
