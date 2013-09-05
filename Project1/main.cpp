#include <iostream>

#include "utils.h"

#include "fread_input_stream.h"

using namespace std;

int main(int argc, char *argv[]) {
  FREADInputStream<int> istream;
  istream.open("test.bin", 0, 10);

  while (!istream.end_of_stream())
    cout << istream.read_next() << endl;

  istream.close();
  
  /*
  int counter = 1;
  generate_file<uint32_t>("test.bin", [&counter]() { return counter++; }, 10);
   */

  return 0;
}
