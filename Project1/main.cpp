#include <iostream>

#include "test.h"

#include "fread_input_stream.h"
#include "fwrite_output_stream.h"

using namespace std;

int main(int argc, char *argv[]) {
  int counter = 1;
  generate_file<uint32_t>("test.bin", [&counter]() { return counter++; }, 10);
  
  /*FREADInputStream<int> istream;
  istream.open("test.bin", 0, 10);
  while (!istream.end_of_stream())
    cout << istream.read_next() << endl;
  istream.close();*/
  
  FWRITEOutputStream<int> ostream;
  ostream.open("test.bin", 5, 10);
  ostream.write(42);
  ostream.write(1337);
  ostream.close();

  istream.open("test.bin", 0, 10);
  while (!istream.end_of_stream())
    cout << istream.read_next() << endl;
  istream.close();
   
  test_reads<FREADInputStream>(8 * 1024 * 1024);

  return 0;
}
