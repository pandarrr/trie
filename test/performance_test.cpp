#pragma once

#include <map>
#include <chrono>
#include <random>
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <algorithm>
#include <gtest/gtest.h>
#include "../src/trie.h"

static const std::string _alpha =
  "abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "1234567890";

class PerformanceTest : public ::testing::Test {
public:

  template <class PredT>
  void measure_time(const PredT &pred) {
    auto start = std::chrono::high_resolution_clock::now();
    pred();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "\tTime: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
  }

  template <class PredT>
  void measure_memory(const PredT &pred) {
    PROCESS_MEMORY_COUNTERS mem_start, mem_end;
    GetProcessMemoryInfo(GetCurrentProcess(), &mem_start, sizeof mem_start);
    pred();
    GetProcessMemoryInfo(GetCurrentProcess(), &mem_end, sizeof mem_end);
    if (mem_end.PeakWorkingSetSize < mem_start.PeakWorkingSetSize)
      std::cout << "\tMemory: no increase\n";
    else
      std::cout << "\tMemory: " << (mem_end.PeakWorkingSetSize - mem_start.PeakWorkingSetSize) / 1024 << "kb\n";
  }

  template <class PredT>
  void measure(const PredT &pred) {
    measure_memory([&]()
    {
      measure_time(pred);
    });
  }

  std::vector<std::string> random_string_set(int size, int max) {
    std::vector<std::string> strings;
    for (int i = 0; i < size; ++i)
      strings.push_back(random_string(max));
    return std::move(strings);
  }

  std::vector<std::string> random_prefix_string_set(int size, int max) {
    std::vector<std::string> strings;
    for (int i = 0; i < size; ++i) {
      std::string result;
      for (int k = 0; k < random_num(1, max); ++k) {
        result += random_alpha();
        strings.push_back(result);
      }
    }
    return std::move(strings);
  }

  std::string random_string(int max) {
    std::string result;
    for (int k = 0; k < random_num(1, max); ++k)
      result += random_alpha();
    return result;
  }

  char random_alpha() {
    return _alpha[uniform(0, _alpha.size() - 1)];
  }

  int random_num(int min, int max) {
    return uniform(min, max);
  }

  int uniform(int min, int max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    return std::uniform_int_distribution<>(min, max)(gen);
  }

};

TEST_F(PerformanceTest, Construction) {
  const int n = 10000;

  std::cout << "Map:\n";
  measure([&]() {
    for (int i = 0; i < n; ++i)
      std::map<std::string, int> _map;
  });

  std::cout << "Trie:\n";
  measure([&]() {
    for (int i = 0; i < n; ++i)
      trie<char, int> _trie(_alpha);
  });
}

TEST_F(PerformanceTest, Basic_Insert_Same_Key) {
  const int n = 100000;
  std::string _key = "panda";

  std::cout << "Map:\n";
  measure([&]() {
    std::map<std::string, int> _map;
    for (int i = 0; i < n; ++i)
      _map[_key] = 10;
  });

  std::cout << "Trie:\n";
  measure([&]() {
    trie<char, int> _trie(_alpha);
    for (int i = 0; i < n; ++i)
      _trie[_key] = 10;
  });
}

TEST_F(PerformanceTest, Heavy_Insert_Prefixes) {
  const int _words = 50000;
  const int _max_len = 26;
  auto _keys = random_prefix_string_set(_words, _max_len);

  std::random_shuffle(_keys.begin(), _keys.end());

  std::cout << "Map:\n";
  measure([&]() {
    std::map<std::string, int> _map;
    for (auto &_key : _keys)
      _map[_key] = 10;
  });

  std::cout << "Trie:\n";
  measure([&]() {
    trie<char, int> _trie(_alpha);
    for (auto &_key : _keys)
      _trie[_key] = 10;
  });
}

TEST_F(PerformanceTest, Heavy_Insert) {
  const int _words = 150000;
  const int _max_len = 26;
  auto _keys = random_string_set(_words, _max_len);

  std::cout << "Map:\n";
  measure([&]() {
    std::map<std::string, int> _map;
    for (auto &_key : _keys)
      _map[_key] = 10;
  });

  std::cout << "Trie:\n";
  measure([&]() {
    trie<char, int> _trie(_alpha);
    for (auto &_key : _keys)
      _trie[_key] = 10;
  });
}

TEST_F(PerformanceTest, Heavy_Insert_Prefixes_With_Iteration) {
  const int _words = 50000;
  const int _max_len = 26;
  auto _keys = random_prefix_string_set(_words, _max_len);

  std::random_shuffle(_keys.begin(), _keys.end());

  std::map<std::string, int> _map;
  trie<char, int> _trie(_alpha);
  for (auto &_key : _keys) {
    _map[_key] = 10;
    _trie[_key] = 10;
  }

  std::cout << "Map:\n";
  measure([&]() {
    for (auto &it : _map)
      ; // do nothing
  });

  std::cout << "Trie:\n";
  measure([&]() {
    for (auto &it : _trie)
      ; // do nothing
  });
}

TEST_F(PerformanceTest, Heavy_Retrieval_Prefixes) {
  const int _words = 50000;
  const int _max_len = 26;
  auto _keys = random_prefix_string_set(_words, _max_len);

  std::random_shuffle(_keys.begin(), _keys.end());

  std::map<std::string, int> _map;
  trie<char, int> _trie(_alpha);
  for (auto &_key : _keys) {
    _map[_key] = 10;
    _trie[_key] = 10;
  }

  std::random_shuffle(_keys.begin(), _keys.end());

  const int iterations = 10;

  std::cout << "Map:\n";
  measure([&]() {
    for (int i = 0; i < iterations; ++i)
      for (auto &it : _keys)
        auto val = _map[it];
  });

  std::cout << "Trie:\n";
  measure([&]() {
    for (int i = 0; i < iterations; ++i)
      for (auto &it : _keys)
        auto val = _trie[it];
  });
}
