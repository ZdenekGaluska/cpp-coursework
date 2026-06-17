# cpp-algorithms

C++ coursework from FIT CTU Prague — algorithms, data structures, and systems programming.

## Contents

| File | What it demonstrates |
|------|----------------------|
| `avl_lazy_range_tree.cpp` | Self-balancing AVL tree with lazy propagation for O(log n) range updates on a sorted set |
| `knapsack_dp.cpp` | 0/1 Knapsack via top-down memoised DP with backtracking for item selection |
| `lis_longest_increasing_subsequence.cpp` | Longest Increasing Subsequence with O(n²) DP and index reconstruction |
| `linker.cpp` | Minimal object-file linker: binary `.o` parsing, BFS dependency resolution, address relocation |
| `polynomial.cpp` | Sparse polynomial with full operator overloading (`*`, `[]`, `()`, `<<`, `bool`) |
| `population_register.cpp` | Sorted register with copy-on-write semantics and manual memory management |

> `linker.cpp` requires binary `.o` test files from the original assignment to run its tests.
