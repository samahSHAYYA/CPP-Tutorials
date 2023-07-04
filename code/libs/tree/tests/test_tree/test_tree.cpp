/**
 * @file test_tree.cpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains all tests related to the Tree template class
 *        defined in tree.hpp file.
 */

#include<unordered_map>     // For using std::unordered_map (hash-table)

#include <gtest/gtest.h>

#include "tree/tree.hpp"

// Common data series for key-only key-value pairs based nodes.
std::vector<int> keys{-5, 10, 7, -2, 0, -8, -5, 6, -4, 1};
std::vector<std::string> values{
    "Mango",            // -5
    "Strawberry",       // 10
    "Grapes",           // 7
    "Apple",            // -2
    "Banana",           // 0
    "Peach",            // -8
    "Pineapple",        // -5
    "Kiwi",             // 6
    "Orange",           // -4
    "Watermelon"        // 1
};

// Corresponding heights upon insertion.
std::vector<size_t> bstHeights{1, 2, 3, 4, 5, 5, 5, 6, 6, 7};
std::vector<size_t> bstHeightsNoDuplicates{1, 2, 3, 4, 5, 5, 5, 6, 6, 7};

std::vector<size_t> avlHeights{1, 2, 2, 3, 3, 3, 3, 4, 4, 4};
std::vector<size_t> avlHeightsNoDuplicates{1, 2, 2, 3, 3, 3, 3, 3, 4, 4};

// Define the types for key-only and key-value-pair scenarios.
using TreeTypes = testing::Types<BSTree<int>,
                                 AVLTree<int>,
                                 BSTree<int, std::string>,
                                 AVLTree<int, std::string>>;

/**
 * Helper function to create a tree with keys only or key-value pairs. Note
 * that Typed Tests in GTest take only one template argument.
 */
template<typename TreeType>
void testInsertion(TreeType &tree, std::unordered_map<int, size_t> &keyCounts) {
    size_t count = 0;
    constexpr bool isAVL = (std::is_same_v<TreeType, AVLTree<int>> ||
                            std::is_same_v<TreeType, AVLTree<int, std::string>>);

    constexpr bool isVTVoid = (std::is_same_v<TreeType, BSTree<int>> ||
                               std::is_same_v<TreeType, AVLTree<int>>);

    size_t height;
    bool inserted;

    /**
     * Remember that TreeType::SearchResult is dependent type and thus should
     * be preceded by `typename` keyword.
     */
    typename TreeType::SearchResult searchResult;
    int key; // We will always have int key unlike value which might not exist

    for (size_t index = 0; index < keys.size(); ++index) {
        key = keys[index];

        if(keyCounts[key] > 0 && !tree.isDuplicationAllowed()) {
            if constexpr (isVTVoid) {
                inserted = tree.insert(key);
            }
            else {
                inserted = tree.insert(key, values[index]);
            }

            EXPECT_FALSE(inserted);
            EXPECT_EQ(count, tree.getCount());
            EXPECT_EQ(keyCounts[key], tree.getCount(key));

            /**
             * Below, we would had used ternary operator `?:`, i.e.,
             * `height = isAVL ? ... : ...`. Nevertheless, as `isAVL` is
             * constexpr determined at compile-time, we used if constexpr ...
             * so that replacement is done at compile-time.
             */
            if constexpr (isAVL) {
                height = avlHeightsNoDuplicates[index];
            }
            else {
                height = bstHeightsNoDuplicates[index];
            }

            EXPECT_EQ(height, tree.getHeight());
        }
        else {
            // Duplicates allowed or first time insertion.
            if(keyCounts[key]) {
                // Already exists.
                searchResult = tree.search(key);
                EXPECT_TRUE(std::get<0>(searchResult));
                EXPECT_EQ(key, std::get<1>(searchResult));

                if constexpr (!isVTVoid) {
                    /**
                     * Here the duplicate key has a value different than
                     * that that of existing key-value pair.
                     */
                    searchResult = tree.search(key, values[index]);
                    EXPECT_FALSE(std::get<0>(searchResult));
                }
            }
            else {
                // First time encounter.
                searchResult = tree.search(key);
                EXPECT_FALSE(std::get<0>(searchResult));
            }

            if constexpr (isVTVoid) {
                inserted = tree.insert(key);
            }
            else {
                inserted = tree.insert(key, values[index]);
            }

            std::cout << tree.toString() << std::endl;

            EXPECT_TRUE(inserted);
            EXPECT_EQ(++count, tree.getCount());
            EXPECT_EQ(++keyCounts[key], tree.getCount(key));

            if(tree.isDuplicationAllowed()) {
                if constexpr (isAVL) {
                    height = avlHeights[index];
                }
                else {
                    height = bstHeights[index];
                }
            }
            else {
                if constexpr (isAVL) {
                    height = avlHeightsNoDuplicates[index];
                }
                else {
                    height = bstHeightsNoDuplicates[index];
                }
            }

            EXPECT_EQ(height, tree.getHeight());

            searchResult = tree.search(key, true);
            EXPECT_TRUE(std::get<0>(searchResult));
            EXPECT_EQ(key, std::get<1>(searchResult));

            /**
             * Below we do not test on AVL tree as due to balancing node
             * location might not preserve temporal order.
             */
            if constexpr (!isVTVoid && !isAVL) {
                EXPECT_EQ(values[index], std::get<2>(searchResult));
            }
        }
    }

    /**
     * Testing search with key and key-value and considering lastEncounter
     * where duplicate keys are allowed.
     *
     * We limit our test to Binary Search Tree and do not include AVL Tree,
     * as in AVL Tree and due to balancing, lastEncounter search might
     * lead to different result (due to rotations).
     */
    if(tree.isDuplicationAllowed() && !isAVL) {
        if constexpr (isVTVoid) {
            searchResult = tree.search(-5, true);

            EXPECT_TRUE(std::get<0>(searchResult));
            EXPECT_EQ(-5, std::get<1>(searchResult));
        }
        else {
            searchResult = tree.search(-5, true);

            EXPECT_TRUE(std::get<0>(searchResult));
            EXPECT_EQ(-5, std::get<1>(searchResult));
            EXPECT_EQ("Pineapple", std::get<2>(searchResult));
        }
    }
}

/**
 * Add more common tests related to removal, serialization, etc.
 * These are kept as exercise for the learner.
 */

// Define the typed test suite for key-only and key-value-pair scenarios.
template <typename TreeType>
class TreeTest : public testing::Test {
    public:
        TreeType tree;
        std::unordered_map<int, size_t> keyCounts;
};

// Register the test case.
TYPED_TEST_SUITE_P(TreeTest);

TYPED_TEST_P(TreeTest, InsertionTest) {
    /**
     * Note that tree and keyCounts are members of the current class.
     * Strange that it will not compile if we remove `this` pointer.
     */
    testInsertion<TypeParam>(this->tree, this->keyCounts);
}

// Instantiate the typed test case for all specified types.
REGISTER_TYPED_TEST_SUITE_P(TreeTest, InsertionTest);

// Instantiate the typed test case for all specified types.
INSTANTIATE_TYPED_TEST_SUITE_P(TreeTestInstantiation, TreeTest, TreeTypes);

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}