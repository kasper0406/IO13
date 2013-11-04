#include <iostream>
#include <ctime>
#include <fstream>
#include <utility>
#include <chrono>
#include "sys/wait.h"
#include <iomanip>
#include <inttypes.h>
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

#ifdef LINUX
#define SCNd64 "li"
#endif

template <typename S>
void client(size_t elements, size_t block_size, size_t buffer_size, size_t d) {
  ExternalHeap<S, int> heap("heap", block_size, buffer_size, d);

  // TODO
  FStream<int> stream;
  stream.open("testfile", 0, elements, buffer_size);

  for (uint64_t i = 0; i < elements; ++i) {
    heap.insert(stream.read_next());
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
  cout << setw(12) << "I";
  cout << setw(12) << "O";
  cout << endl;

  // SET PARAMETERS HERE!
  #ifdef LINUX
  string timeout_exec = "/usr/bin/timeout";
  #else
  string timeout_exec = "/usr/local/Cellar/coreutils/8.21/bin/gtimeout";
  #endif
  int timeout_seconds = 1000;
  int block_size_start = 1024;
  int block_size_end = 1024 * 1024 * 128;
  int buffer_size_start = 1024;
  int buffer_size_end = 1024 * 1024 * 8;
  int d_start = 32;
  int d_end = 4096;
  int64_t elements_start = 1024;
  int64_t elements_end = 1024 * 1024 * 1024;
  vector<char> stream_types { 'm' };
  
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

            double seconds;
            int64_t disk_i;
            int64_t disk_o;
            if (result.first == 0 &&
                sscanf(result.second.c_str(), "%lf %" SCNd64 " %" SCNd64,
                         &seconds, &disk_i, &disk_o) == 3) {
              cout << setw(12) << seconds;
              cout << setw(12) << disk_i;
              cout << setw(12) << disk_o;
              cout << endl;
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

pair<int64_t, int64_t> disk_activity() {
#ifdef LINUX
  auto diskstats = exec("cat /proc/diskstats | grep sdb2");
  if (diskstats.first != 0) {
    cout << "Something went wrong fetching disk activity" << endl;
    return {0,0};
  }
  char junk[128];
  int64_t field1,field2,field3,field4,field5,field6,field7;
  int success = sscanf(diskstats.second.c_str(), "%[^s]sdb2 %" SCNd64 " %" SCNd64 " %"
                                                 SCNd64 " %" SCNd64 " %" SCNd64 " %" SCNd64 " %" SCNd64,
                       (char*)&junk, &field1, &field2, &field3, &field4, &field5, &field6, &field7);
  if (success != 8) {
    cout << "Unable to parse disk activity" << endl;
    return {0,0};
  }
  return {field3, field7};
#else
  return {0,0};
#endif

}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  if (argc > 2 && string(argv[1]) == "client") {
    int64_t elements;
    int block_size;
    int buffer_size;
    int d;
    char stream_type;
    if (sscanf(argv[2], "elements:%" SCNd64 " block_size:%i buffer_size:%i d:%i stream:%c",
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
      int value = rand();
      create.write(reinterpret_cast<const char*>(&value), sizeof(int));
    }

    create.close();

    auto disk_start = disk_activity();
    
    auto beginning = high_resolution_clock::now();
    switch (stream_type) {
      case 'f':
        client<CachedStream<int, FStream, 1024>>(elements, block_size, buffer_size, d);
        break;

      case 's':
        client<CachedStream<int, SysStream, 128>>(elements, block_size, buffer_size, d);
        break;

      case 'b':
        client<BufferedStream<int>>(elements, block_size, buffer_size, d);
        break;

      case 'm':
        client<CachedStream<int, MMapFileStream, 1024>>(elements, block_size, buffer_size, d);
        break;

      default:
        cout << "Unknown stream type" << endl;
        exit(1);
        break;
    }
    high_resolution_clock::duration duration = high_resolution_clock::now() - beginning;
    auto disk_end = disk_activity();
    cout << duration_cast<milliseconds>(duration).count() / 1000.
        << " " << (disk_end.first - disk_start.first)
        << " " << (disk_end.second - disk_end.second)
        << endl;
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
