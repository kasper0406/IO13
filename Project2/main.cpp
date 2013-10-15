#include <iostream>
#include <ctime>
#include <fstream>

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  srand(time(NULL));

  ExternalHeap<DummyStream<int>, int, 10> foo(10);

  for (int i = 0; i < 1000; ++i) {
    foo.insert(i * 977 % 1000); // rand());
  }
  ofstream before("heap_before.dot");
  before << foo.to_dot();
  before.close();
  
  for (int i = 0; i < 1000; i++) {
    /*
    if (i == 60) {
      ofstream intermediate("heap_intermediate_before.dot");
      intermediate << foo.to_dot();
      intermediate.close();
    }
     */
    
    foo.extract_max();
    
    /*
    if (i == 60) {
      ofstream intermediate("heap_intermediate_after.dot");
      intermediate << foo.to_dot();
      intermediate.close();
    }
     */
  }
  
  ofstream after("heap_after.dot");
  after << foo.to_dot();
  after.close();

  cout << "Rand max: " << RAND_MAX << endl;
  for (int i = 0; i < 10; ++i) {
    cout << DummyStream<int>::buffer_[i] << endl;
  }

  return 0;
}
