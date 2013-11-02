#include <iostream>
#include <ctime>
#include <fstream>
#include <utility>
#include <chrono>
#include "sys/wait.h"

#include "test.hpp"
#include "external_heap.hpp"
#include "dummy_stream.hpp"
#include "sys_stream.hpp"
#include "mmap_stream.hpp"
#include "cached_stream.hpp"
#include "f_stream.hpp"
#include "mmap_file_stream.hpp"
#include "buffered_stream.hpp"

using namespace std;
using namespace std::chrono;

template <typename S>
void client(size_t elements, size_t block_size, size_t buffer_size) {
  // TODO(lespeholt): Faa d til at vaere constructor parameter saa det kan testes lettere
  // TODO(lespeholt): Lav saerskilt block size og buffer size
  ExternalHeap<S, int, 2> heap("heap", buffer_size);

  // TODO(lespeholt): Skulle laeses fra en anden fil
  for (uint64_t i = 0; i < elements; ++i) {
    // TODO(lespeholt): Muligvis anden 'rand' funktion
    heap.insert(rand());
  }

  // TODO(lespeholt): Skulle skrives tilbage i fil
  int previous = numeric_limits<int>::max();
  for (uint64_t i = 0; i < elements; ++i) {
    if (previous < heap.peek_max()) {
      cout << "Error in heap" << endl;
      exit(3);
    }
    heap.extract_max();
  }
}

pair<int,	 string> exec(string cmd) {
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) {
    cout << "Not possible to execute command." << endl;
    exit(4);
  }
  char buffer[128];
  string result = "";
  while(!feof(pipe)) {
    if(fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  int status = 0;
  
  if (wait(&status) != -1) {
    pclose(pipe);
  } else {
    status = pclose(pipe);
  }
  bool exited = WIFEXITED(status);
  return {exited ? WEXITSTATUS(status) : WTERMSIG(status), result};
}

void server() {
  cout << "Test start" << endl << endl;
  
  // SET PARAMETERS HERE!
  string timeout_exec = "/usr/local/Cellar/coreutils/8.21/bin/gtimeout";
  int timeout_seconds = 1;
  int block_size = 10;
  int buffer_size = 10;
  int64_t elements_start = 128;
  int64_t elements_end = 4096 * 4096;
  
  for (int64_t elements = elements_start; elements <= elements_end; elements*=2) {
    string command = timeout_exec + " " + to_string(timeout_seconds) + " ./Project2Test client "
    + "\"elements:" + to_string(elements) + " block_size:" + to_string(block_size) + " buffer_size:"
    + to_string(buffer_size) + "\"";
    
    auto result = exec(command);
    
    if (result.first == 0) {
      cout << "Elements: " << elements << " Time: " << result.second << endl;
    } else if (result.first == 124) {
      cout << "Elements: " << elements << " Timeout" << endl;
      break;
    } else {
      cout << "Elements: " << elements << " Error" << endl;
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  if (argc > 2 && string(argv[1]) == "client") {
    int64_t elements;
    int block_size;
    int buffer_size;
    // TODO(lespeholt): Stream type
    if (sscanf(argv[2], "elements:%lli block_size:%i buffer_size:%i",
               &elements, &block_size, &buffer_size) != 3) {
      cerr << "Input matching failed." << endl;
      exit(10);
    };
    
    auto beginning = high_resolution_clock::now();
    
    client<DummyStream<int>>(elements, block_size, buffer_size);

    high_resolution_clock::duration duration = high_resolution_clock::now() - beginning;
  
    cout << duration_cast<milliseconds>(duration).count() / 1000. << endl;
  } else {
  #ifdef NDEBUG
    cout << "Release mode" << endl;
  #else
    cout << "Debug mode" << endl;
  #endif
    
    server();
  }
  
  return 0;
}
