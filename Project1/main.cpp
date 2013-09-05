#include <iostream>
#include <cstdint>

#include "input_stream.h"
#include "output_stream.h"

#include "fread_input_stream.h"

using namespace std;

void perform_reads(uint32_t k, uint32_t n) {

}

int main(int argc, char *argv[]) {
  FREADInputStream<int> istream;
  istream.open("test.bin", 0);
  
  cout << istream.read_next() << endl;

  istream.close();
  
  return 0;
}
