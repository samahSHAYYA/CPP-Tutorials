/**
 * @file tree_implementation.hpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains the implementation of tree template class
 *        defined in tree.hpp methods.
 */

#ifndef TREE_IMPLEMENTATION_HPP
#define TREE_IMPLEMENTATION_HPP

#include <algorithm>        // For using std::max()
#include <fstream>          // For using std::ofstream
#include <iostream>         // For using std::cerr
#include <cstdio>           // For using std::remove (for removing file)

#include "tree/tree.hpp"

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
Tree<NT, KT, VT, balanced, ET>::Tree(bool allowDuplicateKeys) :
                                allowDuplicateKeys(allowDuplicateKeys) {
}

/**
 * REMARK:
 *
 * Below although this constructor is defined for VT being void type, you
 * should include VT as template argument as std::enable_if_t is used to enable
 * it after assuring VT is void (this is reflected by the use of:
 * template<typename> after first template statement).
 */
template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
Tree<NT, KT, VT, balanced, ET>::Tree(const KeyVector &keys,
                                     bool allowDuplicateKeys) :
                                Tree<NT, KT, void, balanced, ET>(allowDuplicateKeys) {
    insert(keys);
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
Tree<NT, KT, VT, balanced, ET>::Tree(const std::vector<std::pair<KT, VT>> &pairs,
                                     bool allowDuplicateKeys) :
                                Tree<NT, KT, VT, balanced, ET>(allowDuplicateKeys) {
    insert(pairs);
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename, typename>
Tree<NT, KT, VT, balanced, ET>::Tree(const std::string &filepath) :
                                Tree(deserialize(filepath)){
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
Tree<NT, KT, VT, balanced, ET>::Tree(const Tree &other) :
                                Tree(other.getItems(),
                                     other.isDuplicationAllowed()) {
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
Tree<NT, KT, VT, balanced, ET>& Tree<NT, KT, VT, balanced, ET>::operator=(const Tree &other) {
    // Perform the check below to avoid self copy
    if(this != &other) {
        // Clear current tree
        clear();

        // Insert all items (whether keys or key-value pairs)
        insert(other.getItems());
    }

    return *this;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
Tree<NT, KT, VT, balanced, ET>::Tree(Tree &&other) noexcept {
     Mover::moveTree(std::move(other), *this);
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
Tree<NT, KT, VT, balanced, ET>& Tree<NT, KT, VT, balanced, ET>::operator=(Tree &&other) noexcept {
    // We check for to assure no self-assignment
    if(this != &other) {
        Mover::moveTree(std::move(other), *this);
    }

    return *this;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
void Tree<NT, KT, VT, balanced, ET>::Mover::moveTree(Tree &&source, Tree &destination) noexcept {
    /**
     * Note that after using std::move, we should not attempt using moved
     * variables.
     */
    destination.root = std::move(source.root);
    destination.allowDuplicateKeys = std::move(source.allowDuplicateKeys);
    destination.count = std::move(source.count);
    destination.height = std::move(source.height);
    destination.invalidateHeight = std::move(source.invalidateHeight);
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
bool Tree<NT, KT, VT, balanced, ET>::isDuplicationAllowed() const {
    return allowDuplicateKeys;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
bool Tree<NT, KT, VT, balanced, ET>::isEmpty() const {
    return root == nullptr;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
void Tree<NT, KT, VT, balanced, ET>::clear() {
    root = nullptr;
    count = 0;
    height = 0;
    invalidateHeight = false;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
size_t Tree<NT, KT, VT, balanced, ET>::getHeight(NodePtr node) const{
    // Calling this function means fresh computation is needed.
    size_t height = 0;

    if(node) {
        auto leftHeight = getHeight(node->left);
        auto rightHeight = getHeight(node->right);
        height = 1 + (leftHeight >= rightHeight ? leftHeight : rightHeight);
    }

    if(node == root) {
        this->height = height;
        invalidateHeight = false;
    }

    return height;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
size_t Tree<NT, KT, VT, balanced, ET>::getHeight() const {
    if(invalidateHeight) {
        // Recomputing tree height.
        getHeight(root);
    }

    return height;
}

/**
 * REMARK:
 *
 * Notice the use of typename before Tree<NT, KT, VT, balanced, ET>::NodePtrPtr.
 * This is needed as NodePtrPtr is dependent type. Within class definition it
 * is not needed, but outside class (as in here) we should use `typename`.
 */
template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<bool, typename>
typename Tree<NT, KT, VT, balanced, ET>::NodePtr* Tree<NT, KT, VT, balanced, ET>::updateBalanceFactors(NodePtr &node) {
    // Setting `imbalanced` to nullptr.
    NodePtr *imbalanced = nullptr;

    if(node) {
        node->balanceFactor = getHeight(node->right);
        node->balanceFactor -= getHeight(node->left);

        if(abs(node->balanceFactor) > 1) {
            imbalanced = &node;
        }

        auto *leftImbalanced = updateBalanceFactors(node->left);
        auto  *rightImbalanced = updateBalanceFactors(node->right);

        /**
         * We consider the deepest imbalanced node (closest to the leaves).
         * In case, we have no imbalanced left and right nodes, then
         * `imbalanced` is what consider.
         *
         * In case `leftImbalanced` or `rightImbalanced` is not nullptr, then
         * we take the non-nullptr value of these as the overall imbalanced
         * node. Note `leftImbalanced` and `rightImbalanced` cannot be
         * simultaneously not null.
         */
        if(leftImbalanced || rightImbalanced) {
            imbalanced = leftImbalanced ? leftImbalanced : rightImbalanced;
        }
        // Else, we keep the value of imbalancedNode
    }

    return imbalanced;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<bool, typename>
void Tree<NT, KT, VT, balanced, ET>::rotateLeft(NodePtr &node) {
    /**
     *  Let  x be unbalanced node we have such a case:
     *                         x
     *                  T1              y
     *                            T2        z
     *                                  T3      T4
     *
     * We transform into:
     *                     y
     *              x             z
     *          T1      T2    T3      T4
     *
     * So here we have:
     *          xNode = *node
     *          yNode = (*node)->right
     *          zNode = yNode->right
     *          T1 = xNode->left
     *          T2 = yNode->left
     *          T3 = zNode->left
     *          T4 = zNode->right
     */
    auto xNode = node;

    // We set node to point to yNode
    node = xNode->right;

    // We set xNode->right to point to T2 (yNode->left)
    xNode->right = node->left;

    // We set yNode->left to point to xNode
    node->left = xNode;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<bool, typename>
void Tree<NT, KT, VT, balanced, ET>::rotateRight(NodePtr &node) {
    /**
     *  Let  x be unbalanced node we have such a case:
     *                         x
     *                  y             T1
     *             z         T2
     *        T3      T4
     *
     * We transform into:
     *                     y
     *              z             x
     *          T3      T4    T2      T1
     *
     * So here we have:
     *          xNode = *node
     *          yNode = (*node)->left
     *          zNode = yNode->left
     *          T1 = xNode->right
     *          T2 = yNode->right
     *          T3 = zNode->left
     *          T4 = zNode->right
     */
    auto xNode = node;

    // We set node to point to yNode
    node = xNode->left;

    // We set xNode->left to point to T2 (yNode->left)
    xNode->left = node->right;

    // We set yNode->right to point to xNode
    node->right = xNode;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<bool, typename>
void Tree<NT, KT, VT, balanced, ET>::balance() {
    auto *imbalancedNode = updateBalanceFactors(root);

    if(imbalancedNode) {
        if((*imbalancedNode)->balanceFactor > 1) {
            /**
             * Right-heavy tree:
             *
             * Check if right child is left-heavy. If it is the case, then a
             * right rotation should be done and followed by a left rotation.
             * Otherwise, only left rotation is needed.
             */
            if((*imbalancedNode)->right->balanceFactor < 0) {
                rotateRight((*imbalancedNode)->right);
            }

            rotateLeft(*imbalancedNode);
        }
        else {
            /**
             * Left-heavy tree:
             *
             * Check if left child is right-heavy. If it is the case, then a
             * left rotation should be done and followed by a right rotation.
             * Otherwise, only right rotation is needed.
             */
            if((*imbalancedNode)->left->balanceFactor > 0) {
                rotateLeft((*imbalancedNode)->left);
            }

            rotateRight(*imbalancedNode);
        }
    }

    // Definitely no more imbalanced nodes.
    updateBalanceFactors(root);
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
bool Tree<NT, KT, VT, balanced, ET>::insert(KT key) {
    bool inserted = false;
    bool isDuplicateKey = false;

    // Make a pointer to the root shared pointer.
    auto *current = &root;

    while(*current) {
        /**
         * Looping until we reach the predecessor node where we should add
         * the new node key and its corresponding value.
         */
        if(key == (*current)->key) {
            isDuplicateKey = true;

            if(!allowDuplicateKeys) {
                // Duplicate key and it is not allowed ==> stop.
                break;
            }
            else {
                // Duplicate keys are allowed.
                if constexpr (balanced) {
                    // Case of AVLTree.
                    inserted = (*current)->addDuplicateItem(key);
                    break;
                }
                else {
                    // Case of BSTree: go to left.
                    current = &(*current)->left;
                }
            }
        }
        else {
            // Key strictly less than or greater than.
            current = key < (*current)->key ? &(*current)->left : &(*current)->right;
        }
    }

    if(!isDuplicateKey || !balanced) {
        /**
         * Case of AVLTree with non-duplicate key or the case of BSTree.
         *
         * In such a case, we have definitely: `*current == nullptr`.
        */
        *current = std::make_shared<NodeType>(key);

        if(current) {
            inserted = true;
        }
    }

    if(inserted) {
        // Update nodes count (or more precisely items count).
        ++count;

        // We do a check if we need to set invalidateHeight.
        if constexpr (balanced) {
            // Only tree structure changes on non-duplicate key insertion.
            invalidateHeight = !isDuplicateKey;

            if(invalidateHeight) {
                // There is change in tree structure ==> balance.
                balance();
            }
            // Else no need for balance as we added duplicate-key-based item.
        }
        else {
            invalidateHeight = true;
        }
    }

    return inserted;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename T, typename>
bool Tree<NT, KT, VT, balanced, ET>::insert(KT key, T value) {
    bool inserted = false;
    bool isDuplicateKey = false;

    // Make a pointer to the root shared pointer.
    auto *current = &root;

    while(*current) {
        /**
         * Looping until we reach the predecessor node where we should add
         * the new node key and its corresponding value.
         */
        if(key == (*current)->key) {
            isDuplicateKey = true;

            if(!allowDuplicateKeys) {
                // Duplicate key and it is not allowed ==> stop.
                break;
            }
            else {
                // Duplicate keys are allowed.
                if constexpr (balanced) {
                    // Case of AVLTree.
                    inserted = (*current)->addDuplicateItem(key, value);
                    break;
                }
                else {
                    // Case of BSTree: go to left.
                    current = &(*current)->left;
                }
            }
        }
        else {
            // Key strictly less than or greater than.
            current = key < (*current)->key ? &(*current)->left : &(*current)->right;
        }
    }

    if(!isDuplicateKey || !balanced) {
        /**
         * Case of AVLTree with non-duplicate key or the case of BSTree.
         *
         * In such a case, we have definitely: `*current == nullptr`.
        */
        *current = std::make_shared<NodeType>(key, value);

        if(current) {
            inserted = true;
        }
    }

    if(inserted) {
        // Update nodes count (or more precisely items count).
        ++count;

        // We do a check if we need to set invalidateHeight.
        if constexpr (balanced) {
            // Only tree structure changes on non-duplicate key insertion.
            invalidateHeight = !isDuplicateKey;

            if(invalidateHeight) {
                // There is change in tree structure ==> balance.
                balance();
            }
            // Else no need for balance as we added duplicate-key-based item.
        }
        else {
            invalidateHeight = true;
        }
    }

    return inserted;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
size_t Tree<NT, KT, VT, balanced, ET>::insert(const InsertInputVector &items) {
    size_t count = 0;

    for(const auto &item : items) {
        if constexpr (std::is_void_v<VT>) {
            count = insert(item) ? count + 1 : count;
        }
        else {
            count = insert(item.first, item.second) ? count + 1 : count;
        }
    }

    return count;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
typename Tree<NT, KT, VT, balanced, ET>::SearchResult Tree<NT, KT, VT, balanced, ET>::search(KT key,
                                                                                             bool lastEncounter)
                                                                                             const {
    /**
     * Note that searchResult will be initialized with default values, where
     * boolean part will be false (being the default value).
     *
     * Note we return the found key as it might be different in regards to
     * non-compared data members. For instance, suppose KT consist of two
     * fields called `id` and `timestamp`, where comparison is only based on
     * `id`, then the searched for key and found one might have different
     * `timestamp`.
     */
    SearchResult searchResult;

    // Making a copy of root shared pointer.
    auto current = root;

    while(current) {
        if(current->key == key) {
            if(!allowDuplicateKeys || !lastEncounter) {
                // We stop at first encounter.
                if constexpr (std::is_void_v<VT>) {
                    searchResult = std::make_pair(true, current->key);
                }
                else {
                    searchResult = std::make_tuple(true,
                                                   current->key,
                                                   current->value);
                }

                // Nothing to do further ==> break the while loop.
                break;
            }

            /**
             * Reached here ==> duplicates are allowed and we need last
             * encountered key.
             */
            if constexpr (balanced) {
                // Get the last encountered item.
                auto lastItem = current->getLastEncounteredItem();

                if constexpr (std::is_void_v<VT>) {
                    searchResult = std::make_pair(true, lastItem);
                }
                else {
                    searchResult = std::make_tuple(true,
                                                   lastItem.first,
                                                   lastItem.second);
                }

                break;
            }
            else {
                /**
                 * It is a BSTree ==> update searchResult to current key and/or
                 * value, then if a duplicate exists it should be to the left.
                 */
                if constexpr (std::is_void_v<VT>) {
                    searchResult = std::make_pair(true, current->key);
                }
                else {
                    searchResult = std::make_tuple(true,
                                                   current->key,
                                                   current->value);
                }

                // Point current to the left subtree.
                current = current->left;
            }
        }
        else {
            // Key is strictly smaller or larger.
            current = key < current->key ? current->left : current->right;
        }
    }

    return searchResult;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename T, typename>
typename Tree<NT, KT, VT, balanced, ET>::SearchResult Tree<NT, KT, VT, balanced, ET>::search(KT key,
                                                                                             T value,
                                                                                             bool lastEncounter)
                                                                                             const {
    SearchResult searchResult;

    // Making a copy of root shared pointer.
    auto current = root;

    while(current) {
        if(current->key == key) {
            if constexpr (balanced) {
                searchResult = current->searchItem(value, lastEncounter);
                break;
            }
            else {
                // Case of BSTree.
                if(!allowDuplicateKeys || !lastEncounter) {
                    // We stop at first encounter.
                    if(current->value == value) {
                        searchResult = std::make_tuple(true,
                                                       current->key,
                                                       current->value);

                        // Nothing to do further ==> break the while loop.
                        break;
                    }
                    else {
                        /**
                         * Values do not match.
                         */
                        if(allowDuplicateKeys) {
                            // Traverse left subtree.
                            current = current->left;
                        }
                        else {
                            /**
                             * No duplicates are allowed and thus no further
                             * search is needed.
                             */
                            break;
                        }
                    }
                }
                else {
                    /**
                     * Duplicates are allowed and we need the last encountered
                     * item.
                     */
                    if(current->value == value) {
                        // Value is found.
                        searchResult = std::make_tuple(true,
                                                       current->key,
                                                       current->value);
                    }

                    /**
                     * If duplicates exist, they are in the left subtree.
                     * Point current to left to update searchResult on every
                     * encounter.
                     */
                    current = current->left;
                }
            }
        }
        else {
            // Key is strictly smaller or larger.
            current = key < current->key ? current->left : current->right;
        }
    }

    return searchResult;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
void Tree<NT, KT, VT, balanced, ET>::removeChildlessNode(NodePtr &node) {
    /**
     * As no children nodes, we can just nullify it (de-allocation is
     * handled by the smart pointer itself).
     */
    node = nullptr;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
void Tree<NT, KT, VT, balanced, ET>::removeSingleChildNode(NodePtr &node) {
    /**
     * As we have a single child (either left or right), we just make node
     * point to the existing child.
     */
    node = node->left ? node->left : node->right;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
void Tree<NT, KT, VT, balanced, ET>::removeDoubleChildNode(NodePtr &node) {
    /**
     * We let node point to the left-most node of the right subtree (or the
     * right node if this right node has no left-child).
     */
    auto itemPtr = node;          // Taking a copy of the node to delete.
    auto successor = node->right; // Taking the right child.

    while(successor->left && successor->left->left) {
        /**
         * Move one step to the left assuring we have access to left child
         * and its parent.
         */
        successor = successor->left;
    }

    /**
     * Reached here means no more left child, i.e.,:
     *      successor->left == nullptr or successor->left->left == nullptr.
     */
    if(successor->left) {
        node = successor->left;
        successor->left = successor->left->right;
        node->right = itemPtr->right;
    }
    else {
        /**
         * Meaning successor is the same as itemPtr->right (i.e. node->right).
         * Note that here we do not have to update node->right.
         */
        node = successor;
    }

    /**
     * In all cases, we need to update node->left to initial left child of
     * itemPtr.
     */
    node->left = itemPtr->left;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
void Tree<NT, KT, VT, balanced, ET>::removeNode(NodePtr &node) {
    if(node->left && node->right) {
        /**
         * Node has two children (note that in this case, we replace
         * deleted node by the left-most node of the right child (or the
         * right-child if does not have left children). This is important as it
         * guarantees that the replacement node of the deleted one has its key
         * strictly greater than the deleted node key.
         */
        removeDoubleChildNode(node);
    }
    else {
        // The node has at most has one child.
        if(node->left || node->right) {
            // Node has single child.
            removeSingleChildNode(node);
        }
        else {
            // Childless node.
            removeChildlessNode(node);
        }
    }
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
size_t Tree<NT, KT, VT, balanced, ET>::remove(KT key, bool all) {
    size_t deletedCount = 0;

    // Make a pointer to the root shared pointer.
    auto *current = &root;

    while(*current) {
        if((*current)->key == key) {
            if constexpr (balanced) {
                // Case of AVLTree.
                if(all || (*current)->duplicates.empty()) {
                    deletedCount = (*current)->getCount();

                    // Deleting the current node.
                    removeNode(*current);

                    // Updating tree nodes count.
                    count -= deletedCount;

                    /**
                     * We tree structure change and thus we should invalidate
                     * height and balance tree as needed.
                     */
                    invalidateHeight = true;
                    balance();
                }
                else {
                    /**
                     * We are required to delete one and we have duplicates.
                     * Then, we should remove the first virtual node, which
                     * is the key or key-value stored in the current node
                     * itself. The key or key-value should be replaced by the
                     * first item in duplicates.
                     */
                    if constexpr (std::is_void_v<VT>) {
                        auto item = (*current)->duplicates.front();
                        (*current)->key = item;
                    }
                    else {
                        auto item = (*current)->duplicates.front();
                        (*current)->key = item.first;
                        (*current)->value = item.second;
                    }

                    // Remove the first copied item.
                    (*current)->duplicates.pop_front();

                    ++deletedCount;

                    /**
                     * Updating tree nodes count (more precisely virtual nodes
                     * count).
                     */
                    --count;

                    /**
                     * Here no need to remove any node as it is still there.
                     * Also, as no structure change, there is no need to
                     * invalidate height and no need to call balance method.
                     */
                }

                // Nothing further to do ==> break the while loop.
                break;
            }
            else {
                // Case of BSTree.
                /**
                 * This variable would be used in case `all` is true to
                 * complete all nodes removal.
                 */
                auto *currentLeft = &(*current)->left;

                removeNode(*current);
                ++deletedCount;
                invalidateHeight = true;
                --count;

                if(!all || !allowDuplicateKeys) {
                    // Nothing to do further.
                    break;
                }
                else {
                    /**
                     * If there is any remaining duplicate, it would be in the
                     * left subtree.
                     */
                    current = currentLeft;
                }
            }
        }
        else {
            // Key is strictly smaller or larger.
            current = key < (*current)->key ? &(*current)->left : &(*current)->right;
        }
    }

    return deletedCount;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
size_t Tree<NT, KT, VT, balanced, ET>::remove(const KeyVector &keys, bool all) {
    size_t deletedCount = 0;

    for(const auto &key : keys) {
        deletedCount += remove(key, all);
    }

    return deletedCount;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename T, typename>
size_t Tree<NT, KT, VT, balanced, ET>::remove(KT key, T value, bool all) {
    size_t deletedCount = 0;

    // Make a pointer to the root shared pointer.
    auto *current = &root;

    while(*current) {
        if((*current)->key == key) {
            if constexpr (balanced) {
                // Case of AVLTree.
                if((*current)->duplicates.empty()) {
                    if((*current)->value == value) {
                        // We should delete the whole node.
                        deletedCount = (*current)->getCount();
                        removeNode(*current);

                        count -= deletedCount;
                        invalidateHeight = true;
                        balance();
                    }
                    // Else not found

                    // In all cases, we are done (just break the while loop).
                    break;
                }
                else {
                    if((*current)->value == value) {
                        /**
                         * Just remove it by replacing node key-value by the
                         * first item in duplicates.
                         */
                        auto item = (*current)->duplicates.front();
                        (*current)->key = item.first;
                        (*current)->value = item.second;

                        // Remove the front / first item in duplicates.
                        (*current)->duplicates.pop_front();

                        ++deletedCount;
                        --count;

                        /**
                         * Note as no node deletion ==> no height invalidation
                         * or balancing is needed.
                         */
                        if(!all) {
                            // We should stop.
                            break;
                        }
                        else {
                            /**
                             * We should complete removing any existing items
                             * with value equal to the passed one. We do this
                             * by continuing the execution of while loop
                             * without doing anything further.
                             */
                            continue;
                        }
                    }
                    else {
                        /**
                         * We need to search for value in the duplicates.
                         * Notice that the third block in the for loop is
                         * not used (as we cannot do ++it after deletion).
                         */
                        for(auto it = (*current)->duplicates.begin();
                            it != (*current)->duplicates.end();) {

                            if(it->second == value) {
                                it = (*current)->duplicates.erase(it);
                                ++deletedCount;
                                --count;

                                if(!all) {
                                    // Break the for loop.
                                    break;
                                }
                                /**
                                 * Else continue with the for loop until all
                                 * values equal to passed value are deleted
                                 * if present or until first occurrence (if
                                 * not all deletion is required).
                                 */
                            }
                            else {
                                // Check next item.
                                ++it;
                            }
                        }

                        // Nothing to do further.
                        break;
                    }
                    /**
                     * Note that here no height invalidation or balancing is
                     * required.
                     */
                }
            }
            else {
                // Case of BSTree.
                if((*current)->value != value && !allowDuplicateKeys) {
                    /**
                     * Nothing to be done as no key duplicates exist and
                     * therefore, the search should stop here.
                     */
                    break;
                }
                // Else either value is found or duplicate keys allowed.

                if((*current)->value != value) {
                    /**
                     * The queried value is not found but duplicates might
                     * exist. We need to traverse left subtree.
                     */
                    current = &(*current)->left;

                    /**
                     * Nothing to further do, just need to continue while
                     * loop without going any further.
                     */
                    continue;
                }

                // Reached here ==> (*current)->value == value.
                /**
                 * This variable would be used in case `all` is true to
                 * complete all nodes removal.
                 */
                auto *currentLeft = &(*current)->left;

                removeNode(*current);

                ++deletedCount;
                invalidateHeight = true;
                --count;

                if(!all || !allowDuplicateKeys) {
                    // Nothing to do further.
                    break;
                }
                else {
                    /**
                     * If there is any remaining duplicate, it would be in the
                     * left subtree.
                     */
                    current = currentLeft;
                }
            }
        }
        else {
            // Key is strictly smaller or larger.
            current = key < (*current)->key ? &(*current)->left : &(*current)->right;
        }
    }

    return deletedCount;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
size_t Tree<NT, KT, VT, balanced, ET>::remove(const std::vector<std::pair<KT, VT>> &pairs,
                                              bool all) {
    size_t deletedCount = 0;

    for(const auto &pair : pairs) {
        deletedCount += remove(pair.first, pair.second);
    }

    return deletedCount;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
size_t Tree<NT, KT, VT, balanced, ET>::getCount() const{
    return count;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
size_t Tree<NT, KT, VT, balanced, ET>::getCount(KT key) const {
    size_t count = 0;
    auto current = root;

    while(current) {
        if(current->key == key) {
            if constexpr (balanced) {
                // Case of AVLTree.
                count += current->getCount();
                break;
            }
            else{
                // Case of BSTree.
                ++count;

                if(allowDuplicateKeys) {
                    // Any duplicate if exists would be in left subtree.
                    current = current->left;
                }
                else {
                    // Duplicate keys are not allowed ==> we should stop here.
                    break;
                }
            }
        }
        else {
            // Key is strictly smaller or larger.
            current = key < current->key ? current->left : current->right;
        }
    }

    return count;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename T, typename>
size_t Tree<NT, KT, VT, balanced, ET>::getCount(KT key, T value) const {
    size_t count = 0;
    auto current = root;

    while(current) {
        if(current->key == key) {
            if constexpr (balanced) {
                // Case of AVLTree.
                count += current->getCount(value);
                break;
            }
            else {
                // Case of BSTree.
                if(current->value == value) {
                    ++count;
                }

                if(allowDuplicateKeys) {
                    // Any duplicate if exists would be in left subtree.
                    current = current->left;
                }
                else {
                    // Duplicate keys are not allowed ==> we should stop here.
                    break;
                }
            }
        }
        else {
            // Key is strictly smaller or larger.
            current = key < current->key ? current->left : current->right;
        }
    }

    return count;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
typename Tree<NT, KT, VT, balanced, ET>::LimitResult Tree<NT, KT, VT, balanced, ET>::minKey() const {
    /**
     * The bool part of minResult will be by default false indicating no
     * minimum exists (case of empty tree).
     *
     * Note LimitResult is a std::pair<bool, KT> if `VT` is void. Otherwise, it
     * is std::tuple<bool, KT, VT>.
     *
     * Note that to modify individual tuple elements, you can use:
     *
     * (I):Index-based access
     *      std::get<index>(myTuple) as for instance:
     *      auto firstIndexElement = std::get<0>(myTuple);
     *      or
     *      std::get<0>(myTuple) = newValue;
     *
     * (II): Type-based access (C++14 or later) (in case of having more than
     *       one element of the same type, it will consider first occurrence of
     *       this type). That's why for this type-access is less recommended.
     *       auto oldValue = std::get<bool>(myTuple);
     *       or
     *       std::get<bool>(myTuple) = true;
     */
    LimitResult minResult;
    auto current = root;

    while(current && current->left) {
        /**
         * As we are looking for minimum key, we need to always traverse left
         * subtree.
         */
        current = current->left;
    }

    if(current) {
        if constexpr (balanced) {
            // Case of AVLTree.
            auto lastItem = current->getLastEncounteredItem();
            if constexpr (std::is_void_v<VT>) {
                minResult = std::make_pair(true, lastItem);
            }
            else {
                minResult = std::make_tuple(true,
                                            lastItem.first,
                                            lastItem.second);
            }
        }
        else {
            // Case of BSTree.
            // We have a minimum value.
            if constexpr (std::is_void_v<VT>) {
                minResult = std::make_pair(true, current->key);
            }
            else {
                minResult = std::make_tuple(true, current->key, current->value);
            }
        }
    }

    return minResult;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
typename Tree<NT, KT, VT, balanced, ET>::LimitResult Tree<NT, KT, VT, balanced, ET>::maxKey() const {
    LimitResult maxResult;
    auto current = root;

    while(current && current->right) {
        /**
         * As we are looking for maximum key, we need to always traverse right
         * subtree.
         */
        current = current->right;
    }

    if(current) {
        /**
         * Here we do not need to do as we did in minimum in which we sought
         * last item in the current AVL Node as we always consider first key
         * value pair in the AVLNode as the maximum compared to its duplicates.
         * This is to go along with theoretical approach that we would in
         * AVLTree if duplicates were allowed to be added as individual nodes as
         * the case of BSTree. In such a case, every new duplicate will go on
         * left and not right. So imagine duplicates in AVLNode its virtual
         * additional subtree, and for max we fetch right-most node.
         */
        // We have a maximum value.
        if constexpr (std::is_void_v<VT>) {
            maxResult = std::make_pair(true, current->key);
        }
        else {
            maxResult = std::make_tuple(true, current->key, current->value);
        }
    }

    return maxResult;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
typename Tree<NT, KT, VT, balanced, ET>::NodePtrVector Tree<NT, KT, VT, balanced, ET>::getNodesAtLevel(size_t level,
                                                                                                       NodePtrVector prevNodes)
                                                                                                       const {
    /**
     * Initializing the vector knowing its size is 2^level, which is the same
     * as 1 << level.
     *
     * We assume that level is less than height, and that the tree is
     * non-empty.
     */
    NodePtrVector levelNodes(1 << level);

    if(level == 0) {
        // Ignoring prevNodes (previous level nodes).
        levelNodes[0] = root;
    }
    else {
        size_t startLevel = 1;

        if(prevNodes.size() == 0) {
            /**
             * Previous level nodes are not passed, and thus we need to start
             * from zero level as previous level by initializing prevNodes
             * with root node.
             */
            prevNodes.push_back(root);
        }
        else {
            /**
             * We have the previous level nodes passed, and we assume that the
             * size of prevNodes is equal to 2^(level - 1) (i.e.
             * (1 <<(level - 1))).
             *
             * In this case, we directly force startLevel to be level.
             */
            startLevel = level;
        }

        size_t index = 0;

        for(auto i = startLevel; i <= level; ++i) {
            size_t prevSize = prevNodes.size();

            for(const auto &prevNode : prevNodes) {
                if(prevNode) {
                    /**
                     * Then we add the children nodes of the current previous
                     * level node, namely left and right children.
                     */
                    levelNodes[index++] = prevNode->left;
                    levelNodes[index++] = prevNode->right;
                }
                else {
                    /**
                     * As prevNode is nullptr, we need to skip two places
                     * keeping them nullptr.
                     */
                    index += 2;
                }
            }
        }
    }

    return levelNodes;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
size_t Tree<NT, KT, VT, balanced, ET>::getMaxStringLength(NodePtr node) const {
    // You can use either length() or size() (both give the same result).
    size_t maxLength = node ? node->toString().length() : 0;

    if(node) {
        auto leftMaxLength = getMaxStringLength(node->left);
        auto rightMaxLength = getMaxStringLength(node->right);

        // Note that we used an initializer list as an argument to std::max.
        maxLength = std::max({maxLength, leftMaxLength, rightMaxLength});
    }

    return maxLength;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
size_t Tree<NT, KT, VT, balanced, ET>::getMaxStringLength() const {
    return getMaxStringLength(root);
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
std::pair<size_t, size_t> Tree<NT, KT, VT, balanced, ET>::getPaddingInterSpacing(size_t level)
                                                                                 const {
    // We assume non-empty tree with level <= height - 1
    std::pair<size_t, size_t> ans;  // Will be set initially to (0, 0)
    auto height = getHeight();

    if(level == height - 1) {
        // Last level
        ans.second = 1; // Interspacing needs to be set and padding kept as 0.
    }
    else {
        /**
         * Case where level is zero or level < height - 1:
         * Start by computing this level padding after getting next level
         * padding and interspacing
         */
        auto nextAns = getPaddingInterSpacing(level + 1);
        ans.first = nextAns.first + (nextAns.second - 1) / 2 + 1;

        // Computing interspacing (remember that 2^a is equal to 1 << a)
        if(level == 0) {
            // No interspacing.
            ans.second = 0;
        }
        else {
            ans.second = (1 << height) - (1 << level);
            ans.second -= 2 * ans.first + 1;
            ans.second /= ((1 << level) - 1);
        }
    }

    return ans;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
std::string Tree<NT, KT, VT, balanced, ET>::getLevelString(size_t level,
                                                           size_t maxNodeStringLength,
                                                           NodePtrVector nodes)
                                                           const {
    /**
     * We assume a non-empty tree with level < tree height. Also, we assume
     * that if nodes is passed, it is of correct size being 2^level.
     */
    if(nodes.size() == 0) {
        nodes = getNodesAtLevel(level);
    }
    // Else, we assume we have the nodes and of size being 2^level.

    // Getting padding and interspacing.
    auto spacing = getPaddingInterSpacing(level);

    // Initializing string with padding spaces.
    const char fillChar = ' '; // Fill character.
    auto str = std::string(spacing.first * maxNodeStringLength, fillChar);

    // Getting number of spaces between two nodes.
    auto interSpacesCount = spacing.second * maxNodeStringLength;

    /**
     * This `diffLen` stores the string length maxNodeStringLength between
     * `maxNodeStringLength` and actual node string representation length.
     */
    size_t diffLen;
    for(const auto &node : nodes) {
        diffLen = node ? maxNodeStringLength - node->toString().length() : 0;
        str += node ? std::string(diffLen, fillChar) + node->toString() : std::string(maxNodeStringLength, fillChar);
        str += std::string(interSpacesCount, fillChar);
    }

    // Removing trailing spaces (being not needed as do not affect display).
    size_t lastNonSpace = str.find_last_not_of(std::string(1, fillChar));

    // Note that std::string::npos signifies no-position found.
    if (lastNonSpace != std::string::npos) {
        // We erase starting lastNonSpace onwards.
        str.erase(lastNonSpace + 1);
    }

    return str;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
std::string Tree<NT, KT, VT, balanced, ET>::toString() const {
    auto count = getCount();
    auto height = getHeight();
    auto maxNodeStringLength = getMaxStringLength();

    std::string str;

    if(root) {
        str = "Tree<Size = " + std::to_string(count);
        str += ", Height = " + std::to_string(height) + ">:\n";
        NodePtrVector nodes;

        for(size_t level = 0; level < height; ++level) {
            nodes = getNodesAtLevel(level, nodes);
            str += getLevelString(level, maxNodeStringLength, nodes) + "\n";
        }
    }
    else {
        str = "Empty-Tree<Size = 0, Height = 0>";
    }

    return str;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
typename Tree<NT, KT, VT, balanced, ET>::KeyVector Tree<NT, KT, VT, balanced, ET>::getSortedKeys(NodePtr node,
                                                                                                 bool reverse)
                                                                                                 const{
    KeyVector sortedKeys;

    if(node) {
        auto leftKeys = getSortedKeys(node->left, reverse);
        auto rightKeys = getSortedKeys(node->right, reverse);

        if(reverse) {
            sortedKeys.insert(sortedKeys.end(),
                              rightKeys.begin(),
                              rightKeys.end());

            sortedKeys.push_back(node->key);

            sortedKeys.insert(sortedKeys.end(),
                              leftKeys.begin(),
                              leftKeys.end());
        }
        else {
            sortedKeys.insert(sortedKeys.end(),
                              leftKeys.begin(),
                              leftKeys.end());

            sortedKeys.push_back(node->key);

            sortedKeys.insert(sortedKeys.end(),
                              rightKeys.begin(),
                              rightKeys.end());
        }
    }

    return sortedKeys;
};

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
typename Tree<NT, KT, VT, balanced, ET>::KeyVector Tree<NT, KT, VT, balanced, ET>::getSortedKeys(bool reverse)
                                                                                                 const{
    return getSortedKeys(root, reverse);
};

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
std::vector<std::pair<KT, VT>> Tree<NT, KT, VT, balanced, ET>::getSortedKeyValuePairs(NodePtr node,
                                                                                      bool reverse)
                                                                                      const {
    std::vector<std::pair<KT, VT>> sortedVector;

    if(node) {
        auto leftVector = getSortedItems(node->left, reverse);
        auto rightVector = getSortedItems(node->left, reverse);

        if(reverse) {
            sortedVector.insert(sortedVector.end(),
                                rightVector.begin(),
                                rightVector.end());

            sortedVector.push_back(node->getKeyValuePair());

            sortedVector.insert(sortedVector.end(),
                                leftVector.begin(),
                                leftVector.end());
        }
        else {
            sortedVector.insert(sortedVector.end(),
                                leftVector.begin(),
                                leftVector.end());

            sortedVector.push_back(node->getKeyValuePair());

            sortedVector.insert(sortedVector.end(),
                                rightVector.begin(),
                                rightVector.end());
        }
    }

    return sortedVector;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename>
std::vector<std::pair<KT, VT>> Tree<NT, KT, VT, balanced, ET>::getSortedKeyValuePairs(bool reverse)
                                                                                      const {
    return getSortedKeyValuePairs(root);
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
typename Tree<NT, KT, VT, balanced, ET>::ItemVector Tree<NT, KT, VT, balanced, ET>::getItems() const {
    ItemVector items;

    if(root) {
        // Tree is non-empty.
        NodePtrVector nodes;
        auto height = getHeight();

        for(auto level = 0; level < height; ++level) {
            // Get level nodes.
            nodes = getNodesAtLevel(level, nodes);

            for(const auto &node : nodes) {
                if(node) {
                    // Case node is not nullptr.
                    if constexpr (std::is_void_v<VT>) {
                        items.push_back(node->key);
                        if constexpr (balanced) {
                            for(const auto &duplicateKey : node->duplicates) {
                                items.push_back(duplicateKey);
                            }
                        }
                    }
                    else {
                        items.push_back(node->getKeyValuePair());
                        if constexpr (balanced) {
                            for(const auto &duplicatePair : node->duplicates) {
                                items.push_back(duplicatePair);
                            }
                        }
                    }
                }
            }
        }
    }

    return items;
}

/**
 * REMARK:
 *
 * Below, we define some helper functions for serialize / deserialize.
 * To limit scope of methods to this file, we can either statically define
 * them or put them in anonymous namespace (this can also be used to limit
 * visibility of functions, variables, classes, etc.).
 *
 * Side Note:
 *
 * Note that static class / static struct does not have same significance as
 * used with variables and functions. A static class / static struct is a
 * class / struct that cannot be instantiated, and all its member methods and
 * data members are necessarily static. It serves as a namespace collecting
 * related data members and methods.
 */
namespace {
    /**
     * @brief Serializes a variable and writes it to the output file.
     *
     * @tparam T The type of the variable to serialize.
     *
     * @param outputFile The output file stream to write the serialized data
     *                   to.
     *
     * @param var The variable to be serialized.
     *
     * @return True if the serialization was successful and false otherwise.
     */
    template<typename T>
    bool serializeVariable(std::ofstream &outputFile, const T &var) {
        size_t dataSize;

        if constexpr (std::is_same_v<T, std::string> ||
                      std::is_base_of_v<std::string, T>) {
            /**
             * Note that string is not fundamental type, and we consider its
             * character data, not the string object to assure consistent
             * serialization. That's why we use var.length() or var.size() and
             * not sizeof(var).
             */
            dataSize = var.length();

            // Write the length of the string.
            outputFile.write(reinterpret_cast<const char*>(&dataSize),
                             sizeof(size_t));

            // Write the characters of the string.
            outputFile.write(var.data(), dataSize);
        }
        else {
            if constexpr (std::is_fundamental_v<T>) {
                /**
                 * It is a fundamental type (int, char,  double, etc.).
                 * Here we use sizeof(var) to get byte size of var.
                 */
                dataSize = sizeof(var);

                // Write the size of var
                outputFile.write(reinterpret_cast<const char*>(&dataSize),
                                sizeof(size_t));

                // Write the binary representation of var
                outputFile.write(reinterpret_cast<const char*>(&var),
                                dataSize);
            }
            else {
                // T has serialize method.
                var.serialize(outputFile);
            }
        }

        return outputFile.good();
    }

    /**
     * @brief Deserializes data from a file and writes it to a variable.
     *
     * @tparam T The type of the variable to deserialize to.
     *
     * @param inputFile The input file stream to read data from.
     *
     * @param var The variable to deserialize to.
     *
     * @return True if the deserialization was successful and false otherwise.
     */
    template<typename T>
    bool deserializeVariable(std::ifstream &inputFile, T &var) {
        size_t dataSize;

        if constexpr (std::is_same_v<T, std::string> ||
                      std::is_base_of_v<std::string, T>) {
            // Read the length of the string.
            inputFile.read(reinterpret_cast<char*>(&dataSize),
                           sizeof(size_t));

            // Resizing string var to accommodate the serialized data.
            var.resize(dataSize);

            /**
             * Read the characters into the string 9using address of var[0].
             */
            inputFile.read(&var[0], dataSize);
        }
        else {
            if constexpr (std::is_fundamental_v<T>) {
                // Reading the size of data.
                inputFile.read(reinterpret_cast<char*>(&dataSize),
                               sizeof(size_t));

                // Read the binary representation of var.
                inputFile.read(reinterpret_cast<char*>(&var),
                               dataSize);
            }
            else {
                // T has static deserialize method.
                var = T::deserialize(inputFile);
            }
        }

        return inputFile.good();
    }
};

/**
 * REMARK:
 *
 * In this serialization method, we assume that when deserializing that we know
 * all template parameters NT, KT and VT. That's why we are not saving template
 * arguments as to keep things simpler. However, a complete serialization
 * would consider template arguments saving as well. In such case,
 * deserialization will require having a type registry.
 */
template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename, typename>
bool Tree<NT, KT, VT, balanced, ET>::serialize(const std::string &filepath,
                                               bool deleteOnFailure)
                                               const {
    // This is the method return value.
    bool serialized = false;

    // This is used to signal writing failure
    bool writeFailure = false;

    // Creating a binary file output stream
    std::ofstream outputFile(filepath, std::ios::binary);

    if(outputFile) {
        /**
         * Successful creation of file output stream:
         * We start by saving allowDuplicateKeys value.
         */
        writeFailure = !serializeVariable<bool>(outputFile, allowDuplicateKeys);

        if(!writeFailure) {
            // Getting all key-value pairs in the tree
            auto items = getItems();
            for(const auto &item : items) {
                /**
                 * We cannot serialize the pair directly because sizeof(pair)
                 * does not account for the sizes of pair.first and pair.second.
                 * For nested types, such as pair, we need to do serialization
                 * as below.
                 *
                 * Here, we assume that KeyType and ValueType are fundamental
                 * or string types.
                 */

                // Serializing key value
                if constexpr (std::is_void_v<VT>) {
                    writeFailure = !serializeVariable<KT>(outputFile, item);
                }
                else {
                    writeFailure = !serializeVariable<KT>(outputFile,
                                                          item.first);
                }

                if(writeFailure) {
                    // Error write ==> stop
                    break;
                }

                /**
                 * Considering the pair value if VT is not void type.
                 * Note that std::is_void_v<VT> is shorthand for:
                 * std::is_void<VT>::value.
                 */
                if constexpr (!std::is_void_v<VT>) {
                    // We have a non-void VT type
                    writeFailure = !serializeVariable<VT>(outputFile,
                                                          item.second);
                }
                // Else we do not have a value to serialize

                if(writeFailure) {
                    break;
                }
            }
        }

        // Closing stream
        outputFile.close();

        if(writeFailure) {
            std::string error = "Fatal error occurred while writing to ";
            error += "the file: " + filepath + " !\n";
            std::cerr << error << std::endl;

            if(deleteOnFailure) {
                std::string error = "Deleting partial / inconsistent file: ";
                error += filepath + " ...\n";
                std::cerr << error << std::endl;

                // Deletion
                if(std::remove(filepath.c_str()) == 0) {
                    // Successful deletion
                    std::string info = "Deleted partial / inconsistent file: ";
                    info += filepath + " !\n";
                    std::cout << info << std::endl;
                }
                else {
                    // Failed deletion
                    error = "Failed to delete the partial / inconsistent ";
                    error += "file: " + filepath + " !\n";
                    std::cerr << error << std::endl;
                }
            }
        }
        else {
            // All data was written successfully.
            serialized = true;
        }
    }
    else {
        // Failed to open the file
        std::string error = "Failed to open the file: " + filepath + " for ";
        error += "writing!\n";
        std::cerr << error << std::endl;
    }

    return serialized;
}

template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT,
         bool balanced,
         typename ET>
template<typename, typename, typename>
Tree<NT, KT, VT, balanced, ET> Tree<NT, KT, VT, balanced, ET>::deserialize(const std::string &filepath) {
    Tree<NT, KT, VT, balanced, ET> tree;

    // Creating a binary file input stream
    std::ifstream inputFile(filepath, std::ios::binary);

    if(inputFile) {
        bool failedRead = false;

        // Reading allowDuplicateKeys
        bool allowDuplicateKeys;
        failedRead = !deserializeVariable<bool>(inputFile, allowDuplicateKeys);

        // Reading items
        typename Tree<NT, KT, VT, balanced, ET>::ItemVector items;
        KT key;

        /**
         * As we are not sure VT is void or not, we use shared pointer instead
         * which still works when VT is void.
         */
        std::shared_ptr<VT> valuePtr = nullptr;

        while(!failedRead) {
            // Reading key size
            if(!deserializeVariable<KT>(inputFile, key)) {
                /**
                 * Only here failure to read is fine meaning end of tree pairs.
                 * Thus, we break the while loop without setting failedRead to
                 * true.
                 */
                break;
            }

            // Reading the value if VT is not void type
            if constexpr (!std::is_void_v<VT>) {
                // Reading pair value
                valuePtr = std::make_shared<VT>();
                failedRead = !deserializeVariable<VT>(inputFile, *valuePtr);
            }

            if(failedRead) {
                // Failure reading the file ==> stop
                break;
            }

            /**
             * All item elements read and thus add the item to items.
             * Note that we used the emplace_back to directly create the item
             * in-place at the end of vector avoiding unneeded copy, which
             * is recommended.
             *
             * Nevertheless, with C++11 and later,
             * items.push_back(std::make_pair(key, value)) will effectively
             * yield the same result,  as std::make_pair creates an rvalue that
             * is moved by push_back method.
             */
            if constexpr (std::is_void_v<VT>) {
                items.emplace_back(key);
            }
            else {
                items.emplace_back(key, *valuePtr);
            }
        }

        // Closing input file
        inputFile.close();

        /**
         * In case we have failedRead, then we have corrupted file.
         */
        if(failedRead) {
            // Corrupted file
            std::string error = "The file " + filepath + " is corrupted!\n";
            std::cerr << error << std::endl;
        }
        else {
            // Successful file reading
            tree = Tree(items, allowDuplicateKeys);
        }
    }
    else {
        // Failed to open the file
        std::string error = "Failed to open the file: " + filepath + " for ";
        error += "reading!\n";
        std::cerr << error << std::endl;
    }

    return tree;
}

#endif