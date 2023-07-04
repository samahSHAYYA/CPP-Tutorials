/**
 * @file test_node.cpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains all tests related to the Node template structure
 *        defined node.hpp file.
 */

#include <gtest/gtest.h>

#include "tree/node.hpp"

// Testing BSTNode with value
TEST(BSTNodeWithValueTest, Constructor) {
  int key = 10;
  std::string value = "Hello";
  BSTNode<int, std::string> node(key, value);

  EXPECT_EQ(node.key, key);
  EXPECT_EQ(value, node.value);
  EXPECT_EQ(nullptr, node.left);
  EXPECT_EQ(nullptr, node.right);
}

TEST(BSTNodeWithValueTest, ToString) {
  BSTNode<int, std::string> node(10, "Hello");

  EXPECT_EQ("<K = 10, V = Hello>", node.toString());
}

TEST(BSTNodeWithValueTest, GetKeyValuePair) {
  int key = 10;
  std::string value = "Hello";
  BSTNode<int, std::string> node(key, value);
  std::pair<int, std::string> expected(key, value);

  EXPECT_EQ(expected, node.getKeyValuePair());
}

// Testing BSTNode without value
TEST(BSTNodeWithoutValueTest, Constructor) {
  int key = 20;
  BSTNode<int> node(key);

  EXPECT_EQ(key, node.key);
  EXPECT_EQ(nullptr,  node.left);
  EXPECT_EQ(nullptr, node.right);
}

TEST(BSTNodeWithoutValueTest, ToString) {
  BSTNode<int> node(20);

  EXPECT_EQ("<K = 20>", node.toString());
}

// Testing AVLNode with value
TEST(AVLNodeWithValueTest, Constructor) {
  int key = 30;
  std::string value = "World";
  AVLNode<int, std::string> node(key, value);

  EXPECT_EQ(key, node.key);
  EXPECT_EQ(value, node.value);
  EXPECT_EQ(0, node.balanceFactor);
  EXPECT_EQ(nullptr, node.left);
  EXPECT_EQ(nullptr, node.right);
}

TEST(AVLNodeWithValueTest, ToString) {
  AVLNode<int, std::string> node(30, "World");

  EXPECT_EQ("<K = 30, V = World, BF = 0, C = 1>", node.toString());
}

TEST(AVLNodeWithValueTest, GetKeyValuePair) {
  int key = 30;
  std::string value = "World";
  AVLNode<int, std::string> node(key, value);
  std::pair<int, std::string> expected(key, value);

  EXPECT_EQ(expected, node.getKeyValuePair());
}

// Testing AVLNode without value
TEST(AVLNodeWithoutValueTest, Constructor) {
  int key = 40;
  AVLNode<int> node(key);

  EXPECT_EQ(key, node.key);
  EXPECT_EQ(node.balanceFactor, 0);
  EXPECT_EQ(nullptr, node.left);
  EXPECT_EQ(nullptr, node.right);
}

TEST(AVLNodeWithoutValueTest, ToString) {
  AVLNode<int> node(40);

  EXPECT_EQ("<K = 40, BF = 0, C = 1>", node.toString());
}

TEST(AVLNodeWithoutValueTest, AddDuplicateGetCountGetLastItem) {
  AVLNode<int> node(40);
  node.addDuplicateItem(40);
  node.addDuplicateItem(40);

  EXPECT_EQ(3, node.getCount());
  EXPECT_EQ("<K = 40, BF = 0, C = 3>", node.toString());
  EXPECT_EQ(40, node.getLastEncounteredItem());
}

TEST(AVLNodeWithValueTest, AddDuplicateGetCountGetLastItemSearch) {
  AVLNode<int, std::string> node(40, "V-1");
  node.addDuplicateItem(40, "V-2");
  node.addDuplicateItem(40, "V-3");

  EXPECT_EQ(3, node.getCount());
  EXPECT_EQ(1, node.getCount("V-1"));
  EXPECT_EQ(1, node.getCount("V-2"));
  EXPECT_EQ(1, node.getCount("V-3"));

  EXPECT_EQ("<K = 40, V = V-1, BF = 0, C = 3>", node.toString());

  std::pair<int, std::string> expectedLastItem(40, "V-3");
  EXPECT_EQ(expectedLastItem, node.getLastEncounteredItem());

  std::tuple<bool, int, std::string> expected(true, 40, "V-2");
  EXPECT_EQ(expected, node.searchItem("V-2"));
}

// Run all the tests
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
