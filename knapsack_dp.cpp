/*
 * # Knapsack
 * 
 * Your task is to implement function `knapsack` which
 * finds subset of given items such that sum of values
 * is maximized while sum of weights does not exceed given
 * maximum. The arguments are:
 * 
 * - `const std::vector<Item>& items`: The list of items
 *   from which the subset is selected. The struct `Item`
 *   has two members:
 *   - `unsigned weight` and
 *   - `unsigned value`.
 * 
 * - `unsigned max_weight`: Maximum allowed weight of the
 *   selected subset.
 * 
 * The return value is `std::vector<bool>` of the same
 * length as `items` containing `true` if the corresponding
 * item is selected.
 * 
 * Time limit is 9 seconds for the small test, 6.5 seconds
 * for the big test and 2.6 seconds for bonus points from
 * big tests (slightly slower solutions are awarded partial
 * points).
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
#include <variant>

struct Item {
  unsigned weight;
  unsigned value;
};

#endif

int rec_knap(const std::vector<Item>& items, unsigned max_weight, int i, std::vector<std::vector<int>> &dp, std::vector<std::vector<bool>> &parent) {
    if (i >= items.size()) return 0;
    if (dp[i][max_weight] >= 0) return dp[i][max_weight];

    int not_take = rec_knap(items, max_weight, i+1, dp, parent);
    if (items[i].weight > max_weight) return not_take;
    int take = rec_knap(items, max_weight - items[i].weight, i + 1, dp, parent) + items[i].value;

    if (not_take > take) {
        dp[i][max_weight] = not_take;
        return dp[i][max_weight];
    }
    else {
        dp[i][max_weight] = take;
        parent[i][max_weight] = 1;
        return dp[i][max_weight];   
    }
}

std::vector<bool> back_tracking(const std::vector<Item>& items,
                                std::vector<std::vector<bool>> &parent,
                                unsigned max_weight) {
    std::vector<bool> result(items.size(), false);
    int w = max_weight;
    for (size_t i = 0; i < items.size(); i++) {
        if (w >= 0 && parent[i][w]) { 
            result[i] = true;
            w -= items[i].weight;
        }
    }
    return result;
}

std::vector<bool> knapsack(const std::vector<Item>& items, unsigned max_weight) {
  std::vector<std::vector<int>> dp(items.size(), std::vector<int>(max_weight + 1, -1) );
  std::vector<std::vector<bool>> parent(items.size(), std::vector<bool>(max_weight + 1, 0) );

  rec_knap(items, max_weight, 0, dp, parent);
  return back_tracking(items, parent, max_weight);
}


#ifndef __PROGTEST__


struct Test {
  unsigned best_value;
  unsigned max_weight;
  std::vector<Item> items;
};

Test tests[] = {
  {   0,  5, {} },
  {   0,  0, {{1,1}, {2,2}, {3,3}} },
  {   5,  5, {{1,1}, {2,2}, {3,3}} },
  {   7,  7, {{1,1}, {4,5}, {6,6}} }, 
  {  15, 10, {{2,3}, {3,4}, {4,5}, {5,8}} },
  { 220, 50, {{10,60}, {20,100}, {30,120}} },
};


std::vector<Item> gen_items(
  size_t item_count,
  unsigned avg_weight,
  uint32_t seed
) {
  std::mt19937 R(seed);

  std::vector<Item> ret(item_count, {
    .weight = avg_weight - avg_weight / 10,
    .value = 100*avg_weight - 10*avg_weight,
  });

  unsigned w_diff = 1 + avg_weight / 5;
  unsigned v_diff = 1 + 20 * avg_weight;

  for (auto& [ w, v ] : ret) {
    w += R() % w_diff;
    v += R() % v_diff;
  }

  return ret;
}

struct GenTest {
  unsigned best_value;
  unsigned max_weight;
  size_t item_count;
  unsigned avg_weight;
  uint32_t seed;
};

constexpr GenTest small_tests[] =  {
  { .best_value=11782, .max_weight=126, .item_count=13, .avg_weight=19, .seed=298470443 },
  { .best_value=11741, .max_weight=126, .item_count=13, .avg_weight=19, .seed=2942770775 },
  { .best_value=11767, .max_weight=126, .item_count=13, .avg_weight=19, .seed=2337056925 },
  { .best_value=12256, .max_weight=124, .item_count=13, .avg_weight=19, .seed=1159733202 },
  { .best_value=11582, .max_weight=127, .item_count=13, .avg_weight=19, .seed=3197197766 },
  { .best_value=11859, .max_weight=126, .item_count=13, .avg_weight=19, .seed=1598519539 },
  { .best_value=13560, .max_weight=131, .item_count=13, .avg_weight=20, .seed=2363788283 },
  { .best_value=11363, .max_weight=120, .item_count=13, .avg_weight=18, .seed=3276142926 },
  { .best_value=12060, .max_weight=124, .item_count=13, .avg_weight=19, .seed=661475593 },
  { .best_value=11326, .max_weight=122, .item_count=13, .avg_weight=18, .seed=2706605226 },
  { .best_value=12069, .max_weight=124, .item_count=13, .avg_weight=19, .seed=121124069 },
  { .best_value=12083, .max_weight=128, .item_count=13, .avg_weight=19, .seed=198282099 },
  { .best_value=12307, .max_weight=128, .item_count=13, .avg_weight=19, .seed=4115812295 },
  { .best_value=12037, .max_weight=124, .item_count=13, .avg_weight=19, .seed=124331330 },
  { .best_value=12199, .max_weight=129, .item_count=13, .avg_weight=19, .seed=2376060647 },
  { .best_value=11458, .max_weight=122, .item_count=13, .avg_weight=18, .seed=2268424193 },
  { .best_value=14068, .max_weight=131, .item_count=13, .avg_weight=20, .seed=1083990070 },
  { .best_value=11476, .max_weight=120, .item_count=13, .avg_weight=18, .seed=2431142120 },
  { .best_value=14432, .max_weight=130, .item_count=13, .avg_weight=20, .seed=1352932079 },
  { .best_value=11373, .max_weight=120, .item_count=13, .avg_weight=18, .seed=4082612256 },
};

constexpr GenTest mid_tests[] =  {
  { .best_value=117957, .max_weight=1098, .item_count=51, .avg_weight=43, .seed=545950422 },
  { .best_value=110358, .max_weight=1053, .item_count=53, .avg_weight=39, .seed=3292394601 },
  { .best_value=107152, .max_weight=1018, .item_count=53, .avg_weight=38, .seed=2200004422 },
  { .best_value=113588, .max_weight=1061, .item_count=50, .avg_weight=42, .seed=486321110 },
  { .best_value=107046, .max_weight=1036, .item_count=54, .avg_weight=38, .seed=2254707491 },
  { .best_value=112075, .max_weight=1061, .item_count=50, .avg_weight=42, .seed=393433098 },
  { .best_value=115908, .max_weight=1083, .item_count=52, .avg_weight=41, .seed=4016305763 },
  { .best_value=111716, .max_weight=1075, .item_count=50, .avg_weight=43, .seed=3398840265 },
  { .best_value=110444, .max_weight=1047, .item_count=52, .avg_weight=40, .seed=543285653 },
  { .best_value=111798, .max_weight=1042, .item_count=51, .avg_weight=40, .seed=4257094173 },
  { .best_value=116979, .max_weight=1097, .item_count=50, .avg_weight=43, .seed=220369618 },
  { .best_value=112441, .max_weight=1080, .item_count=54, .avg_weight=40, .seed=2067318742 },
  { .best_value=108468, .max_weight=1054, .item_count=51, .avg_weight=41, .seed=878212200 },
  { .best_value=111800, .max_weight=1086, .item_count=53, .avg_weight=40, .seed=1032145022 },
  { .best_value=107720, .max_weight=1024, .item_count=53, .avg_weight=38, .seed=1385597312 },
  { .best_value=117036, .max_weight=1084, .item_count=54, .avg_weight=40, .seed=1615854086 },
  { .best_value=116096, .max_weight=1087, .item_count=53, .avg_weight=41, .seed=4058702599 },
  { .best_value=106020, .max_weight=1019, .item_count=51, .avg_weight=39, .seed=2577848314 },
  { .best_value=110623, .max_weight=1034, .item_count=51, .avg_weight=40, .seed=3059485874 },
  { .best_value=107184, .max_weight=1014, .item_count=52, .avg_weight=39, .seed=902457258 },
};

constexpr GenTest big_tests[] =  {
  { .best_value=9117870, .max_weight=85839, .item_count=1013, .avg_weight=169, .seed=2794198006 },
  { .best_value=8695136, .max_weight=82056, .item_count=1056, .avg_weight=155, .seed=2899145875 },
  { .best_value=8864184, .max_weight=83093, .item_count=1010, .avg_weight=164, .seed=3925242915 },
  { .best_value=8567272, .max_weight=80876, .item_count=1021, .avg_weight=158, .seed=2933229649 },
  { .best_value=9111983, .max_weight=85411, .item_count=1003, .avg_weight=170, .seed=1912002139 },
  { .best_value=8799671, .max_weight=82837, .item_count=1041, .avg_weight=159, .seed=3533840663 },
  { .best_value=9187872, .max_weight=86310, .item_count=1035, .avg_weight=166, .seed=3986533362 },
  { .best_value=8540028, .max_weight=80392, .item_count=1075, .avg_weight=149, .seed=2666294446 },
  { .best_value=8644940, .max_weight=81020, .item_count=1000, .avg_weight=162, .seed=211985912 },
  { .best_value=9281826, .max_weight=86835, .item_count=1092, .avg_weight=159, .seed=3401883063 },
  { .best_value=8682409, .max_weight=81228, .item_count=1041, .avg_weight=156, .seed=1296573887 },
  { .best_value=8976213, .max_weight=84260, .item_count=1047, .avg_weight=160, .seed=1122115022 },
  { .best_value=8736989, .max_weight=82046, .item_count=1032, .avg_weight=159, .seed=969166099 },
  { .best_value=8471593, .max_weight=80009, .item_count=1046, .avg_weight=152, .seed=2909110692 },
  { .best_value=9252982, .max_weight=86905, .item_count=1042, .avg_weight=166, .seed=4096498724 },
  { .best_value=9408253, .max_weight=87902, .item_count=1084, .avg_weight=162, .seed=3200223221 },
  { .best_value=8580549, .max_weight=80860, .item_count=1057, .avg_weight=152, .seed=318212205 },
  { .best_value=8903048, .max_weight=83612, .item_count=1012, .avg_weight=165, .seed=1554383788 },
  { .best_value=9391384, .max_weight=87854, .item_count=1039, .avg_weight=169, .seed=4083052429 },
  { .best_value=9329217, .max_weight=87608, .item_count=1056, .avg_weight=165, .seed=1716552479 },
};

#define OK() do {} while (0)
#define CHECK(succ, ...) do { \
    if (succ) break; \
    fprintf(stderr, __VA_ARGS__); \
    assert(0); \
  } while (0)

void check_sol(const Test& t, const std::vector<bool>& sol) {
  CHECK(t.items.size() == sol.size(), "Wrong length of the solution.\n");

  unsigned weight = 0, value = 0;
  for (size_t i = 0; i < t.items.size(); i++) {
    if (!sol[i]) continue;
    weight += t.items[i].weight;
    value += t.items[i].value;
  }

  CHECK(weight <= t.max_weight, "Selected items are too heavy.\n");
  CHECK(value == t.best_value,
    "Expected value %u but got %u.\n", t.best_value, value);

  OK();
}

void run_gen_tests(const auto& tests) {
  for (const GenTest& t : tests) {
    const Test test = {
      t.best_value, t.max_weight,
      gen_items(t.item_count, t.avg_weight, t.seed)
    };

    check_sol(test, knapsack(test.items, test.max_weight));
  }
}

int main() {
  for (auto&& t : tests)
    check_sol(t, knapsack(t.items, t.max_weight));

  std::cout << "Small tests..." << std::endl;
  run_gen_tests(small_tests);
//   std::cout << "Medium tests..." << std::endl;
//   run_gen_tests(mid_tests);
//   std::cout << "Big tests..." << std::endl;
//   run_gen_tests(big_tests);

  std::cout << "All tests passed." << std::endl;
}

#endif

