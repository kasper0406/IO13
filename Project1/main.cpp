#include <iostream>

#include "utils.h"

using namespace std;

int main(int argc, char *argv[]) {
  cout << "foo";

  generate_file<uint32_t>("foo.txt", random_uint32, 10);

  return 0;
}
