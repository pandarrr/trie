#pragma once

#include <gtest/gtest.h>
#include "../src/trie.h"
#include <algorithm>

static const std::string _alpha =
  "abcdefghijklmnopqrstuvwxyz"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "1234567890";

class TrieTest : public ::testing::Test {
public:
  trie<char, int> _trie;

  TrieTest() : _trie(_alpha) {}
};

TEST_F(TrieTest, Basic_Insert_Retrieval) {
  _trie["panda"] = 1;
  EXPECT_EQ(1, _trie["panda"]);
}

TEST_F(TrieTest, Multi_Insert_Retrieval) {
  _trie["panda"] = 1;
  _trie["polar"] = 2;
  _trie["koala"] = 3;
  _trie["grizzly"] = 4;

  EXPECT_EQ(1, _trie["panda"]);
  EXPECT_EQ(2, _trie["polar"]);
  EXPECT_EQ(3, _trie["koala"]);
  EXPECT_EQ(4, _trie["grizzly"]);
}

TEST_F(TrieTest, Multi_Insert_Retrieval_Prefixed) {
  _trie["p"] = 1;
  _trie["po"] = 2;
  _trie["pol"] = 3;
  _trie["pola"] = 4;
  _trie["polar"] = 5;

  EXPECT_EQ(1, _trie["p"]);
  EXPECT_EQ(2, _trie["po"]);
  EXPECT_EQ(3, _trie["pol"]);
  EXPECT_EQ(4, _trie["pola"]);
  EXPECT_EQ(5, _trie["polar"]);
}

TEST_F(TrieTest, Size) {
  _trie["panda"] = 1;
  EXPECT_EQ(1, _trie.size());

  _trie["pand"] = 1;
  EXPECT_EQ(2, _trie.size());

  _trie["pan"] = 1;
  EXPECT_EQ(3, _trie.size());

  _trie["pa"] = 1;
  EXPECT_EQ(4, _trie.size());
}

TEST_F(TrieTest, Insert_Existence_Check) {
  _trie["polar"] = 5;
  EXPECT_EQ(1, _trie.size());
  EXPECT_FALSE(_trie.has("p"));
  EXPECT_FALSE(_trie.has("po"));
  EXPECT_FALSE(_trie.has("pol"));
  EXPECT_FALSE(_trie.has("pola"));
  EXPECT_TRUE(_trie.has("polar"));
}

TEST_F(TrieTest, Complex_Insert_Existence_Check) {
  _trie["polar"] = 5;
  _trie["polarize"] = 5;
  _trie["polarity"] = 5;
  _trie["poland"] = 5;
  EXPECT_EQ(4, _trie.size());
  EXPECT_FALSE(_trie.has("polan"));
  EXPECT_FALSE(_trie.has("polari"));
  EXPECT_FALSE(_trie.has("polarit"));
  EXPECT_FALSE(_trie.has("polariz"));

  EXPECT_TRUE(_trie.has("polarize"));
  EXPECT_TRUE(_trie.has("polarity"));
  EXPECT_TRUE(_trie.has("poland"));
}

TEST_F(TrieTest, Basic_Erase) {
  _trie["polar"] = 5;
  _trie["polarize"] = 5;
  _trie["polarity"] = 5;
  EXPECT_EQ(3, _trie.size());
  EXPECT_TRUE(_trie.has("polar"));
  EXPECT_TRUE(_trie.has("polarize"));
  EXPECT_TRUE(_trie.has("polarity"));

  EXPECT_EQ(1, _trie.erase("polar"));
  EXPECT_EQ(2, _trie.size());
  EXPECT_FALSE(_trie.has("polar"));
  EXPECT_TRUE(_trie.has("polarize"));
  EXPECT_TRUE(_trie.has("polarity"));

  EXPECT_EQ(1, _trie.erase("polarize"));
  EXPECT_EQ(1, _trie.size());
  EXPECT_FALSE(_trie.has("polar"));
  EXPECT_FALSE(_trie.has("polarize"));
  EXPECT_TRUE(_trie.has("polarity"));

  EXPECT_EQ(1, _trie.erase("polarity"));
  EXPECT_EQ(0, _trie.size());
  EXPECT_FALSE(_trie.has("polar"));
  EXPECT_FALSE(_trie.has("polarize"));
  EXPECT_FALSE(_trie.has("polarity"));
}

TEST_F(TrieTest, Basic_Erase_2) {
  _trie["p"] = 1;
  _trie["po"] = 2;
  _trie["pol"] = 3;
  _trie["pola"] = 4;
  _trie["polar"] = 5;

  EXPECT_EQ(5, _trie.size());
  EXPECT_EQ(1, _trie.erase("polar"));
  EXPECT_EQ(4, _trie.size());
  EXPECT_TRUE(_trie.has("p"));
  EXPECT_TRUE(_trie.has("po"));
  EXPECT_TRUE(_trie.has("pol"));
  EXPECT_TRUE(_trie.has("pola"));
  EXPECT_FALSE(_trie.has("polar"));
}

TEST_F(TrieTest, Insert_And_Erase) {
  std::vector<std::string> values;
  values.push_back("polar");
  values.push_back("poland");
  values.push_back("grizzly");
  values.push_back("polarize");
  values.push_back("koala");
  values.push_back("panda");
  values.push_back("polarity");

  for (auto &it : values)
    _trie[it] = 5;
  EXPECT_EQ(values.size(), _trie.size());

  std::random_shuffle(values.begin(), values.end());

  for (auto &it : values)
    EXPECT_EQ(1, _trie.erase(it));
  EXPECT_EQ(0, _trie.size());

  std::random_shuffle(values.begin(), values.end());

  for (auto &it : values)
    EXPECT_FALSE(_trie.has(it));
}

TEST_F(TrieTest, Iteration) {
  _trie["panda"] = 1;
  _trie["polar"] = 2;
  _trie["koala"] = 3;
  _trie["grizzly"] = 4;


  std::vector<std::string> actual_bears;
  std::vector<int> actual_values;
  for (auto &bear : _trie) {
    actual_bears.push_back(bear.key<std::string>());
    actual_values.push_back(bear.value());
  }

  std::vector<std::string> expected_bears;
  expected_bears.push_back("grizzly");
  expected_bears.push_back("koala");
  expected_bears.push_back("panda");
  expected_bears.push_back("polar");

  std::vector<int> expected_values;
  expected_values.push_back(4);
  expected_values.push_back(3);
  expected_values.push_back(1);
  expected_values.push_back(2);

  EXPECT_EQ(expected_bears, actual_bears);
  EXPECT_EQ(expected_values, actual_values);
}

TEST_F(TrieTest, Iterate_Empty) {
  for (auto &it : _trie) {
    // Do nothing
  }
}

TEST_F(TrieTest, Reverse_Iterate) {
  _trie["panda"] = 1;
  _trie["polar"] = 2;
  _trie["koala"] = 3;
  _trie["grizzly"] = 4;

  std::vector<std::string> actual_bears;
  std::vector<int> actual_values;
  for (auto start = _trie.rbegin(), end = _trie.rend(); start != end; ++start) {
    actual_bears.push_back(start->key<std::string>());
    actual_values.push_back(start->value());
  }

  std::vector<std::string> expected_bears;
  expected_bears.push_back("polar");
  expected_bears.push_back("panda");
  expected_bears.push_back("koala");
  expected_bears.push_back("grizzly");

  std::vector<int> expected_values;
  expected_values.push_back(2);
  expected_values.push_back(1);
  expected_values.push_back(3);
  expected_values.push_back(4);

  EXPECT_EQ(expected_bears, actual_bears);
  EXPECT_EQ(expected_values, actual_values);
}

TEST_F(TrieTest, Erase_Iterator) {
  _trie["panda"] = 1;
  _trie["polar"] = 2;
  _trie["koala"] = 3;
  _trie["grizzly"] = 4;

  EXPECT_EQ(3, _trie.erase(_trie.begin())->value());
  EXPECT_EQ(1, _trie.erase(_trie.begin())->value());
  EXPECT_EQ(2, _trie.erase(_trie.begin())->value());
  EXPECT_EQ(_trie.end(), _trie.erase(_trie.begin()));
}

TEST_F(TrieTest, Erase_Iterator_From_End) {
  _trie["panda"] = 1;
  _trie["polar"] = 2;
  _trie["koala"] = 3;
  _trie["grizzly"] = 4;

  EXPECT_EQ(_trie.end(), _trie.erase(--_trie.end()));
  EXPECT_EQ(3, _trie.size());
  EXPECT_EQ(_trie.end(), _trie.erase(--_trie.end()));
  EXPECT_EQ(2, _trie.size());
  EXPECT_EQ(_trie.end(), _trie.erase(--_trie.end()));
  EXPECT_EQ(1, _trie.size());
  EXPECT_EQ(_trie.end(), _trie.erase(--_trie.end()));
  EXPECT_EQ(0, _trie.size());
}

TEST_F(TrieTest, Erase_Iterator_Range) {
  _trie["panda"] = 1;
  _trie["polar"] = 2;
  _trie["koala"] = 3;
  _trie["grizzly"] = 4;
  _trie["brown"] = 5;
  _trie["russian"] = 6;
  _trie["black"] = 7;

  auto first = ++_trie.begin();
  auto last = --(--_trie.end());

  EXPECT_STREQ("polar", _trie.erase(first, last)->key<std::string>().c_str());
  EXPECT_EQ(3, _trie.size());

  EXPECT_TRUE(_trie.has("black"));
  EXPECT_TRUE(_trie.has("russian"));
  EXPECT_TRUE(_trie.has("polar"));
  EXPECT_FALSE(_trie.has("brown"));
  EXPECT_FALSE(_trie.has("grizzly"));
  EXPECT_FALSE(_trie.has("koala"));
  EXPECT_FALSE(_trie.has("panda"));
}

TEST_F(TrieTest, Clear) {
  _trie["panda"] = 1;
  _trie["polar"] = 2;
  _trie["koala"] = 3;
  _trie["grizzly"] = 4;
  _trie["brown"] = 5;
  _trie["russian"] = 6;
  _trie["black"] = 7;

  EXPECT_EQ(7, _trie.size());
  _trie.clear();
  EXPECT_EQ(0, _trie.size());
}

TEST_F(TrieTest, Insert_Retrieve_Empty_String) {
  _trie[""] = 1;

  EXPECT_EQ(1, _trie.size());
  EXPECT_TRUE(_trie.has(""));
  EXPECT_EQ(1, _trie.begin()->value());

  /*
  TODO:
  Does it make sense to have the end be the root when the root
  is also the node for empty strings?
  */
  EXPECT_EQ(1, _trie.end()->value());
}


struct iless {
  bool operator ()(const char &left, const char &right) const {
    return ::toupper(left) < ::toupper(right);
  }
};

class InsensitveTrieTest : public ::testing::Test {
public:
  trie<char, int, iless> _trie;

  InsensitveTrieTest() : _trie(_alpha) {}
};

TEST_F(InsensitveTrieTest, Basic_Insert) {
  _trie["pAnDa"] = 1;
  _trie["pOLAR"] = 2;
  _trie["Koala"] = 3;
  _trie["grizzly"] = 4;

  EXPECT_EQ(1, _trie["PaNdA"]);
  EXPECT_EQ(2, _trie["pOLAR"]);
  EXPECT_EQ(3, _trie["koala"]);
  EXPECT_EQ(4, _trie["GRIZZLY"]);
}

