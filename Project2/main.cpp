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
    foo.insert(i * 23 % 100); // rand());
  }
  ofstream before("heap_before.dot");
  before << foo.to_dot();
  before.close();
  
  for (int i = 0; i < 6; i++)
    foo.extract_max();
  
  ofstream after("heap_after.dot");
  after << foo.to_dot();
  after.close();

  cout << "Rand max: " << RAND_MAX << endl;
  for (int i = 0; i < 10; ++i) {
    cout << DummyStream<int>::buffer_[i] << endl;
  }

  return 0;
}
