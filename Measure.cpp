#include <iostream>
#include <chrono>
#include <random>

#include <set>
#include <map>
#include <list>
#include <forward_list>

#include "Allocator.h"
#include <iostream>
#include <omp.h>
const std::size_t growSize = 1024;
const int numberOfIterations = 1024;
const int randomRange = 1024;

template <typename Container>
class PerformanceTest {
  virtual void testIteration(int newSize) = 0;


 protected:

  Container container;

  std::default_random_engine randomNumberGenerator;
  std::uniform_int_distribution<int> randomNumberDistribution;


 public:

  PerformanceTest() :
    randomNumberGenerator(0),
    randomNumberDistribution(0, randomRange) {
  }

  double run() {
    auto from = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < numberOfIterations; i++)
      testIteration(randomNumberDistribution(randomNumberGenerator));

    auto to = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::duration<double>>(to - from).count();
  }
};

template <typename Container>
class PushFrontTest : public PerformanceTest<Container> {
  virtual void testIteration(int newSize) {
    int size = 0;

    while (size < newSize)
      this->container.push_front(size++);

    for (; size > newSize; size--)
      this->container.pop_front();
  }
};

template <typename Container>
class PushBackTest : public PerformanceTest<Container> {
  virtual void testIteration(int newSize) {
    int size = 0;

    while (size < newSize)
      this->container.push_back(size++);

    for (; size > newSize; size--)
      this->container.pop_back();
  }
};

template <typename Container>
class MapTest : public PerformanceTest<Container> {
  virtual void testIteration(int newSize) {
    int size = 0;

    while (size < newSize)
      this->container.insert(std::pair<char, int>(size++, size));

    while (size > newSize)
      this->container.erase(--size);
  }
};

template <typename Container>
class SetTest : public PerformanceTest<Container> {
  virtual void testIteration(int newSize) {
    int size = 0;

    while (size < newSize)
      this->container.insert(size++);

    while (size > newSize)
      this->container.erase(--size);
  }
};

template <typename StlContainer, typename FastContainer>
void printTestStatus(const char *name, StlContainer &stlContainer, FastContainer &fastContainer) {
  std::cout << std::fixed;
  std::cout << name << " - Default STL Allocator : " << stlContainer.run() << " seconds." << std::endl;
  std::cout << name << " - Memory Pool Allocator : " << fastContainer.run() << " seconds." << std::endl;
  std::cout << std::endl;
}

template<class Alloc>
std::ostream &operator<<(std::ostream &ostr, std::list<int, Alloc> &list) {
  for (auto &i : list) {
    ostr << " " << i;
  }
  return ostr;
}
void sortTest() {
  std::list<int, Moya::Allocator<int, 32>>l;
  for (int i = 10; i > 0; i--) {
    l.push_back(i);
  }
  std::cout << "before sort:" << l << "\n";
  l.sort();
  std::cout << "after sort :" << l << "\n";
}
void spiceTest() {
  std::list<int, Moya::Allocator<int, 32>> list1 = { 1, 2, 3, 4, 5 };
  std::list<int, Moya::Allocator<int, 32>> list2 = { 10, 20, 30, 40, 50 };

  auto it = list1.begin();
  std::advance(it, 2);

  list1.splice(it, list2);

  std::cout << "list1: " << list1 << "\n";
  std::cout << "list2: " << list2 << "\n";

  list2.splice(list2.begin(), list1, it, list1.end());

  std::cout << "list1: " << list1 << "\n";
  std::cout << "list2: " << list2 << "\n";
}

template<class Alloc>
void pushbackTest(int n, char *str) {
  double t0 = omp_get_wtime();
  {
    std::list<int, Alloc> l;
    for (int i = 0; i < n; i++) {
      l.push_back(i);
    }
  }
  double t1 = omp_get_wtime();
  printf("pusbackTest(%s) used time %fs\n", str, t1 - t0);
}

int main() {
  sortTest();
  spiceTest();
  int n = 1024;
  pushbackTest<std::allocator<int>>(n, "std 1024");
  pushbackTest<Moya::Allocator<int, 1024>>(n, "allocator 1024");
  pushbackTest<std::allocator<int>>(n * n, "std 1024*1024");
  pushbackTest<Moya::Allocator<int, 1024>>(n * n, "allocator 1024*1024");
  typedef int DataType;
  typedef Moya::Allocator<DataType, growSize> MemoryPoolAllocator;

  std::cout << "Allocator performance measurement example" << std::endl;
  std::cout << "Version: 1.0" << std::endl << std::endl;

  PushFrontTest<std::forward_list<DataType>> pushFrontForwardListTestStl;
  PushFrontTest<std::forward_list<DataType, MemoryPoolAllocator>> pushFrontForwardListTestFast;
  printTestStatus("ForwardList PushFront", pushFrontForwardListTestStl, pushFrontForwardListTestFast);

  PushFrontTest<std::list<DataType>> pushFrontListTestStl;
  PushFrontTest<std::list<DataType, MemoryPoolAllocator>> pushFrontListTestFast;
  printTestStatus("List PushFront", pushFrontListTestStl, pushFrontListTestFast);

  PushBackTest<std::list<DataType>> pushBackListTestStl;
  PushBackTest<std::list<DataType, MemoryPoolAllocator>> pushBackListTestFast;
  printTestStatus("List PushBack", pushBackListTestStl, pushBackListTestFast);

  MapTest<std::map<DataType, DataType, std::less<DataType>>> mapTestStl;
  MapTest<std::map<DataType, DataType, std::less<DataType>, MemoryPoolAllocator>> mapTestFast;
  printTestStatus("Map", mapTestStl, mapTestFast);

  SetTest<std::set<DataType, std::less<DataType>>> setTestStl;
  SetTest<std::set<DataType, std::less<DataType>, MemoryPoolAllocator>> setTestFast;
  printTestStatus("Set", setTestStl, setTestFast);

  return 0;
}
