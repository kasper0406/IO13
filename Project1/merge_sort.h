#pragma once

#include <algorithm>
#include <vector>
#include <queue>

#include "input_stream.h"
#include "output_stream.h"

using namespace std;

namespace IO13 {

typedef uint32_t StreamID;

template <typename T, template <typename> class OUT, template <typename> class IN>
void merge(OUT<T> out, vector<IN<T>>& ins) {
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

    out.write(element);
    if (!ins[id].end_of_stream()) {
      q.push(make_pair(ins[id].read_next(), id));
    }
  }
}

}  // namespace IO13