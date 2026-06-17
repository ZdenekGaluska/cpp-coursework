/*
 * # Longest Increasing Subsequence
 * 
 * Your task is to implement (template of) the function
 * `longest_increasing_subsequence`:
 * 
 * - Its argument is a sequence of objects in which you should
 *   find the longest increasing subsequence.
 * - The subsequence must be strictly increasing. You may use
 *   `<`, `std::less` or `<=>` to compare elements of the sequence.
 * - The return value is a list of indices of the elements of the
 *   subsequence. These indices must be in ascending order.
 * 
 * The bonus test requires solution with almost linear time
 * complexity.
 * 
 * The time limits are 9, 5, 5 and 3 seconds (the bonus test awards
 * more points for solutions faster than 2 and 1 second).
 * 
 */

#ifndef __PROGTEST__
#include <cassert>
#include <cstdarg>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <array>
#include <algorithm>
#include <functional>
#include <deque>
#include <queue>
#include <random>
#include <ranges>
#include <type_traits>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <compare>
#include <ranges>


struct TestFailed : std::runtime_error {
  using std::runtime_error::runtime_error;
};

std::string fmt(const char *f, ...) {
  va_list args1;
  va_list args2;
  va_start(args1, f);
  va_copy(args2, args1);
  
  std::string buf(vsnprintf(nullptr, 0, f, args1), '\0');
  va_end(args1);
  
  vsnprintf(buf.data(), buf.size() + 1, f, args2);
  va_end(args2);

  return buf;
}

#define CHECK(succ, ...) do { \
    if (!(succ)) throw TestFailed(fmt(__VA_ARGS__)); \
  } while (0)

#endif


// Return a vector of indices of the longest increasing subsequence
template <typename T>
std::vector<size_t> longest_increasing_subsequence(const std::vector<T>& data) {
    if (data.empty()) return {};

    int n = data.size();
    std::vector<int> lis(n, 1);  
    std::vector<int> next(n, -1);   
    int max_len = 0;
    int start_index = 0;

    for (int i = n - 1; i >= 0; i--) {
        for (int j = i + 1; j < n; j++) {
            if (data[j] > data[i] && lis[i] < lis[j] + 1) {
                lis[i] = lis[j] + 1;
                next[i] = j;
            }
        }
        if (lis[i] > max_len) {
            max_len = lis[i];
            start_index = i;
        }
    }

    std::vector<size_t> result;
    int i = start_index;
    while (i != -1) {
        result.push_back(i);
        i = next[i];
    }

    return result;
}



#ifndef __PROGTEST__

const std::pair<size_t, std::vector<int>> TESTS[] = {
  { 3, { 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 2 } },
  // 20
  { 6, { 15, 7, 4, 3, 16, 14, 20, 10, 19, 11, 12, 6, 17, 8, 9, 5, 2, 13, 1, 18 } },
  { 7, { 19, 15, 13, 6, 2, 20, 5, 7, 3, 18, 16, 17, 9, 8, 10, 12, 11, 1, 14, 4 } },
  { 6, { 17, 20, 5, 14, 7, 19, 1, 12, 15, 9, 4, 2, 6, 3, 13, 16, 11, 18, 8, 10 } },
  { 5, { 10, 20, 13, 6, 17, 7, 18, 3, 5, 14, 8, 12, 19, 2, 16, 9, 4, 15, 11, 1 } },
  { 8, { 1, 4, 18, 2, 6, 7, 10, 3, 17, 15, 16, 14, 9, 20, 12, 8, 5, 11, 13, 19 } },
  { 8, { 1, 11, 19, 6, 14, 4, 9, 17, 5, 13, 8, 16, 10, 12, 2, 18, 7, 15, 20, 3 } },
  { 8, { 14, 3, 9, 16, 20, 12, 8, 1, 10, 13, 15, 4, 17, 11, 18, 6, 5, 19, 2, 7 } },
  { 6, { 3, 16, 18, 13, 10, 20, 11, 8, 9, 12, 4, 7, 2, 19, 14, 17, 15, 5, 6, 1 } },
  { 6, { 15, 3, 10, 19, 2, 1, 17, 9, 7, 4, 12, 13, 20, 14, 18, 16, 5, 11, 8, 6 } },
  { 7, { 9, 20, 19, 11, 3, 2, 5, 12, 7, 14, 18, 10, 8, 13, 15, 4, 17, 1, 16, 6 } },
  { 5, { 16, 2, 19, 18, 14, 12, 11, 20, 7, 9, 13, 3, 8, 17, 5, 15, 6, 4, 10, 1 } },
  { 6, { 11, 10, 15, 5, 2, 17, 9, 6, 12, 18, 16, 4, 7, 8, 3, 14, 20, 1, 19, 13 } },
  { 7, { 1, 7, 8, 20, 13, 19, 15, 6, 14, 16, 5, 12, 4, 3, 18, 9, 17, 2, 10, 11 } },
  { 8, { 6, 4, 12, 10, 11, 14, 7, 19, 2, 18, 13, 3, 15, 8, 16, 17, 1, 20, 9, 5 } },
  { 7, { 15, 8, 1, 14, 5, 2, 11, 4, 9, 16, 20, 12, 3, 13, 19, 6, 18, 7, 10, 17 } },
  { 6, { 20, 8, 6, 18, 3, 9, 13, 16, 17, 11, 2, 15, 12, 5, 19, 10, 7, 4, 14, 1 } },
  { 5, { 12, 14, 11, 8, 19, 18, 16, 6, 10, 17, 7, 9, 3, 4, 2, 20, 15, 13, 5, 1 } },
  { 7, { 16, 4, 11, 1, 12, 19, 5, 18, 9, 8, 14, 15, 3, 17, 10, 7, 13, 2, 20, 6 } },
  { 6, { 11, 16, 13, 5, 1, 19, 12, 6, 4, 15, 14, 20, 7, 10, 3, 2, 17, 8, 9, 18 } },
  { 5, { 10, 19, 14, 17, 12, 18, 20, 16, 1, 9, 13, 2, 11, 8, 15, 7, 4, 3, 6, 5 } },
  // 50
  { 10, { 38, 26, 10, 7, 41, 8, 29, 17, 43, 33, 39, 32, 6, 50, 1, 42, 14, 40, 25,
          11, 18, 23, 15, 9, 20, 34, 21, 35, 12, 27, 22, 45, 44, 5, 37, 13, 3, 31,
          47, 46, 30, 19, 4, 49, 36, 28, 24, 2, 48, 16 } },
  { 10, { 43, 35, 28, 15, 20, 6, 26, 40, 42, 11, 7, 41, 18, 27, 13, 49, 17, 46,
          24, 21, 47, 8, 1, 2, 33, 45, 9, 48, 29, 23, 36, 39, 44, 30, 12, 5, 31,
          38, 16, 4, 32, 25, 19, 22, 10, 3, 50, 37, 14, 34 } },
  { 10, { 5, 20, 23, 34, 14, 2, 31, 44, 36, 26, 25, 22, 10, 16, 33, 46, 48, 43,
          47, 40, 7, 29, 37, 15, 28, 17, 8, 9, 49, 50, 21, 30, 41, 6, 4, 32, 24,
          27, 42, 45, 35, 19, 38, 12, 11, 13, 1, 18, 39, 3 } },
  // 70
  { 15, { 35, 1, 41, 38, 24, 2, 10, 37, 32, 16, 30, 51, 48, 63, 17, 69, 47, 60,
          54, 21, 50, 33, 56, 13, 6, 29, 5, 65, 49, 44, 26, 7, 25, 31, 64, 12, 9,
          58, 28, 11, 40, 43, 18, 15, 68, 57, 45, 59, 67, 70, 34, 3, 19, 61, 52,
          23, 4, 46, 27, 36, 14, 53, 39, 66, 20, 42, 8, 55, 22, 62 } },
  { 12, { 23, 45, 51, 68, 25, 17, 12, 29, 55, 41, 39, 62, 67, 61, 52, 58, 34, 4,
          10, 6, 5, 16, 50, 42, 33, 60, 40, 36, 13, 21, 3, 2, 63, 35, 19, 64, 20,
          24, 32, 31, 22, 1, 26, 48, 9, 69, 38, 66, 28, 43, 59, 54, 7, 8, 14, 53,
          70, 37, 47, 65, 56, 11, 18, 46, 30, 27, 44, 49, 57, 15 } },
  { 12, { 15, 25, 35, 65, 6, 21, 33, 16, 3, 55, 32, 60, 64, 46, 12, 19, 5, 43, 11,
          59, 67, 42, 58, 62, 24, 47, 44, 34, 37, 26, 23, 51, 14, 28, 57, 48, 8,
          9, 49, 22, 61, 7, 18, 66, 27, 54, 45, 39, 17, 63, 4, 52, 68, 38, 40, 30,
          2, 56, 70, 13, 20, 53, 41, 50, 69, 36, 1, 10, 31, 29 } },
  // 90
  { 16, { 73, 25, 66, 74, 81, 54, 57, 58, 84, 39, 5, 89, 76, 29, 60, 53, 7, 77,
          67, 78, 2, 70, 6, 50, 52, 11, 48, 17, 36, 27, 51, 28, 18, 88, 22, 1, 14,
          82, 19, 13, 61, 75, 35, 86, 55, 90, 49, 87, 45, 41, 62, 21, 79, 31, 83,
          71, 40, 9, 43, 4, 46, 20, 3, 15, 16, 68, 59, 63, 23, 65, 33, 64, 69, 34,
          32, 85, 10, 30, 56, 47, 12, 26, 8, 24, 80, 37, 38, 42, 72, 44 } },
  { 14, { 82, 31, 71, 27, 42, 73, 59, 28, 78, 11, 72, 76, 38, 24, 70, 53, 90, 54,
          23, 9, 65, 64, 89, 3, 86, 45, 80, 68, 26, 37, 19, 51, 77, 14, 81, 79,
          88, 7, 16, 29, 20, 46, 18, 22, 21, 63, 74, 25, 10, 41, 85, 52, 35, 34,
          60, 69, 40, 57, 62, 58, 39, 1, 17, 87, 13, 75, 56, 4, 5, 43, 61, 15, 67,
          12, 33, 8, 44, 6, 83, 36, 49, 32, 66, 50, 84, 48, 30, 47, 55, 2 } },
  { 16, { 47, 32, 35, 85, 55, 79, 78, 68, 60, 66, 51, 8, 42, 52, 6, 14, 90, 61, 5,
          54, 48, 86, 1, 37, 23, 24, 27, 2, 57, 87, 80, 64, 74, 31, 75, 33, 39,
          71, 63, 43, 58, 44, 56, 15, 46, 16, 28, 11, 73, 12, 18, 53, 82, 7, 20,
          40, 4, 34, 29, 76, 9, 25, 41, 36, 38, 65, 45, 72, 89, 88, 84, 30, 62,
          81, 26, 83, 50, 19, 21, 22, 59, 13, 70, 17, 10, 67, 49, 69, 77, 3 } },
  // 110
  { 17, { 11, 71, 39, 99, 105, 74, 92, 7, 110, 26, 15, 90, 56, 76, 36, 100, 85,
          108, 82, 9, 31, 51, 73, 21, 83, 103, 48, 29, 27, 104, 75, 23, 46, 59, 54,
          18, 79, 22, 106, 4, 40, 64, 57, 8, 52, 70, 35, 37, 88, 101, 58, 28, 87,
          89, 61, 86, 20, 62, 16, 43, 24, 68, 77, 13, 10, 47, 96, 107, 60, 65, 49,
          2, 34, 102, 42, 67, 94, 32, 69, 17, 98, 12, 81, 55, 63, 53, 6, 38, 5,
          109, 14, 84, 45, 97, 33, 41, 93, 91, 44, 30, 72, 1, 78, 19, 80, 50, 66,
          95, 25, 3 } },
  { 17, { 72, 110, 78, 47, 65, 46, 32, 6, 24, 96, 31, 105, 62, 76, 106, 102, 85, 86,
          61, 84, 98, 25, 20, 103, 7, 19, 67, 29, 36, 33, 58, 64, 69, 1, 81, 60, 87,
          92, 22, 95, 35, 56, 40, 10, 88, 50, 77, 15, 8, 74, 73, 30, 63, 12, 80, 79,
          48, 104, 66, 11, 34, 49, 17, 42, 89, 41, 68, 55, 59, 39, 52, 94, 14, 13,
          28, 27, 75, 37, 91, 45, 2, 43, 21, 83, 101, 97, 16, 38, 109, 44, 53, 54,
          99, 9, 3, 4, 107, 23, 18, 93, 70, 26, 90, 71, 82, 100, 57, 5, 108, 51 } },
  { 18, { 31, 15, 58, 62, 18, 88, 60, 51, 16, 64, 4, 80, 79, 57, 41, 102, 29, 32,
          103, 38, 14, 3, 96, 75, 106, 94, 72, 85, 73, 69, 5, 108, 95, 67, 107, 35,
          40, 100, 98, 54, 23, 25, 74, 87, 1, 8, 2, 21, 47, 39, 48, 70, 24, 52, 101,
          59, 34, 83, 55, 105, 65, 61, 77, 86, 81, 82, 45, 11, 26, 12, 44, 37, 93,
          49, 66, 19, 92, 13, 91, 9, 109, 33, 7, 50, 43, 27, 71, 36, 22, 78, 42, 90,
          28, 84, 68, 97, 56, 46, 110, 89, 6, 17, 30, 53, 99, 63, 104, 10, 76, 20 } },
};

struct BigTest {
  size_t ref_sol;
  // sequence of length mod - 1 of
  // mul % mod, (2*mul) % mod, (3*mul) % mod, ...
  int mod, mul;

  std::vector<int> data() const {
    std::vector<int> ret;
    int x = 0;
    for (int i = 1; i < mod; i++)
      ret.push_back(x = (x + mul) % mod);
    return ret;
  }
};

constexpr BigTest BIG_TESTS[] = {
  {      29,     300,    11 },
  {      19,     800,    61 },
  {     127,   2'000,    17 },
  {     715,   5'000,     7 },
  {     153,  10'000, 3'867 },
  {       1,  10'000, 9'999 },
  {   3'999,   4'000,     1 },
  {   9'999,  10'000,     1 },
  {   9'999,  10'000,     1 },
  {  99'999, 100'000,     1 },
  { 998'999, 999'000,     1 },
};


void check_sol(size_t ref, const std::vector<size_t>& sol, const std::vector<int>& data) {
  if (sol.size() >= 1) CHECK(sol[0] < data.size(), "Index %zu out of bounds", 0);

  for (size_t i = 1; i < sol.size(); i++) {
    CHECK(sol[i] < data.size(), "Index %zu out of bounds", i);
    CHECK(sol[i-1] < sol[i], "Solution goes backwards at index %zu\n", i);
    CHECK(data[sol[i-1]] < data[sol[i]], "Solution decreases at index %zu\n", i);
  }

  CHECK(sol.size() == ref,
    "Expected solution of length %zu but got %zu\n", ref, sol.size());
}

int main() {
  for (auto&& [ r, data ] : TESTS) {
    std::vector<size_t> sol = longest_increasing_subsequence(data);
    try {
      check_sol(r, sol, data);
    } catch (const TestFailed& e) {
      std::cout << "Test failed: " << e.what() << std::endl;
      return 1;
    }
  }
  
  std::cout << "Big tests:" << std::endl;

  for (auto&& t : BIG_TESTS) {
    auto data = t.data();
    std::cout << "Size: " << t.mod - 1 << std::endl;
    std::vector<size_t> sol = longest_increasing_subsequence(data);
    try {
      check_sol(t.ref_sol, sol, data);
    } catch (const TestFailed& e) {
      std::cout << "Test failed: " << e.what() << std::endl;
      return 1;
    }
  }

  std::cout << "All tests passed." << std::endl;
}

#endif

