#include <iostream>

#include "utils.h"

#include "fread_input_stream.h"

using namespace std;

int main(int argc, char *argv[]) {
  /*
  FREADInputStream<int> istream;
  istream.open("test.bin", 0);

  cout << istream.read_next() << endl;

  istream.close();
   */
  
  generate_file<uint32_t>("test.bin", random_uint32, 10);

  return 0;
}
