#include <iostream>
#include <ctime>
#include <fstream>
#include <utility>
#include <chrono>
#include "sys/wait.h"
#include <iomanip>
#include <cstdlib>

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
void client(size_t elements, size_t block_size, size_t buffer_size, size_t d) {
  ExternalHeap<S, int> heap("heap", block_size, buffer_size, d);

  S stream;
  stream.open("testfile", 0, elements, buffer_size);

  for (uint64_t i = 0; i < elements; ++i) {
    heap.insert(stream.read_next());
  }

  stream.seek(0);

  int previous = numeric_limits<int>::max();
  for (uint64_t i = 0; i < elements; ++i) {
    int current = heap.peek_max();
    if (previous < current) {
      cout << "Error in heap" << endl;
      exit(3);
    }
    stream.write(current);
    heap.extract_max();
    previous = current;
  }

  stream.close();
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

  cout.precision(3);
  cout.setf(ios::fixed, ios::floatfield); 
  cout << setw(12) << "Elements";
  cout << setw(12) << "Block size";
  cout << setw(12) << "Buffer size";
  cout << setw(12) << "Time";
  cout << endl;

  // SET PARAMETERS HERE!
  string timeout_exec = "/usr/local/Cellar/coreutils/8.21/bin/gtimeout";
  int timeout_seconds = 1;
  int block_size_start = 128;
  int block_size_end = 128 * 128;
  int buffer_size_start = 64;
  int buffer_size_end = 64 * 64;
  int64_t elements_start = 128;
  int64_t elements_end = 4096 * 4096;
  
  for (int block_size = block_size_start; block_size <= block_size_end; block_size*=2) {
    for (int buffer_size = buffer_size_start; buffer_size <= min(buffer_size_end, block_size); buffer_size*=2) {
      for (int64_t elements = elements_start; elements <= elements_end; elements*=2) {
        string command = timeout_exec + " " + to_string(timeout_seconds) + " ./Project2Test client "
        + "\"elements:" + to_string(elements) + " block_size:" + to_string(block_size) + " buffer_size:"
        + to_string(buffer_size) + "\"";
        
        auto result = exec(command);
        
        cout << setw(12) << elements;
        cout << setw(12) << block_size;
        cout << setw(12) << buffer_size;

        if (result.first == 0) {
          double seconds = atof(result.second.c_str());
          cout << setw(12) << seconds << endl;
        } else if (result.first == 124) {
          cout << setw(12) << "Timeout" << endl;
          break;
        } else {
          cout << setw(12) << "Error" << endl;
          break;
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  if (argc > 2 && string(argv[1]) == "client") {
    int64_t elements;
    int block_size;
    int buffer_size;
    int d = 2;
    // TODO(lespeholt): Stream type og d
    if (sscanf(argv[2], "elements:%lli block_size:%i buffer_size:%i",
               &elements, &block_size, &buffer_size) != 3) {
      cerr << "Input matching failed." << endl;
      exit(10);
    };

    typedef FStream<int> Stream;

    // TODO(lespeholt): Maybe cache file and write it faster!
    fstream create("testfile", fstream::out | fstream::binary);
    if (!create.is_open()) {
      cout << "Could not create file" << endl;
      exit(1);
    }

    for (int64_t i = 0; i < elements; ++i) {
      int value = rand();  // TODO(lespeholt): Maybe other rand function
      create.write(reinterpret_cast<const char*>(&value), sizeof(int));
    }

    create.close();
    
    auto beginning = high_resolution_clock::now();
    
    client<Stream>(elements, block_size, buffer_size, d);

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
