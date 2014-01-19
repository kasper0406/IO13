#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE

#include <iostream>
#include <ctime>
#include <fstream>
#include <utility>
#include <chrono>
#include "sys/wait.h"
#include <iomanip>
#include <unistd.h>
#include <tuple>
#include <inttypes.h>
#include <cstdlib>

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
void client(size_t elements, size_t block_size, size_t buffer_size, size_t d, size_t cache_size) {
  ExternalHeap<S, int> heap("heap", block_size, buffer_size, d, cache_size);

  // TODO
  FStream<int> stream(0);
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
  cout << setw(12) << "Cache size";
  cout << setw(12) << "Time";
  cout << setw(12) << "I sectors";
  cout << setw(12) << "O sectors";
  cout << setw(12) << "Read ops";
  cout << setw(12) << "Write ops";

  cout << endl;

  // SET PARAMETERS HERE!
  #ifdef LINUX
  string timeout_exec = "/usr/bin/timeout";
  #else
  string timeout_exec = "/usr/local/Cellar/coreutils/8.21/bin/gtimeout";
  #endif
  int timeout_seconds = 1000;
  int block_size_start = 1024;
  int block_size_end = 4096 * 4096 * 8;
  int buffer_size_start = 1024;
  int buffer_size_end = 1024 * 16;
  int d_start = 2;
  int d_end = 4096;
  int64_t elements_start = 1024 * 1024;
  int64_t elements_end = 1024 * 1024 * 256;
  int64_t cache_size_start = 128;
  int64_t cache_size_end = 1024 * 16;
  double max_height = 4.;
  vector<char> stream_types { 'm', 'b', 'f' };
  
  for (double allowed_max_height = 2.; allowed_max_height < max_height; ++allowed_max_height) {
    for (int64_t elements = elements_start; elements <= elements_end; elements*=2) {
      for (char stream_type : stream_types) {
        for (int block_size = block_size_start; block_size <= block_size_end; block_size*=4) {
          for (int d = d_start; d <= d_end; d*=4) {
            for (int buffer_size = (stream_type == 'b' ? buffer_size_start : 0);
                 buffer_size <= (stream_type == 'b' ? buffer_size_end : 0); buffer_size= max(1, buffer_size * 4)) {
              for (int cache_size = cache_size_start; cache_size <= cache_size_end; cache_size*=4) {
                // Rules
                // Buffer size smaller than block size
                if (buffer_size > block_size) continue;
                // Element size larger than block size
                if (elements < block_size) continue;
                // Not enough elements to satisfy 'd'
                if ((int64_t)d * (int64_t)block_size > elements) continue;
                // Memory size in elements
                const int64_t M = 512 * 1024 * 1024 / 4;
                const int64_t V = block_size;
                const int64_t P = cache_size;
                const int64_t B = buffer_size == 0 ? 1024 : buffer_size;
                const int64_t N = elements;
                // (N + V - 1) / V == ceil(N / V)
                const int64_t estimated_memory_usage = V + 2*P*(N + V - 1)/V+(d+1)*B + B;

                // Make use of most of the memory space.
                if (M / 64 > estimated_memory_usage) continue;
                // Don't suffocate
                if (M < estimated_memory_usage) continue;

                const double height = log(N / V) / log(d);

                if (height > allowed_max_height) continue;
                if (height <= allowed_max_height - 1) continue;

                string command = timeout_exec + " " + to_string(timeout_seconds) + " ./Project2Test client "
                + "\"elements:" + to_string(elements) + " block_size:" + to_string(block_size) + " buffer_size:"
                + to_string(buffer_size) + " d:" + to_string(d) + " stream:" + stream_type + " cache_size:" + to_string(cache_size) + "\"";
                
                cout << setw(12) << stream_type;
                cout << setw(12) << elements;
                cout << setw(12) << block_size;
                cout << setw(12) << d;
                cout << setw(12) << buffer_size;
                cout << setw(12) << cache_size << flush;
                
                auto result = exec(command);
                //auto result = make_pair(0, string("0 0 0 0 0 0 0 0"));
                
                double seconds;
                int64_t disk_i;
                int64_t disk_o;
                int64_t disk_reads;
                int64_t disk_writes;
                int64_t disk_time;
                if (result.first == 0 &&
                    sscanf(result.second.c_str(), "%lf %" SCNd64 " %" SCNd64 " %" SCNd64 " %" SCNd64 " %" SCNd64,
                             &seconds, &disk_i, &disk_o, &disk_reads, &disk_writes, &disk_time) == 6) {
                  cout << setw(12) << seconds;
                  cout << setw(12) << disk_i;
                  cout << setw(12) << disk_o;
                  cout << setw(12) << disk_reads;
                  cout << setw(12) << disk_writes;
                  cout << endl;
                } else if (result.first == 124) {
                  cout << setw(12) << "Timeout" << endl;
                  // break;
                } else {
                  cout << setw(12) << "Error" << endl;
                  // break;
                }
              }
            }
          }
        }
      }
    }
  }
}

void flush_disk() {
  #ifdef LINUX
  sync();
  auto result = exec("echo 3 | sudo tee /proc/sys/vm/drop_caches");
  if (result.first != 0) {
    cout << "Error flushing disk" << endl;
  }
  #endif
}

tuple<int64_t, int64_t, int64_t, int64_t, int64_t> disk_activity() {
#ifdef LINUX
  auto diskstats = exec("cat /proc/diskstats | grep sdb2");
  if (diskstats.first != 0) {
    cout << "Something went wrong fetching disk activity" << endl;
    return make_tuple(0,0,0,0,0);
  }
  char junk[128];
  int64_t field1,field2,field3,field4,field5,field6,field7,field8,field9,field10,field11;
  int success = sscanf(diskstats.second.c_str(), "%[^s]sdb2 %" SCNd64 " %" SCNd64 " %"
                                                 SCNd64 " %" SCNd64 " %" SCNd64 " %" SCNd64 " %" SCNd64
                                                 " %" SCNd64 " %" SCNd64 " %" SCNd64 " %" SCNd64,
                       (char*)&junk, &field1, &field2, &field3, &field4, &field5, &field6, &field7, &field8, &field9, &field10,&field11);
  if (success != 12) {
    cout << "Unable to parse disk activity" << endl;
    return make_tuple(0,0,0,0,0);
  }
  return make_tuple(field3, field7, field1, field5, field11);
#else
  return make_tuple(0,0,0,0,0);
#endif

}

int main(int argc, char *argv[]) {
  srand(time(NULL));

  if (argc > 2 && string(argv[1]) == "client") {
    int64_t elements;
    int block_size;
    int buffer_size;
    int d;
    int cache_size;
    char stream_type;
    if (sscanf(argv[2], "elements:%" SCNd64 " block_size:%i buffer_size:%i d:%i stream:%c cache_size:%i",
               &elements, &block_size, &buffer_size, &d, &stream_type, &cache_size) != 6) {
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

    flush_disk();

    auto disk_start = disk_activity();
    
    auto beginning = high_resolution_clock::now();
    if (cache_size != 0) {
      switch (stream_type) {
        case 'f':
          client<CachedStream<int, FStream>>(elements, block_size, buffer_size, d, cache_size);
          break;

        case 's':
          client<CachedStream<int, SysStream>>(elements, block_size, buffer_size, d, cache_size);
          break;

        case 'b':
          client<CachedStream<int, BufferedStream>>(elements, block_size, buffer_size, d, cache_size);
          break;

        case 'm':
          client<CachedStream<int, MMapFileStream>>(elements, block_size, buffer_size, d, cache_size);
          break;

        default:
          cout << "Unknown stream type" << endl;
          exit(1);
          break;
      }
    } else {
      switch (stream_type) {
        case 'f':
          client<FStream<int>>(elements, block_size, buffer_size, d, cache_size);
          break;

        case 's':
          client<SysStream<int>>(elements, block_size, buffer_size, d, cache_size);
          break;

        case 'b':
          client<BufferedStream<int>>(elements, block_size, buffer_size, d, cache_size);
          break;

        case 'm':
          client<MMapFileStream<int>>(elements, block_size, buffer_size, d, cache_size);
          break;

        case 'n':
          client<MMapStream<int>>(elements, block_size, buffer_size, d, cache_size);
          break;
          
        default:
          cout << "Unknown stream type" << endl;
          exit(1);
          break;
      }
    }
    high_resolution_clock::duration duration = high_resolution_clock::now() - beginning;
    auto disk_end = disk_activity();
    cout << duration_cast<milliseconds>(duration).count() / 1000.
        << " " << (get<0>(disk_end) - get<0>(disk_start))
        << " " << (get<1>(disk_end) - get<1>(disk_start))
        << " " << (get<2>(disk_end) - get<2>(disk_start))
        << " " << (get<3>(disk_end) - get<3>(disk_start))
        << " " << (get<4>(disk_end) - get<4>(disk_start))
        << endl;
  } else {
  #ifdef NDEBUG
    cout << "Release mode" << endl;
  #else
    cout << "Debug mode" << endl;
  #endif
    cout << "Size of off_t: " << sizeof(off_t);
    
    server();
  }
  
  return 0;
}
