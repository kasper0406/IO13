#include <iostream>
#include <ctime>
#include <fstream>

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  srand(time(NULL));

  ExternalHeap<DummyStream<int>, int, 3> foo(3);

  for (int i = 0; i < 100; ++i) {
    foo.insert(i); // rand());
    if (i % 3 == 0)
      foo.extract_max();
  }
  ofstream dot("heap.dot");
  dot << foo.to_dot();
  dot.close();

  cout << "Rand max: " << RAND_MAX << endl;
  for (int i = 0; i < 10; ++i) {
    cout << DummyStream<int>::buffer_[i] << endl;
  }

  return 0;
}
