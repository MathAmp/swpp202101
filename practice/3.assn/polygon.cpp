#include <cstdint>
#include <vector>
#include <iostream>
using namespace std;

uint64_t area(vector<pair<int, int>> &points) {
  // This code has signed overflows. :)
  // Please fix this so it correctly evaluates area..!
  uint64_t total = 0;
  int left = 0;
  size_t n = points.size();

  for (unsigned i = 0; i < n; i++) {
    unsigned j = (i + 1) % n;
    int x_i = points[i].first;
    int y_i = points[i].second;
    int x_j = points[j].first;
    int y_j = points[j].second;
    int64_t partial = int64_t(x_i) * int64_t(y_j) - int64_t(y_i) * int64_t(x_j);
    total += partial / 2;
    left += partial % 2;
  }

  return total + left / 2;
}
