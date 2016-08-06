#pragma once

#include <gtest/gtest.h>

int main(int argc, char ** args) {
  ::testing::InitGoogleTest(&argc, args);
  return RUN_ALL_TESTS();
}