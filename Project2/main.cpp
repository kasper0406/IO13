#include <iostream>

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  ExternalHeap<int, int> foo(10);
  DummyStream<int> bar;

  return 0;
}
