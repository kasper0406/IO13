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

template <class S>
void print_test_header(ostream& out, uint64_t elements) {
  string name = typeid(S).name();

  out << endl << "--- " << name << " read/write test ---" << endl;
  out << "Elements: " << elements << endl;
  out << endl << "n\t\tk\tTrials\tMin    [s]\tLower  [s]\tMedian [s]\tUpper  [s]\tMax [s]";
//#ifdef __linux__
//  out << "\tL2 hits\tL2 misses\tL3 hits\tL3 misses\tInst. ret.\tCPU Energy [J]\tDRAM Energy [J]";
//#endif

  out << endl;
}

template <template<typename> class IN, template<typename> class OUT>
void sanity_test() {
  const string filename = "test_file";
  const uint64_t elements = 1024;
  
  // Sequential write test
  OUT<uint32_t> out;
  out.open(filename, 0, elements);

  for (uint64_t i = 0; i < elements; ++i) {
    out.write(i);
  }

  out.close();

  // Sequential read test
  IN<uint32_t> in;
  in.open(filename, 0, elements);

  for (uint64_t i = 0; i < elements; ++i) {
    if (in.read_next() != i) {
      cout << "Sanity check for "
           << typeid(IN<uint32_t>).name() << " and "
           << typeid(OUT<uint32_t>).name() << " failed";
      exit(1);
    }
  }

  in.close();

  generate_file<uint64_t>(filename, random_uint32, elements);

  // Parallel write test
  OUT<uint32_t> out0;
  OUT<uint32_t> out1;
  OUT<uint32_t> out2;
  OUT<uint32_t> out3;
  out0.open(filename, 0, elements / 4);
  out1.open(filename, elements / 4, elements / 2);
  out2.open(filename, elements / 2, (elements / 4) * 3);
  out3.open(filename, (elements / 4) * 3, elements);

  for (uint64_t i = 0; i < elements; ) {
    out0.write(i++);
    out1.write(i++);
    out2.write(i++);
    out3.write(i++);
  }

  out0.close();
  out1.close();
  out2.close();
  out3.close();

  // Parallel read test
  IN<uint32_t> in0;
  IN<uint32_t> in1;
  IN<uint32_t> in2;
  IN<uint32_t> in3;
  in0.open(filename, 0, elements / 4);
  in1.open(filename, elements / 4, elements / 2);
  in2.open(filename, elements / 2, (elements / 4) * 3);
  in3.open(filename, (elements / 4) * 3, elements);

  for (uint64_t i = 0; i < elements; ) {
    if (in0.read_next() != i++
        || in1.read_next() != i++
        || in2.read_next() != i++
        || in3.read_next() != i++) {
      cout << "Sanity check failed for "
           << typeid(IN<uint32_t>).name() << " and "
           << typeid(OUT<uint32_t>).name() << " failed";
      exit(1);
    }
  }

  in0.close();
  in1.close();
  in2.close();
  in3.close();

  cout << "Sanity check for "
       << typeid(IN<uint32_t>).name() << " and " << endl << "                 "
       << typeid(OUT<uint32_t>).name() << " succeeded" << endl;
}

// TODO(lespeholt): Tildels copy-paste for test_reads og test_writes

// Tester flere streams der interleaves. Det er det der giver den bedste
// merge-sort approksimation. Hvis de koeres efter hinanden er det jo bare ligesom
// at koere een sekventielt.
template <class S>
void test_reads(uint64_t elements) {
  const string filename = "test_file";
  const uint32_t max_k = 32;
  const uint32_t trials = 3;
    
  if (!(elements && !(elements & (elements - 1)))) {
    cout << "Number of elements not a power of 2" << endl;
    exit(1);
  }

  if (max_k > elements) {
    cout << "Max number of streams must be lower or equal to the number of elements" << endl;
    exit(1);
  }

  generate_file<typename S::type>(filename, random_uint32, elements);

  print_test_header<S>(cout, elements);

  for (uint32_t k = 1; k <= max_k; k *= 2) {
    vector<S> streams(k);

    uint64_t n = elements / k;

    stringstream test;
    test << setw(16) << to_string(n) + "\t" + to_string(k);

    auto read_test = [&]() {
      for (uint32_t i = 0; i < k; ++i) {
        streams[i].open(filename, (uint64_t)i * (elements / k), ((uint64_t)i + 1LL) * (elements / k));
      }

      for (uint64_t i = 0; i < n; ++i) {
        for (auto& stream : streams) {
          stream.read_next();
        }
      }

      for (auto& stream : streams) {
        // Sanity check
        if (!stream.end_of_stream()) {
          cout << "Not eof" << endl;
          exit(1);
        }
        stream.close();
      }
    };

    measure(cout, test.str(), trials, read_test);
  }
}

// Tester flere streams der interleaves. Det er det der giver den bedste
// merge-sort approksimation. Hvis de koeres efter hinanden er det jo bare ligesom
// at koere een sekventielt.
// For write tests, udskrives der bare 0'er
template <class S>
void test_writes(uint64_t elements) {
  const string filename = "test_file";
  const uint32_t max_k = 32;
  const uint32_t trials = 3;
    
  if (!(elements && !(elements & (elements - 1)))) {
    cout << "Number of elements not a power of 2" << endl;
    exit(1);
  }

  if (max_k > elements) {
    cout << "Max number of streams must be lower or equal to the number of elements" << endl;
    exit(1);
  }

  print_test_header<S>(cout, elements);

  for (uint32_t k = 1; k <= max_k; k *= 2) {
    vector<S> streams(k);

    uint64_t n = elements / k;

    stringstream test;
    test << setw(16) << to_string(n) + "\t" + to_string(k);

    auto write_test = [&]() {
      for (uint32_t i = 0; i < k; ++i) {
        streams[i].open(filename, (uint64_t)i * (elements / k), ((uint64_t)i + 1LL) * (elements / k));
      }

      for (uint64_t i = 0; i < n; ++i) {
        for (auto& stream : streams) {
          stream.write(0);
        }
      }

      for (auto& stream : streams) {
        stream.close();
      }
    };

    measure(cout, test.str(), trials, write_test);
  }
}