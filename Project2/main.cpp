#include <iostream>
#include <ctime>

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  srand(time(NULL));

  ExternalHeap<DummyStream<int>, int> foo(3);

  for (int i = 0; i < 10; ++i) {
    foo.insert(rand());
  }

  cout << "Rand max: " << RAND_MAX << endl;
  for (int i = 0; i < 10; ++i) {
    cout << DummyStream<int>::buffer_[i] << endl;
  }

  return 0;
}
