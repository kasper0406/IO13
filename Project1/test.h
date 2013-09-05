#pragma once

#include <sstream>
#include <iomanip>
#include <cassert>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <chrono>

#include "utils.h"
#include "input_stream.h"
#include "output_stream.h"

//#ifdef __linux__
//#include </home/espeholt/ipcm/cpucounters.h>
//#endif

using namespace std;
using namespace std::chrono;

struct Measurement {
  Measurement()
  : time(0),
    l2_cache_hits(0),
    l2_cache_misses(0),
    l3_cache_hits(0),
    l3_cache_misses(0),
    instructions_retired(0),
    cpu_energy_used(0),
    dram_energy_used(0)
  { }

  bool operator<(const Measurement& o) const {
    return time < o.time;
  }

  double time;

  uint64_t l2_cache_hits;
  uint64_t l2_cache_misses;
  uint64_t l3_cache_hits;
  uint64_t l3_cache_misses;
  uint64_t instructions_retired;
  double cpu_energy_used;
  double dram_energy_used;
};

template <typename Func>
void measure(ostream& out,
             const string& description,
             const size_t trials,
             Func f)
{
  const size_t iMin = 0;
  const size_t iLower = trials / 4;
  const size_t iMedian = trials / 2;
  const size_t iUpper = iLower + iMedian;
  const size_t iMax = trials - 1;
  
  vector<Measurement> measurements;
  
  out << description << "\t" << trials << "\t";
  
  for (unsigned int i = 0; i < trials; i++) {
    Measurement measurement;

    auto beginning = high_resolution_clock::now();
    
//#ifdef __linux__
//    SystemCounterState before_sstate = getSystemCounterState();
//#endif

    // Run code to be benchmarked
    f();      

//#ifdef __linux__
//    SystemCounterState after_sstate = getSystemCounterState();
//    measurement.l2_cache_hits        = getL2CacheHits(before_sstate, after_sstate);
//    measurement.l2_cache_misses      = getL2CacheMisses(before_sstate, after_sstate);
//    measurement.l3_cache_hits        = getL3CacheHits(before_sstate, after_sstate);
//    measurement.l3_cache_misses      = getL3CacheMisses(before_sstate, after_sstate);
//    measurement.instructions_retired = getInstructionsRetired(before_sstate, after_sstate);
//    measurement.cpu_energy_used      = getConsumedJoules(before_sstate, after_sstate);
//    measurement.dram_energy_used     = getDRAMConsumedJoules(before_sstate, after_sstate);
//#endif

    high_resolution_clock::duration duration = high_resolution_clock::now() - beginning;
    double time_spent = duration_cast<milliseconds>(duration).count() / 1000.;

    measurement.time = time_spent;
    measurements.push_back(measurement);
  }
  
  // Could be optimized with selection instead of sorting
  sort(measurements.begin(), measurements.end());
  
  out << fixed << measurements[iMin].time << "\t";
  out << fixed << measurements[iLower].time << "\t";
  out << fixed << measurements[iMedian].time << "\t";
  out << fixed << measurements[iUpper].time << "\t";
  out << fixed << measurements[iMax].time << "\t";

//#ifdef __linux__
//  out << fixed << measurements[iMedian].l2_cache_hits << "\t";
//  out << fixed << measurements[iMedian].l2_cache_misses << "\t";
//  out << fixed << measurements[iMedian].l3_cache_hits << "\t";
//  out << fixed << measurements[iMedian].l3_cache_misses << "\t";
//  out << fixed << measurements[iMedian].instructions_retired << "\t";
//  out << fixed << measurements[iMedian].cpu_energy_used << "\t";
//  out << fixed << measurements[iMedian].dram_energy_used << "\t";
//#endif
  
  out << endl;
};

template <class IS>
void print_test_reads_header(ostream& out, uint64_t elements) {
  string name = typeid(IS).name();

  out << "--- Stream \"" << name << "\" read test ---" << endl;
  out << "Elements: " << elements << endl;
  out << endl << "n\t\tk\tTrials\tMin    [s]\tLower  [s]\tMedian [s]\tUpper  [s]\tMax [s]";
//#ifdef __linux__
//  out << "\tL2 hits\tL2 misses\tL3 hits\tL3 misses\tInst. ret.\tCPU Energy [J]\tDRAM Energy [J]";
//#endif

  out << endl;
}

// Tester reads fra flere streams der interleaves. Det er det der giver den bedste
// merge-sort approksimation. Hvis de koeres efter hinanden er det jo bare ligesom
// at koere een sekventielt.
template <template<typename> class IS>
void test_reads(uint64_t elements) {
  const string filename = "test_file";
  const uint32_t max_k = 32;

  if (!(elements && !(elements & (elements - 1)))) {
    cout << "Number of elements not a power of 2" << endl;
    exit(1);
  }

  if (max_k > elements) {
    cout << "Max number of streams must be lower or equal to the number of elements" << endl;
    exit(1);
  }

  generate_file<uint32_t>(filename, random_uint32, elements);

  print_test_reads_header<IS<uint32_t>>(cout, elements);

  for (uint32_t k = 1; k <= max_k; k *= 2) {
    vector<IS<int32_t>> streams(k);

    uint64_t n = elements / k;

    stringstream test;
    test << setw(16) << to_string(n) + "\t" + to_string(k);
    measure(cout, test.str(), 3, [&]() {
      for (uint32_t i = 0; i < k; ++i) {
        streams[i].open(filename, (uint64_t)i * (elements / k), ((uint64_t)i + 1LL) * (elements / k));
      }

      for (uint64_t i = 0; i < n; ++i) {
        for (auto& stream : streams) {
          stream.read_next();
        }
      }

      for (auto& stream : streams) {
        if (!stream.end_of_stream()) {
          cout << "Not eof" << endl;
          exit(1);
        }
        stream.close();
      }
    });
  }
}