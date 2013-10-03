#pragma once

#include <algorithm>
#include <vector>
#include <queue>
#include <string>

#include "input_stream.h"
#include "output_stream.h"

using namespace std;

namespace IO13 {

typedef uint32_t StreamID;

template <typename T, template <typename> class OUT, template <typename> class IN>
IN<T> merge(vector<IN<T>>& ins) {
  int elements = 0;
  for (auto& input : ins)
    elements += input.size();
  
  static uint64_t counter = 0;
  OUT<T> output;
  output.open("tmp" + to_string(counter), 0, elements);
  
  typedef pair<T, StreamID> TPair;
  priority_queue<TPair,vector<TPair>, greater<TPair>> q;

  for (StreamID i = 0; i < ins.size(); ++i) {
    q.push(make_pair(ins[i].read_next(),i));
  }

  while (!q.empty()) {
    auto p = q.top();
    q.pop();
    T element = p.first;
    StreamID id = p.second;

    output.write(element);
    if (!ins[id].end_of_stream()) {
      q.push(make_pair(ins[id].read_next(), id));
    }
  }
  
  output.close();
  
  // Create input stream for output
  IN<T> result;
  result.open("tmp" + to_string(counter), 0, elements);
  
  counter++;
  return move(result);
}

template<typename T>
int compare(const void* t1, const void* t2) {
  return *(T*)t1 - *(T*)t2;
}
 
template <template <typename> class IN, template <typename> class OUT, typename T>
void sort(uint64_t N, string file, uint64_t M, uint32_t d) {
  const string temp = "tmp";
  const uint32_t blocks = ceil(double(N) / double(M));
  
  queue<IN<T>> queue;
  
  // Step 1
  OUT<T> outputstream;
  outputstream.open(temp, 0, N);
  for (int k = 0; k < blocks; k++) {
    const uint64_t start = k * M;
    const uint64_t end = min(N, (k + 1) * M);
    
    IN<T> inputstream;
    inputstream.open(file, start, end);
    vector<T> elements(end - start);
    
    for (int i = 0; !inputstream.end_of_stream(); i++)
      elements[i] = inputstream.read_next();
    
    inputstream.close();
    
    qsort(&elements[0], end - start, sizeof(T), compare<T>);
    // sort(elements.begin(), elements.end());
    
    for (int i = 0; i < end - start; i++)
      outputstream.write(elements[i]);
    
    IN<T> input;
    input.open(temp, start, end);
    queue.push(move(input));
  }
  outputstream.close();
  
  // Step 2
  while (queue.size() != 1) {
    vector<IN<T>> streamsToMerge;
    for (int i = 0; i < d && queue.size() > 0; i++) {
      streamsToMerge.push_back(move(queue.front()));
      queue.pop();
    }
    
    queue.push(move(merge<T, OUT, IN>(streamsToMerge)));
    
    for (auto& stream : streamsToMerge) {
      stream.closeAndRemove();
    }
  }
  
  IN<T> input = move(queue.front());
  OUT<T> output;
  output.open(file, 0, N);
  
  while (!input.end_of_stream())
    output.write(input.read_next());
  
  output.close();
  input.close();
}

}  // namespace IO13
