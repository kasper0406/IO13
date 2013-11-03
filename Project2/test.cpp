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
    heap.insert(stream.read_next(), true);
  }

  // heap.sift_all();

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
  cout << setw(12) << "Stream type";
  cout << setw(12) << "Elements";
  cout << setw(12) << "Block size";
  cout << setw(12) << "d";
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
  int d_start = 2;
  int d_end = 4096;
  int64_t elements_start = 128;
  int64_t elements_end = 4096 * 4096;
  vector<char> stream_types { 'f', 's', 'b', 'm' };
  
  for (char stream_type : stream_types) {
    for (int block_size = block_size_start; block_size <= block_size_end; block_size*=2) {
      for (int d = d_start; d <= d_end; d*=2) {
        for (int buffer_size = (stream_type == 'b' ? buffer_size_start : 0);
             buffer_size <= (stream_type == 'b' ? min(buffer_size_end, block_size) : 0); buffer_size= max(1, buffer_size * 2)) {
          for (int64_t elements = max(elements_start, (int64_t)block_size); elements <= elements_end; elements*=2) {
            if (d * block_size > elements) continue;

            string command = timeout_exec + " " + to_string(timeout_seconds) + " ./Project2Test client "
            + "\"elements:" + to_string(elements) + " block_size:" + to_string(block_size) + " buffer_size:"
            + to_string(buffer_size) + " d:" + to_string(d) + " stream:" + stream_type + "\"";
            
            auto result = exec(command);
            
            cout << setw(12) << stream_type;
            cout << setw(12) << elements;
            cout << setw(12) << block_size;
            cout << setw(12) << d;
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
  }
}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  if (argc > 2 && string(argv[1]) == "client") {
    int64_t elements;
    int block_size;
    int buffer_size;
    int d;
    char stream_type;
    if (sscanf(argv[2], "elements:%lli block_size:%i buffer_size:%i d:%i stream:%c",
               &elements, &block_size, &buffer_size, &d, &stream_type) != 5) {
      cerr << "Input matching failed." << endl;
      exit(10);
    };


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
    switch (stream_type) {
      case 'f':
        client<FStream<int>>(elements, block_size, buffer_size, d);
        break;

      case 's':
        client<SysStream<int>>(elements, block_size, buffer_size, d);
        break;

      case 'b':
        client<BufferedStream<int>>(elements, block_size, buffer_size, d);
        break;

      case 'm':
        client<MMapFileStream<int>>(elements, block_size, buffer_size, d);
        break;

      default:
        cout << "Unknown stream type" << endl;
        exit(1);
        break;
    }
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
