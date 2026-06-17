#ifndef __PROGTEST__
#include <cassert>
#include <iomanip>
#include <cstdint>
#include <iostream>
#include <memory>
#include <limits>
#include <optional>
#include <algorithm>
#include <functional>
#include <bitset>
#include <list>
#include <array>
#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <random>
#include <type_traits>
#include <utility>

struct Hobbit {
  std::string name;
  int hp, off, def;

  friend bool operator == (const Hobbit&, const Hobbit&) = default;
};

std::ostream& operator << (std::ostream& out, const Hobbit& h) {
  return out
    << "Hobbit{\"" << h.name << "\", "
    << ".hp=" << h.hp << ", "
    << ".off=" << h.off << ", "
    << ".def=" << h.def << "}";
}

template < typename T >
std::ostream& operator << (std::ostream& out, const std::optional<T>& x) {
  if (!x) return out << "EMPTY_OPTIONAL";
  return out << "Optional{" << *x << "}";
}

#endif


struct HobbitArmy {
  static constexpr bool CHECK_NEGATIVE_HP = true;


  struct Node {
    Hobbit hobbit;
    Node* left;
    Node* right;
    int height;

    mutable int lazy_hp = 0;
    mutable int lazy_off = 0;
    mutable int lazy_def = 0;

    int min_hp = 0;
    
    std::string min_name;
    std::string max_name;

    Node(const Hobbit& hob) : hobbit(hob), left(nullptr), right(nullptr), height(1), min_hp(hob.hp), 
    min_name(hob.name), max_name(hob.name) {}
  };

  void destroyTree(Node* n) {
    if(!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
  }
  ~HobbitArmy(){
    destroyTree(root);
  }

  Node* root = nullptr;

  inline int getHeight(Node* n) {
    if (n) {
        return n->height;
    }
    return 0;
  }

  inline int getBalance(Node* n) {
    if (n) {
        return getHeight(n->right) - getHeight(n->left);
    }
    return 0;
  }

  inline void updateHeight(Node* n) {
    if (n) {
        n->height = 1 + std::max(getHeight(n->left), getHeight(n->right));
    }
  }

  static inline void updateMinHP(Node* n) {
    if (!n) return;
    
    n->min_hp = n->hobbit.hp;
    
    if (n->left) {
      n->min_hp = std::min(n->min_hp, n->left->min_hp + n->left->lazy_hp);
    }
    
    if (n->right) {
      n->min_hp = std::min(n->min_hp, n->right->min_hp + n->right->lazy_hp);
    }
  }

  static inline void updateMinMax(Node* n) {
    if (!n) return;

    n->min_name = n->hobbit.name;
    n->max_name = n->hobbit.name;

    if (n->left) {
      n->min_name = n->left->min_name;
    }
    if (n->right) {
      n->max_name = n->right->max_name;
    }
  }

  static void pushLazy(Node* n) {
    if(!n) return;

    n->hobbit.def += n->lazy_def;
    n->hobbit.off += n->lazy_off;
    n->hobbit.hp += n-> lazy_hp;

    if (n->left) {
      n->left->lazy_hp += n->lazy_hp;
      n->left->lazy_off += n->lazy_off;
      n->left->lazy_def += n->lazy_def;
    }

    if (n->right) {
        n->right->lazy_hp += n->lazy_hp;
        n->right->lazy_off += n->lazy_off;
        n->right->lazy_def += n->lazy_def;
    }
    
    n->lazy_hp = 0;
    n->lazy_off = 0;
    n->lazy_def = 0;
    updateMinHP(n);
  }

  Node* rotateLeft(Node* n) {
    pushLazy(n);
    pushLazy(n->right);

    Node* y = n->right;
    Node* B = y->left;


    y->left = n;
    n->right = B;

    updateHeight(n);
    updateHeight(y);
    updateMinMax(n);
    updateMinMax(y);
    updateMinHP(n);
    updateMinHP(y);
    return y;
  }

  Node* rotateRight(Node* n) {
    pushLazy(n);
    pushLazy(n->left);

    Node* y = n->left;
    Node* B = y->right;

    y->right = n;
    n->left = B;

    updateHeight(n);
    updateHeight(y);
    updateMinMax(n);
    updateMinMax(y);
    updateMinHP(n);
    updateMinHP(y);
    return y;
  }

  Node* findMin(Node* n) {
    pushLazy(n);
    while(n->left) {
      pushLazy(n->left);
      n = n->left;
    }
    return n;
  }

  Node* insert(Node* n, const Hobbit& hobbit, bool& inserted) {
    if(!n) {
        inserted = true;
        return new Node(hobbit);
    }

    pushLazy(n);

    const int cmp = hobbit.name.compare(n->hobbit.name);
    if (cmp < 0) {
        n->left = insert(n->left, hobbit, inserted);
    } else if(cmp > 0) {
        n->right = insert(n->right, hobbit, inserted);
    }   
    else {
        inserted = false;
        return n;
    }

    updateHeight(n);
    updateMinMax(n);
    updateMinHP(n);
    int balance = getBalance(n);

    if (balance < -1) {
      if (n->left && hobbit.name < n->left->hobbit.name) {
        return rotateRight(n);
      } else if (n->left){
        n->left = rotateLeft(n->left);
        return rotateRight(n);
      }
    }
    
    if (balance > 1) {
      if (n->right && hobbit.name > n->right->hobbit.name) {
        return rotateLeft(n);
      } else if (n->right){
        n->right = rotateRight(n->right);
        return rotateLeft(n);
      }
    }

    return n;
  }

  Node* eraseNode(Node* n, const std::string& name, std::optional<Hobbit>& result) {
    if (!n) {
      result = std::nullopt;
      return nullptr;
    }

    pushLazy(n);

    const int cmp = name.compare(n->hobbit.name);
    if (cmp > 0) {
      n->right = eraseNode(n->right, name, result);
    } else if (cmp < 0) {
      n->left = eraseNode(n->left, name, result);
    } else {
      result = n->hobbit;

      if (!n->left && !n->right) {
        delete n; 
        return nullptr;
      } else if (!n->left || !n->right) {
        Node* temp = nullptr;

        if (n->left) {
          temp = n->left;
        } else {
          temp = n->right;
        }
      
        delete n; 
        return temp;
      }
      else {
        Node* successor = findMin(n->right);
        n->hobbit = successor->hobbit;
        pushLazy(successor);
        std::optional<Hobbit> dummy;
        n->right = eraseNode(n->right, successor->hobbit.name, dummy);
      }
    }

    if(!n) return nullptr;

    updateHeight(n);
    updateMinMax(n);
    updateMinHP(n);
    int balance = getBalance(n);

    if (balance > 1 && getBalance(n->right) >= 0) {
        return rotateLeft(n);
    }
    if (balance > 1 && getBalance(n->right) < 0) {
        n->right = rotateRight(n->right);
        return rotateLeft(n);
    }
    if (balance < -1 && getBalance(n->left) <= 0) {
        return rotateRight(n);
    }
    if (balance < -1 && getBalance(n->left) > 0) {
        n->left = rotateLeft(n->left);
        return rotateRight(n);
    }
    return n;
  }

  bool add(const Hobbit& hobbit) {
    if (hobbit.hp <= 0) return false;
    if constexpr(CHECK_NEGATIVE_HP) {
        if (hobbit.hp < 0) return false;
    }

    bool inserted = false;
    root = insert(root, hobbit, inserted);
    return inserted;
  }

  std::optional<Hobbit> erase(const std::string& hobbit_name) {
    std::optional<Hobbit> result;
    root = eraseNode(root, hobbit_name, result);
    return result;
  }

  std::optional<Hobbit> stats(const std::string& hobbit_name) const {
    Node* current = root;
    while (current) {
      pushLazy(current); 
      if (hobbit_name == current->hobbit.name) {
        return current->hobbit;
      } else if (hobbit_name > current->hobbit.name) {
        current = current->right;
      } else {
        current = current->left;
      }
    }
    return std::nullopt;
  }

  void enchant_push(Node* n, const std::string& first, const std::string& last, int hp_diff, int off_diff, int def_diff) {
    if (!n) return;
    pushLazy(n);

    if (n->max_name < first || n->min_name > last) return;

    if (n->min_name >= first && n->max_name <= last) {
      n->lazy_hp += hp_diff;   
      n->lazy_off += off_diff;
      n->lazy_def += def_diff;
      updateMinHP(n);
      return;  
    }

    if (n->hobbit.name >= first && n->hobbit.name <= last) {
      n->hobbit.hp += hp_diff;
      n->hobbit.off += off_diff;
      n->hobbit.def += def_diff;
    }

    enchant_push(n->right, first, last, hp_diff, off_diff, def_diff);
    enchant_push(n->left, first, last, hp_diff, off_diff, def_diff);
    updateMinHP(n);
    return;
  }
 
  bool check_if_enchantable(Node* n, const std::string& first, const std::string& last, int hp_diff, int current_lazy = 0) const {
    if (!n) return true;
    
    int total_lazy = current_lazy + n->lazy_hp;

    if (n->max_name < first || n->min_name > last) return true;

    if (n->max_name <= last && n->min_name >= first) {
        return n->min_hp + total_lazy + hp_diff > 0;
    }

    if (n->hobbit.name <= last && n->hobbit.name >= first) {
        if (n->hobbit.hp + total_lazy + hp_diff <= 0) return false;
    }
    
    return check_if_enchantable(n->left, first, last, hp_diff, total_lazy) && check_if_enchantable(n->right, first, last, hp_diff, total_lazy);
  }

  bool enchant(
    const std::string& first,
    const std::string& last,
    int hp_diff,
    int off_diff,
    int def_diff
  ) {
    if (first > last) return true;  
    
    if (!CHECK_NEGATIVE_HP) {
      enchant_push(root, first, last, hp_diff, off_diff, def_diff);
      return true;
    } else {
      if (check_if_enchantable(root, first, last, hp_diff)) {
        enchant_push(root, first, last, hp_diff, off_diff, def_diff);
        return true;
      }
      return false;
    }
  }

  void for_each(auto&& fun) const {
    for_each_impl(root, fun);
  }

  private:
  static void for_each_impl(Node *node, auto& fun) {
    if (!node) return;
    pushLazy(node);
    for_each_impl(node->left, fun);
    fun(node->hobbit);
    for_each_impl(node->right, fun);
  }
};

#ifndef __PROGTEST__

////////////////// Dark magic, ignore ////////////////////////

template < typename T >
auto quote(const T& t) { return t; }

std::string quote(const std::string& s) {
  std::string ret = "\"";
  for (char c : s) if (c != '\n') ret += c; else ret += "\\n";
  return ret + "\"";
}

#define STR_(a) #a
#define STR(a) STR_(a)

#define CHECK_(a, b, a_str, b_str) do { \
    auto _a = (a); \
    decltype(a) _b = (b); \
    if (_a != _b) { \
      std::cout << "Line " << __LINE__ << ": Assertion " \
        << a_str << " == " << b_str << " failed!" \
        << " (lhs: " << quote(_a) << ")" << std::endl; \
      fail++; \
    } else ok++; \
  } while (0)

#define CHECK(a, b) CHECK_(a, b, #a, #b)

 
////////////////// End of dark magic ////////////////////////


void check_army(const HobbitArmy& A, const std::vector<Hobbit>& ref, int& ok, int& fail) {
  size_t i = 0;

  A.for_each([&](const Hobbit& h) {
    CHECK(i < ref.size(), true);
    CHECK(h, ref[i]);
    i++;
  });

  CHECK(i, ref.size());
}

void test1(int& ok, int& fail) {
  HobbitArmy A;
  check_army(A, {}, ok, fail);

  CHECK(A.add({"Frodo", 100, 10, 3}), true);
  CHECK(A.add({"Frodo", 200, 10, 3}), false);
  CHECK(A.erase("Frodo"), std::optional(Hobbit("Frodo", 100, 10, 3)));
  CHECK(A.add({"Frodo", 200, 10, 3}), true);

  CHECK(A.add({"Sam", 80, 10, 4}), true);
  CHECK(A.add({"Pippin", 60, 12, 2}), true);
  CHECK(A.add({"Merry", 60, 15, -3}), true);
  CHECK(A.add({"Smeagol", 0, 100, 100}), false);

  if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
    CHECK(A.add({"Smeagol", -100, 100, 100}), false);

  CHECK(A.add({"Smeagol", 200, 100, 100}), true);

  CHECK(A.enchant("Frodo", "Frodo", 10, 1, 1), true);
  CHECK(A.enchant("Sam", "Frodo", -1000, 1, 1), true); // empty range
  CHECK(A.enchant("Bilbo", "Bungo", 1000, 0, 0), true); // empty range
  
  if constexpr(HobbitArmy::CHECK_NEGATIVE_HP)
    CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), false);

  CHECK(A.enchant("Frodo", "Sam", 1, 0, 0), true);
  CHECK(A.enchant("Frodo", "Sam", -60, 1, 1), true);

  CHECK(A.stats("Gandalf"), std::optional<Hobbit>{});
  CHECK(A.stats("Frodo"), std::optional(Hobbit("Frodo", 151, 12, 5)));
  CHECK(A.stats("Merry"), std::optional(Hobbit("Merry", 1, 16, -2)));

  check_army(A, {
    {"Frodo", 151, 12, 5},
    {"Merry", 1, 16, -2},
    {"Pippin", 1, 13, 3},
    {"Sam", 21, 11, 5},
    {"Smeagol", 200, 100, 100},
  }, ok, fail);
}

int main() {
  int ok = 0, fail = 0;
  test1(ok, fail);

  if (!fail) std::cout << "Passed all " << ok << " tests!" << std::endl;
  else std::cout << "Failed " << fail << " of " << (ok + fail) << " tests." << std::endl;
}

#endif

