/**
 * @file tree.hpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains the data structures and classes associated with
 *        binary trees.
 */

#ifndef TREE_HPP
#define TREE_HPP

#include <memory>    // For using smart pointers, specifically std::shared_ptr
#include <string>    // For using std::string
#include <utility>   // For using std::pair and std::tuple
#include <type_traits>  // For using with partial specialization
#include <vector>       // For using std::vector

#include "tree/node.hpp"  // For using BSTNode and AVLNode

/**
 * REMARK:
 *
 * While in binary search tree, duplicates can be allowed by having the
 * rule (<= on left and > on right). However, this is not possible directly in
 * an AVL tree as due to balancing and rotations, we might have the rules above
 * violated (to clarify that consider inserting the following keys as mentioned:
 * -5, 10, 7, -2, 0, -8, -5, then upon inserting the final element -5 and
 * rebalancing you will have if applied balancing rotations -5 on the right
 * of the first -5 node, which violates <= being on left and > being on right).
 *
 * To allow for duplication in AVL tree, we can add `count` field in the `Node`
 * but this works if the key type considers in comparison all its data fields.
 * Also, this would not be sufficient if we have key-value `Node`. The most
 * generic way to allow duplicates in case of AVL tree, is to let the AVLNode
 * have additional list field called `duplicates` that stores duplicate items.
 * We used `list` not `vector` to avoid the contiguous memory need in case of
 * `vector`and make it more memory efficient.
 *
 * Finally, note that the approach we used for duplicate keys in case of
 * AVLTree, can be used also for BSTree. However, we chose not to adopt it
 * with BSTree, although it might be more memory efficient and performant
 * for learning purposes, so that the learner can compare and contrast
 * and analyze pros and cons per each approach.
 */

/**
 * @class Tree
 *
 * @brief Template class representing a tree data structure.
 *
 * @tparam NT The template class representing the node type of the tree.
 *
 * @tparam KT The type of the keys stored in the tree nodes.
 *
 * @tparam VT The type of the values stored in the tree nodes (it is
 *         optional and its default is void).
 *
 * @tparam ET It is EnableType allowing to enable / disable template based on
 *            on std::enable_if_t condition.
 *
 * @example Usage:
 * @code
 *   // Declare a tree with AVLNode as the node type, int as the key type, and
 *   // string as the value type (notice that ET - EnableType is automatically
 *   // deduced).
 *   Tree<AVLNode, int, std::string> myTree;
 *
 *   // Perform operations on the tree
 *   myTree.insert(5, "apple");
 *   myTree.insert(3, "banana");
 *   myTree.insert(7, "orange");
 *   myTree.remove(3);
 *   myTree.clear();
 *   :
 * @code
 *
 * @attention The Tree class template is final as it completely provides needed
 *            functionality for binary search and AVL trees. Other tree types
 *            such as Red-Black trees should have separate implementation.
 *
 * @note As here we are limiting usage of tree with Node defined in node.hpp,
 *       we could have simplified the template class below as:
 *
 *       template<typename KT,
 *                typename VT,
 *                bool balanced = false,
 *                typename ET = std::enable_if_t<IsComparable<KT>>>
 *       class Tree final {
 *          public:
 *               // Defining aliases
 *               using TNode = Node<KT, VT, balanced>;
 *               using TNodePtr = std::shared_ptr<TNode>;
 *               using TNodePtrPtr = std::shared_ptrTNode>;
 *               using TNodePtrVector = std::vector<TNodePtr>;
 *               using KeyVector = std::vector<KT>;
 *               :
 *               // Do needed changes in declarations and implementation
 *       };
 *
 *       This simplifies things. Nevertheless, It is kept it like that here for
 *       learning purposes (specifically, to emphasize the use of templated
 *       class within another template class)..
 *
 * @attention We used the
 *            "template<typename, typename, bool> class NT" without considering
 *            NT's EnableType (ET) as it is automatically deduced.
 *
 *            We check that KT and VT (if not void) are default constructible
 *            (has empty constructor) to be able to use them in std::pair
 *            and std::tuple in search, minKey and maxKey.
 */
template<template<typename, typename, bool> class NT,
         typename KT,
         typename VT = void,
         bool balanced = false,
         typename ET = std::enable_if_t<IsComparable<KT> &&
                                        std::is_default_constructible_v<KT> &&
                                        (std::is_void_v<VT> ||
                                         std::is_default_constructible_v<VT>)>>
class Tree final {

    public:
        /**
         * Defining aliases: Note that the 4th EnableType for NT is not passed
         * As it will be automatically replaced based on KT type test.
         */
        using NodeType = NT<KT, VT, balanced>;
        using NodePtr = std::shared_ptr<NodeType>;
        using NodePtrVector = std::vector<NodePtr>;
        using KeyVector = std::vector<KT>;

        using InsertInputVector = std::conditional_t<std::is_void_v<VT>,
                                                     KeyVector,
                                                     std::vector<std::pair<KT, VT>>>;

        using SearchResult = std::conditional_t<std::is_void_v<VT>,
                                                std::pair<bool, KT>,
                                                std::tuple<bool, KT, VT>>;

        using LimitResult = SearchResult;

        using ItemVector = std::conditional_t<std::is_void_v<VT>,
                                              KeyVector,
                                              std::vector<std::pair<KT, VT>>>;


        /**
         * @brief Constructs a new Tree object.
         *
         * @param allowDuplicateKeys Flag indicating whether duplicate keys are
         *                           allowed in the tree (default: true).
         */
        Tree(bool allowDuplicateKeys = true);

        /**
         * @brief Constructs a new Tree object initialized with the given
         *        key-value pairs.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param pairs A vector of key-value pairs to initialize the tree.
         *
         * @param allowDuplicateKeys Flag indicating whether duplicate keys are
         *                           allowed in the tree (default: true).
         *
         * @attention This is the way, we enable / disable a member method
         *            or constructor. Notice that in the unnamed enable
         *            type, we checked to assure T is void and same as VT.
         *            This is needed to prevent intentional usage this
         *            constructor with T = void when VT is not void, as we
         *            cannot directly use  VT. If we want to always check VT
         *            and ignore T, we can define the following dummy trait:
         *            template<typename T>
         *            constexpr bool dummyTrue = true;
         *
         *            In std::enable_if_t condition we can use it with &&
         *            dummyTrue<T>. Nevertheless, any attempt to pass T other
         *            than VT, will not generate a compilation error and
         *            correct behaviour is imposed.
         *
         *            This means if VT is not void, and you try:
         *            Tree<void>(keys, ..). It will yield compilation error
         *            as VT is not void. Note that the std::enable_if_t
         *            expression or call should be dependent on T, otherwise we
         *            get compile error. This dependency is required to apply
         *            SFINAE (Substitution Failure Is Not An Error).
         *
         *            Note that normal use is: Tree(keys, ...), in which T will
         *            be set by default to VT, which what we always want.
         */
        template<typename T = VT,
                 typename = std::enable_if_t<std::is_void_v<T> &&
                                             std::is_same_v<T, VT>>>
        Tree(const KeyVector &keys, bool allowDuplicateKeys = true);

        /**
         * @brief Constructs a new Tree object initialized with the given
         *        key-value pairs.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param pairs A vector of key-value pairs to initialize the tree.
         *
         * @param allowDuplicateKeys Flag indicating whether duplicate keys are
         *                           allowed in the tree (default: true).
         */
        template<typename T = VT,
                 typename = std::enable_if_t<!std::is_void_v<T> &&
                                             std::is_same_v<T, VT>>>
        Tree(const std::vector<std::pair<KT, VT>> &pairs,
             bool allowDuplicateKeys = true);

        /**
         * @brief Constructs a new Tree object and loads the tree data from a
         *        file at the specified filepath.
         *
         * @tparam T (default: KT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @tparam U (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param filepath The path to the file containing the tree data.
         *
         * @note The file should contain a serialized representation of the
         *       tree.
         *
         *       It is available in case KT and VT are both serializable.
         */
        template<typename T = KT,
                 typename U = VT,
                 typename = std::enable_if_t<AreSerializable<T, U> &&
                                             std::is_same_v<T, KT> &&
                                             std::is_same_v<U, VT>>>
        Tree(const std::string &filepath);

        /**
         * @brief Copy constructor: Constructs a new `Tree` by copying the
         *        contents of another `Tree`.
         *
         * @param other The `Tree` object to be copied.
         */
        Tree(const Tree &other);

        /**
         * @brief Copy assignment operator: Assigns the contents of another
         *        `Tree` to this `Tree`.
         *
         * @param other The `Tree` object to be copied.
         *
         * @return A reference to the modified `Tree` object.
         */
        Tree& operator=(const Tree &other);

        /**
         * @brief Move constructor: Moves the contents of another `Tree` into
         *        this `Tree`.
         *
         * @param other The `Tree` object to be moved.
         *
         * @note Notice the noexcept that is required for move constructor.
         */
        Tree(Tree &&other) noexcept;

        /**
         * @brief Move assignment operator: Moves the contents of another Tree
         *        into this Tree by transferring ownership of its resources.
         *
         * @param other The Tree object to be moved.
         *
         * @return Reference to the moved-to Tree object.
         *
         * @note The move assignment operator allows efficient transfer of
         *       resources  from one Tree to another without unnecessary
         *       copying. The move is performed by transferring ownership of
         *       the underlying data, such as the root node, from other to
         *       this. The move assignment operator requires the noexcept
         *       specifier to indicate that it does not throw any exceptions.
         */
        Tree& operator=(Tree &&other) noexcept;

        /**
         * @brief Checks if duplicate keys are allowed in the tree.
         *
         * @return True if duplicate keys are allowed and false otherwise.
         *
         * @note The function isDuplicationAllowed() returns the current status
         *       of whether duplicate keys are allowed in the tree. It is a
         *       const member method as it does not modify the state of the
         *       tree. That's why const specifier is added (if declared
         *       const Tree<...> instance; this method can still be called).
         */
        bool isDuplicationAllowed() const;

        /**
         * @brief Checks if the tree is empty.
         *
         * @return True if the tree is empty and false otherwise.
         */
        bool isEmpty() const;

        /**
         * @brief Clears the contents of the tree, removing all nodes.
         */
        void clear();

        /**
         * @brief Inserts a new key into the tree.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param key The key to be inserted.
         *
         * @return True if the key was successfully inserted, and false if the
         *         key already exists in the tree and duplication is not
         *         allowed or if failed to insert due memory issues.
         */
        template<typename T = VT,
                 typename = std::enable_if_t<std::is_void_v<T> &&
                                             std::is_same_v<T, VT>>>
        bool insert(KT key);

        /**
         * @brief Inserts a new key-value pair into the tree.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param key The key to be inserted.
         *
         * @param value The value associated with the key.
         *
         * @return True if the key-value pair was successfully inserted, and
         *         false if the key already exists in the tree and duplication
         *         is not allowed or if failed to insert due memory issues.
         */
        template<typename T = VT,
                 typename = std::enable_if_t<!std::is_void_v<T> &&
                                              std::is_same_v<T, VT>>>
        bool insert(KT key, T value);

        /**
         * @brief Inserts a vector of keys (case VT is void) or key-value pairs
         *        into the tree (case of VT being non-void type).
         *
         * @param items The keys or key-value pairs vector to be inserted.
         *
         * @return The number of successfully inserted keys or key-value pairs.
         */
        size_t insert(const InsertInputVector &items);

        /**
        * @brief Searches for a key in the tree and returns the search result
        *        which is pair of boolean and key (if VT is void), or
        *        tuple of boolean, key and value (if VT is non-void). Note
        *        that we return key as well as key might have non-comparable
        *        data members which might be different.
        *
        * @param key The key to search for.
        *
        * @param lastEncounter Optional parameter that determines the search
        *                      behavior in case of duplicate keys. If set to
        *                      true, it returns the last encountered value for
        *                      duplicate keys. If set to false (default), it
        *                      returns the first encountered value for
        *                      duplicate keys.
        *
        * @return A SearchResult, namely std::pair<bool, KT> if VT is void,
        *         and std::tuple<bool, KT, VT> otherwise.
        *
        *         If the key is not found, the boolean value will be false, and
        *         all other values will be default valued.
        *
        * @note In case duplicate keys are not allowed, lastEncounter is
        *       useless.
        *
        * @attention Here it is required to have VT default constructible,
        *            meaning with default empty constructor, as we are
        *            returning an std::pair<bool, VT>, and which should have
        *            default value in case key is not found.
        */
        SearchResult search(KT key, bool lastEncounter = false) const;

        /**
         * @brief Searches for a key-value pair in the tree.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param key The key to search for.
         *
         * @param value The value to search for.
         *
         * @return A SearchResult, namely std::tuple<bool, KT, VT>, with the
         *         bool indicating if key and value are found, and the exact
         *         key and value captured.
         *
         *         It is enabled only if VT has equality operator, and which
         *         means being non-void in the first place.
         *
         * @attention Here, we can directly set return to
         *            `std::tuple<bool, KT, VT>` as we know that VT is non-void.
         *           However, we kept it like that for simplification purposes
         *           and to keep it consistent with the key-based search.
         */
        template<typename T = VT,
                 typename = std::enable_if_t<HasEqualityOperator_v<T> &&
                                             std::is_same_v<T, VT>>>
        SearchResult search(KT key,
                            T value,
                            bool lastEncounter = false)
                            const;

        /**
         * @brief Removes the nodes with the specified key from the tree.
         *
         * @param key The key of the nodes to be removed.
         *
         * @param all Flag indicating whether all nodes with the specified key
         *            should be removed (true) or only the first encountered
         *            node (false, default value).
         *
         * @return The number of nodes removed from the tree.
         *
         * @note In case of tree where duplicate keys are not allowed, the all
         *       argument becomes useless.
         */
        size_t remove(KT key, bool all = false);

        /**
         * @brief Removes the nodes with the specified keys from the tree.
         *
         * @param keys The keys of the nodes to be removed.
         *
         * @param all Flag indicating whether all nodes with the specified keys
         *            should be removed (true) or only the first encountered
         *            node per each key (false, default value).
         *
         * @return The number of nodes removed from the tree.
         *
         * @note In case of tree where duplicate keys are not allowed, the
         *       `all` argument becomes useless.
         */
        size_t remove(const KeyVector &keys, bool all = false);

        /**
         * @brief Removes nodes with the specified key-value pair from the tree.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param key The key of the nodes to be removed.
         *
         * @param value The value of the nodes to be removed.
         *
         * @param all Flag indicating whether all nodes with the specified
         *            key-value pair should be removed (true) or only the first
         *            encountered node (false, default value).
         *
         * @return The number of nodes removed from the tree.
         *
         * @note In case of tree where duplicate keys are not allowed, the
         *       `all` argument becomes useless.
         *
         *       It is enabled only if VT has operator== (which means
         *       implicitly being non-void type).
         */
        template<typename T = VT,
                 typename = std::enable_if_t<HasEqualityOperator_v<T> &&
                                             std::is_same_v<T, VT>>>
        size_t remove(KT key, T value, bool all = false);

        /**
         * @brief Removes nodes with the specified key-value pairs from the
         *        tree.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param pairs The key-value pairs of the nodes to be removed.
         *
         * @param all Flag indicating whether all nodes with the specified
         *            key-value pairs should be removed (true) or only the
         *            first encountered node per key-value pair(false,
         *            default value).
         *
         * @return The number of nodes removed from the tree.
         *
         * @note In case of tree where duplicate keys are not allowed, the
         *       `all` argument becomes useless.
         *
         *       It is enabled only if VT is non-void.
         */
        template<typename T = VT,
                 typename = std::enable_if_t<HasEqualityOperator_v<T> &&
                                             std::is_same_v<T, VT>>>
        size_t remove(const std::vector<std::pair<KT, VT>> &pairs,
                      bool all = false);

        /**
         * @brief Gets the total number of nodes in the tree.
         *
         * @return The total number of nodes in the tree.
         */
        size_t getCount() const;

        /**
         * @brief Gets the number of nodes with the specified key in the tree.
         *
         * @param key The key to count nodes for.
         *
         * @return The number of nodes with the specified key in the tree.
         */
        size_t getCount(KT key) const;

        /**
         * @brief Gets the number of nodes with the specified key and value in
         *        the tree.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param key The key to count nodes for.
         *
         * @param value The value to count nodes for.
         *
         * @return The number of nodes with the specified key and value in the
         *         tree.
         */
        template<typename T = VT,
                 typename = std::enable_if_t<HasEqualityOperator_v<T> &&
                                             std::is_same_v<T, VT>>>
        size_t getCount(KT key, T value) const;

        /**
         * @brief Returns the height of the tree. It is the maximum number of
         *        levels from the root node to any leaf node. If the tree is
         *        empty, the height is considered to be 0.
         *
         * @return The height of the tree.
         *
         * @attention In case of non-const Tree instance, calling getHeight
         *            will call this non-const method.
         */
        size_t getHeight() const;

        /**
         * @brief In case of `VT` being void type, it returns a pair of boolean
         *        and minimum key value (if non-empty tree), which is in the
         *        left-most leaf node. Otherwise, it returns a tuple of
         *        boolean, the minimum key and its corresponding value.
         *
         * @return An std::pair or a three-element tuple.
         */
        LimitResult minKey() const;

       /**
         * @brief In case of `VT` being void type, it returns a pair of boolean
         *        and maximum key value (if non-empty tree), which is in the
         *        left-most leaf node. Otherwise, it returns a tuple of
         *        boolean, the maximum key and its corresponding value.
         *
         * @return An std::pair or a three-element tuple.
         */
        LimitResult maxKey() const;

        /**
         * @brief Returns a vector of keys in sorted order.
         *
         * @param reverse (optional) Set to `true` to retrieve the keys in
         *                 descending order. Default is `false` (i.e., ascending
         *                 order).
         *
         * @return A vector of keys in the specified sort order.
         */
        KeyVector getSortedKeys(bool reverse = false) const;

        /**
         * @brief Returns a vector of key-value pairs in sorted order.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param reverse (optional) Set to `true` to retrieve the pairs in
         *                 descending order. Default is `false` (i.e., ascending
         *                 order).
         *
         * @return A vector of key-value pairs in the specified sort order.
         */
        template<typename T = VT,
                 typename = std::enable_if_t<!std::is_void_v<T> &&
                                             std::is_same_v<T, VT>>>
        std::vector<std::pair<KT, VT>> getSortedKeyValuePairs(bool reverse = false)
                                                              const;

        /**
         * @brief Returns a vector of all items in the tree that allows exact
         *        tree recreation. In case `VT` is void type, the return will
         *        be vector of keys. Otherwise, it will be a vector of
         *        key-value pairs.
         *
         * @return A vector of keys or key-value pairs.
         */
        ItemVector getItems() const;

        /**
         * @brief Serializes the tree and saves it to a binary file.
         *
         * @tparam T (default: KT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @tparam U (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param filepath The path of the file where the serialized tree
         *                 should be saved.
         *
         * @param deleteOnFailure (optional) Set to `true` to delete the file
         *                        if serialization fails. Default is `false`.
         *
         * @return True if serialization and file writing were successful,
         *         and false otherwise.
         */
        template<typename T = KT,
                 typename U = VT,
                 typename = std::enable_if_t<AreSerializable<T, U> &&
                                             std::is_same_v<T, KT> &&
                                             std::is_same_v<U, VT>>>
        bool serialize(const std::string &filepath,
                       bool deleteOnFailure = false)
                       const;

        /**
         * @brief Deserializes a tree from a binary file.
         *
         * @tparam T (default: KT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @tparam U (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param filepath The path of the file containing the serialized tree.
         *
         * @return An instance of the `Tree` class, reconstructed from the
         *         serialized contents of the file.
         */
        template<typename T = KT,
                 typename U = VT,
                 typename = std::enable_if_t<AreSerializable<T, U> &&
                                             std::is_same_v<T, KT> &&
                                             std::is_same_v<U, VT>>>
        static Tree deserialize(const std::string &filepath);

        /**
         * @brief Returns string representation of the tree.
         *
         * @tparam T (default: KT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @return String representation of the tree.
         *
         * @note This will be present if at least KT is convertible to string.
         */
        template<typename T = KT,
                 typename = std::enable_if_t<IsConvertibleToString<T> &&
                                             std::is_same_v<T, KT>>>
        std::string toString() const;

    private:
        /**
         * @brief It is the root node of the tree.
         */
        NodePtr root = nullptr;

        /**
         * @brief It flags if duplicate keys are allowed.
         */
        bool allowDuplicateKeys = true;

        /**
         * @brief It stores the nodes count to avoid multiple computations.
         *        It is updated with each insert or remove operation.
         *
         * @note Unlike `height` and `invalidateHeight` which are mutable,
         *       `count`, does not require mutability as in const case, no
         *       insertion / removal is possible and hence no change in `count`
         *       value.
         */
        size_t count = 0;

        /**
         * @brief It stores tree height. It is updated on insert and remove
         *        operations.
         *
         * @attention This is made mutable to allow caching the height when
         *            called by getHeight() which has const specifier.
         */
        mutable size_t height = 0;

        /**
         * @brief It is a flag used to signal that height should not be read
         *        from `height' (cached value), as it is invalid and thus force
         *        its recomputation.
         *
         * @attention This is made mutable to allow caching the height when
         *            called by getHeight() which has const specifier.
         */
        mutable bool invalidateHeight = false;

        /**
         * @brief Calculates the height of a node in the tree.
         *
         * @param node A shared pointer to the node whose height is to be
         *             calculated.
         *
         * @return The height of the node.
         */
        size_t getHeight(NodePtr node) const;

        /**
         * @brief Removes a childless node from the tree.
         *
         * @param node A reference to a shared pointer to the node to be
         *             removed.
         *
         * @note A childless node is a node that does not have any children
         *       (i.e., both the left and right child pointers are nullptr).
         *
         * @warning It is the responsibility of the caller to ensure that the
         *          provided node is indeed a childless node.
         */
        void removeChildlessNode(NodePtr &node);

        /**
         * @brief Removes a node with single child (having either the left or
         *       right child a nullptr but not both) from the tree.
         *
         * @param node A reference to a shared pointer to the node to be
         *             removed.
         *
         * @warning It is the responsibility of the caller to ensure that the
         *          provided node is indeed a single child node.
         */
        void removeSingleChildNode(NodePtr &node);

        /**
         * @brief Removes a node with two children from the tree.
         *
         * @param node A reference to a shared pointer to the node to be
         *             removed.
         *
         * @warning It is the responsibility of the caller to ensure that the
         *          provided node has left and right children.
         */
        void removeDoubleChildNode(NodePtr &node);

        /**
         * @brief Removes a general node regardless of its children statuses.
         *
         * @param node A reference to a shared pointer to the node to be
         *             removed.
         */
        void removeNode(NodePtr &node);

        /**
         * @brief It updates the balance factors in an AVL tree.
         *
         * @tparam flag (default: balanced) It is used to enable this method
         *         only if `balanced` is true, indicating AVL tree.
         *
         * @param node It is shared pointer to a shared pointer to a node.
         *
         * @return A shared pointer to a shared pointer to the deepest
         *         imbalanced node (closest to leaf nodes).
         *
         * @attention Here we did not enforce (flag == balanced) in enablement
         *            condition, as the method is `private` and thus it will
         *            not be tampered with.
         */
        template<bool flag = balanced, typename = std::enable_if_t<flag>>
        NodePtr* updateBalanceFactors(NodePtr &node);

        /**
         * @brief Performs left rotation for the specified node.
         *
         * @tparam flag (default: balanced) It is used to enable this method
         *         only if `balanced` is true, indicating AVL tree.
         *
         * @param node It is shared pointer reference to the node that
         *             requires left rotation to retain tree balance or as a
         *             step towards retaining this balance.
         */
        template<bool flag = balanced, typename = std::enable_if_t<flag>>
        void rotateLeft(NodePtr &node);

         /**
         * @brief Performs right rotation for the specified node.
         *
         * @tparam flag (default: balanced) It is used to enable this method
         *         only if `balanced` is true, indicating AVL tree.
         *
         * @param node It is shared pointer reference to the node that
         *             requires right rotation to retain tree balance or as a
         *             step towards retaining this balance.
         */
        template<bool flag = balanced, typename = std::enable_if_t<flag>>
        void rotateRight(NodePtr &node);

         /**
         * @brief It balances the tree if needed.
         *
         * @tparam flag (default: balanced) It is used to enable this method
         *         only if `balanced` is true, indicating AVL tree.
         */
        template<bool flag = balanced, typename = std::enable_if_t<flag>>
        void balance();

        /**
         * @brief Retrieves the nodes at the specified level in the tree.
         *
         * @param level The level for which to retrieve the nodes (should be
         *              less than tree's height).
         *
         * @param prevNodes (Optional) A vector of previously collected nodes
         *                  (default: empty).
         *
         * @return A vector of shared node pointers of the specified level.
         */
        NodePtrVector getNodesAtLevel(size_t level,
                                      NodePtrVector prevNodes = {})
                                      const;

        /**
         * @brief Returns the maximum string length representation per node
         *        starting with the specified node. It works recursively.
         *
         * @tparam T (default: KT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param node Shared pointer to the node for which getting maximum
         *             string length should start from.
         *
         * @return The maximum string length.
         */
        template<typename T = KT,
                 typename = std::enable_if_t<IsConvertibleToString<T>>>
        size_t getMaxStringLength(NodePtr node) const;

         /**
         * @brief Returns the maximum string length representation per node
         *        for the entire tree nodes.
         *
         * @tparam T (default: KT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @return The maximum string length.
         */
        template<typename T = KT,
                 typename = std::enable_if_t<IsConvertibleToString<T>>>
        size_t getMaxStringLength() const;

        /**
         * @brief Calculates the padding and interspacing for a given level in
         *        the tree in terms of node block size (assuming all node
         *        representation sizes are equal to one block). This is used
         *        for getting string representation of the tree.
         *
         * @tparam T (default: KT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param level The level in the tree for which to calculate the
         *              padding and interspacing.
         *
         * @return A pair of values representing the padding and interspacing
         *         for the specified level in terms of individual node block
         *         size. So, if the node block size is 12 characters then to
         *         get the padding and interspacing in characters, you need to
         *         multiply by node size (12 characters in this example).
         *
         * @note The padding represents the number of space blocks to insert
         *       before the starting node at the specified level and
         *       interspacing represents the number of space blocks to add
         *       between two adjacent nodes. To maintain tree structure, even
         *       empty nodes in the level should be replaced with space block
         *       equal to node block size.
         */
        template<typename T = KT,
                 typename = std::enable_if_t<IsConvertibleToString<T>>>
        std::pair<size_t, size_t> getPaddingInterSpacing(size_t level) const;

        /**
         * @brief Generates the string representation of the nodes at the
         *        specified level in the tree.
         *
         * @tparam T (default: KT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param level The level in the tree for which to generate the string
         *              representation.
         *
         * @param maxNodeStringLength The maximum length of the string
         *                             representation of a single node.
         *
         * @param nodes (Optional) The vector of nodes at the specified level.
         *              If not provided, it will be internally calculated.
         *
         * @return A string representing the nodes at the specified level.
         *
         * @note The string representation of each node is aligned based on
         *        the maximum node string length, ensuring consistent
         *        formatting across different levels. Empty nodes are
         *        represented by  spaces with the same length as a single
         *        node's string representation.
         */
        template<typename T = KT,
                 typename = std::enable_if_t<IsConvertibleToString<T>>>
        std::string getLevelString(size_t level,
                                   size_t maxNodeStringLength,
                                   NodePtrVector nodes = {})
                                   const;

        /**
         * @brief Returns the vector of sorted keys starting from the specified
         *        node. It works recursively.
         *
         * @param node Shared pointer to the node starting from which the
         *             sorted keys should be extracted.
         *
         * @param reverse It flags whether the order should be ascending
         *                (default: false) or descending (true).
         *
         * @return Vector of keys in requested order.
         */
        KeyVector getSortedKeys(NodePtr node, bool reverse = false) const;

         /**
         * @brief Returns the vector of sorted key-value pairs starting from
         *        the specified node. It works recursively.
         *
         * @tparam T (default: VT) It is used for enablement within the unnamed
         *         template argument.
         *
         * @param node Shared pointer to the node starting from which the
         *             sorted keys should be extracted.
         *
         * @param reverse It flags whether the order should be ascending
         *                (default: false) or descending (true).
         *
         * @return Vector of key-value pairs in requested order.
         */
        template<typename T = VT,
                 typename = std::enable_if_t<!std::is_void_v<T>>>
        std::vector<std::pair<KT, VT>> getSortedKeyValuePairs(NodePtr node,
                                                              bool reverse = false)
                                                              const;

        /**
         * @struct Mover
         *
         * @brief A helper class used to move a Tree object from a source to a
         *        destination. It is a friend of the Tree class, allowing it to
         *        access the internal data and perform the move operation.
         */
        struct Mover {
            /**
             * @brief Moves the contents of a source Tree instance to a
             *        destination Tree instance.
             *
             * @param source The source Tree instance to move from.
             *
             * @param destination The destination Tree instance to move to.
             *
             * @note This operation is noexcept, meaning it will not throw any
             *       exceptions.
             */
            static void moveTree(Tree &&source, Tree &destination) noexcept;
        };

        /**
         * @brief Declaring Mover to be a friend class to this Tree template
         *        class to allow its member methods a privileged access to all
         *        Tree members and internal data.
         */
        friend struct Mover;

};

/**
 * Creating aliases for binary search and AVL trees, maintaining same original
 * template argument list as Tree to avoid template argument list matching
 * issues. These aliases are called MetaBSTree and MetaAVLTree and are capable
 * of easily substituting Tree.
 *
 * Notice the use of unnamed `template<typename, typename, bool> class`, as
 * it is just a dummy placeholder that will be overriden by MetaBSTNode or
 * MetaAVLNode and only present for maintaining same original template argument
 * list as that of Tree. The same applies for the unnamed bool template
 * argument, which will be also overriden.
 *
 * Note that the `Meta` prefix signifies that the alias should be used where
 * the specific brand of Tree is needed without changing template argument
 * list maintaining correct substitution, such as in template metaprogramming.
 */
template<template<typename, typename, bool> class,
         typename KT,
         typename VT = void,
         bool = false>
using MetaBSTree = Tree<MetaBSTNode, KT, VT, false>;

template<template<typename, typename, bool> class,
         typename KT,
         typename VT = void,
         bool = true>
using MetaAVLTree = Tree<MetaAVLNode, KT, VT, true>;

/**
 * These are direct BSTree and AVLTree aliases, but unlike MetaBSTree and
 * MetaAVLTree, they cannot substitute Tree as they have shortened template
 * argument list.
 */
template<typename KT, typename VT = void>
using BSTree = MetaBSTree<MetaBSTNode, KT, VT>;

template<typename KT, typename VT = void>
using AVLTree = MetaAVLTree<MetaAVLNode, KT, VT>;

/**
 * Here we include tree_implementation.hpp. This is needed in templated
 * entities as implementation in .cpp file is only done for explicit
 * specialization of templated entities (meaning for cases where all
 * template arguments are known). As for primary and partial specialization
 * where not all template argument are known, the definitions of methods
 * should go alongside their definition in the same header file, as in our case
 * here. That's why we wrote implementation in another header-only file
 * and included it here, so the compiler finds the definitions with
 * declarations in same file and thus avoiding linking errors.
 */
#include "tree_implementation.hpp"

#endif
