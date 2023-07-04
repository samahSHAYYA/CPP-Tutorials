/**
 * @file node.hpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains the different node structure template.
 */

#ifndef NODE_HPP
#define NODE_HPP

#include <list>     // For using std::list
#include <memory>   // For using std::shared_ptr
#include <string>   // For using std::string
#include <utility>  // For using std::pair

#include "tree/type_traits_extensions.hpp"

/**
 * REMARK:
 *
 * In defining all the node templates below and specialization, we have not
 * used inheritance although it can help avoid some code duplication, as the
 * derived entities would not satisfy the Liskov Substitution Principle (the `L`
 * part of SOLID principles) that states that a derived class should correctly
 * be able to substitute its parent. This would not be possible in a Tree
 * formed of nodes as you cannot change node type once created an instance.
 * Also, as we are limiting Node functionality to current explicit
 * specialization, we made Node final and thus, cannot be inherited and further
 * specialized based on inheritance to keep it consistent with the Tree
 * functionality.
 *
 * Note that we made Node as struct with all members being public as we intend
 * to use it within a Tree structure but using private std::shared_ptr and
 * it would never return a Node or expose a Node. Thus, it is better to keep
 * it struct with public members, avoiding the need for accessors.
 *
 * If want to make Node as class with private members and only few public ones
 * and still allow Tree to have full access to it, we can do that by making
 * Tree a friend template class to Node. Nevertheless, try to kept as simple
 * as possible (simple enough and not simpler).
 *
 * Also, although we have repeated definition of same methods in several
 * specializations, in the implementation, we are going to see how can make
 * this simple without repetition of implementation code.
 */

/**
 * @struct Node
 *
 * @brief Represents a node in a binary tree.
 *
 * @tparam KT The type of the key stored in the node.
 *
 * @tparam VT The type of the value stored in the node (void: default).
 *
 * @tparam balanced Tells whether the node is BSTNode (false : default) or
 *                  AVLNode (true).
 *
 * @tparam ET EnableType used to enable or disable template.
 *
 * @note The class is made final to prevent its inheritance as it includes all
 *       functionalities for binary search and AVL trees.
 */
template<typename KT,
         typename VT = void,
         bool balanced = false,
         typename ET = std::enable_if_t<IsComparable<KT>>>
struct Node final {
    /**
     * @brief The key stored in the node.
     */
    KT key;

    /**
     * @brief The value associated with the key.
     */
    VT value;

    /**
     * @brief Shared pointer to the left child node.
     *
     * @note We explicitly made it defaulted to nullptr although it will
     *       will hold that value by default. This adheres to best practices
     *       regarding smart and classical pointer being more explicit, more
     *       readable and more consistent.
     */
    std::shared_ptr<Node> left = nullptr;

    /**
     * @brief Shared pointer to the right child node.
     */
    std::shared_ptr<Node> right = nullptr;

    /**
     * @brief Default empty constructor is deleted.
     */
    Node() = delete;

    /**
     * @brief Constructs a `Node` object with the given key and value.
     *
     * @param key The key to be stored in the node.
     *
     * @param value The value associated with the key.
     *
     * @note The constructor initializes the `key` member with the provided
     *       `key` parameter, and the `value` member with the provided `value`
     *       parameter. The `left` and `right` child pointers are initialized
     *       to `nullptr`.
     */
    Node(KT key, VT value);

    /**
     * @brief Returns a string representation of the BST node.
     *
     * @tparam T (default: KT) It is used for enablement within the unnamed
     *         template argument.
     *
     * @return A string describing the node.
     *
     * @note The std::enable_if_t is used to enable definition of this function
     *       in case at least KT is convertible to string type. In case of VT,
     *       if it is convertible to string, it will be represented; otherwise,
     *       it will be discarded in string representation.
     */
    template<typename T = KT,
             typename = std::enable_if_t<IsConvertibleToString<T> &&
                                         std::is_same_v<T, KT>>>
    std::string toString() const;

    /**
     * @brief Returns the key-value pair representation of the node.
     *
     * @return A key-value pair describing the node.
     */
    std::pair<KT, VT> getKeyValuePair() const;
};

/**
 * @struct Node
 *
 * @brief Partial Specialization for Key-Only BSTNode (balanced = false).
 */
template<typename KT, typename ET>
struct Node<KT, void, false, ET> {
    /**
     * @brief The key stored in the node.
     */
    KT key;

    /**
     * @brief Shared pointer to the left child node.
     */
    std::shared_ptr<Node> left = nullptr;

    /**
     * @brief Shared pointer to the right child node.
     */
    std::shared_ptr<Node> right = nullptr;

    /**
     * @brief Default empty constructor is deleted.
     */
    Node() = delete;

    /**
     * @brief Constructs a `Node` object with the given key.
     *
     * @param key The key to be stored in the node.
     */
    Node(KT key);

    /**
     * @brief Returns a string representation of the BST node.
     *
     * @tparam T (default: KT) It is used for enablement within the unnamed
     *         template argument.
     *
     * @return A string describing the node.
     */
    template<typename T = KT,
             typename = std::enable_if_t<IsConvertibleToString<T> &&
                                         std::is_same_v<T, KT>>>
    std::string toString() const;
};

// Below are AVL / Balanced Node Partial Specialization
/**
 * @struct Node
 *
 * @brief Partial specialization for AVLNode (balanced = true), and in the
 *        presence of VT.
 *
 * @note Here, it is necessary to mention the `true` value of 'balanced' as the
 *       default value of `balanced' is false (in case dismissed).
 */
template<typename KT, typename VT, typename ET>
struct Node<KT, VT, true, ET> {
    /**
     * @brief The key stored in the node.
     */
    KT key;

    /**
     * @brief The value associated with the key.
     */
    VT value;

    /**
     * @brief The balance factor (defaulted to zero explicitly).
     */
    int balanceFactor = 0;

    /**
     * @brief Stores duplicate keys.
     */
    std::list<std::pair<KT, VT>> duplicates;

    /**
     * @brief Shared pointer to the left child node.
     */
    std::shared_ptr<Node> left = nullptr;

    /**
     * @brief Shared pointer to the right child node.
     */
    std::shared_ptr<Node> right = nullptr;

    /**
     * @brief Default empty constructor is deleted.
     */
    Node() = delete;

    /**
     * @brief Constructs a `Node` object with the given key and value.
     *
     * @param key The key to be stored in the node.
     *
     * @param value The value associated with the key.
     *
     * @note The constructor initializes the `key` member with the provided
     *       `key` parameter, and the `value` member with the provided `value`
     *       parameter. The `left` and `right` child pointers are initialized
     *       to `nullptr`.
     */
    Node(KT key, VT value);

    /**
     * @brief Adds a pair of duplicate key and a value.
     *
     * @param key The key to add. Note that although `key` is equal to
     *            `this->key`, they might not be effectively identical in
     *            the sense that not all `KT` data fields might be considered
     *            in comparison operations.
     *
     * @param value The value to add long the passed key.
     *
     * @return True if successfully added the duplicate item and false
     *         otherwise.
     */
    bool addDuplicateItem(KT key, VT value);

    /**
     * @brief Returns a pair containing the last entered duplicate key and its
     *        associated value.
     *
     * @return The last pair of key and value.
     *
     * @note In case we have duplicates, it will return the last item in
     *       `duplicates`. If no duplicates, it will return `this->key` and
     *        `this->value` as a pair.
     */
    std::pair<KT, VT> getLastEncounteredItem() const;

    /**
     * @brief Searches if node or its duplicates contain an item with value
     *        equal to the passed `value`. The return is a tuple of boolean,
     *        KT key instance, and VT value instance. The key and value parts
     *        are to be considered on successful search where the boolean part
     *        is true.
     *
     * @param lastEncounter (default: false) It tells whether we should stop
     *                      on first encounter (default: false) or on last
     *                      encounter (true).
     *
     * @return The search result being a three-element tuple of boolean, key,
     *         and value.
     */
    std::tuple<bool, KT, VT> searchItem(VT value,
                                        bool lastEncounter = false)
                                        const;

    /**
     * @brief Returns a string representation of the AVL node.
     *
     * @tparam T (default: KT) It is used for enablement within the unnamed
     *         template argument.
     *
     * @return A string describing the node.
     */
    template<typename T = KT,
             typename = std::enable_if_t<IsConvertibleToString<T> &&
                                         std::is_same_v<T, KT>>>
    std::string toString() const;

    /**
     * @brief Returns the key-value pair representation of the node.
     *
     * @return A key-value pair describing the node.
     */
    std::pair<KT, VT> getKeyValuePair() const;

    /**
     * @brief Returns key occurrence count, which is 1 + duplicates.size().
     *
     * @return The total number of key occurrences.
     */
    size_t getCount() const;

    /**
     * @brief Returns key-value occurrence count where both key and value are
     *        equal.
     *
     * @param value The value to search for along side the node key.
     *
     * @return The total number of key-value occurrences.
     */
    size_t getCount(VT value) const;
};

/**
 * @struct Node
 *
 * @brief Partial Specialization for Key-Only AVLNode (balanced = true).
 */
template<typename KT, typename ET>
struct Node<KT, void, true, ET> {
    /**
     * @brief The key stored in the node.
     */
    KT key;

    /**
     * @brief The balance factor (defaulted to zero explicitly).
     */
    int balanceFactor = 0;

    /**
     * @brief Stores duplicate keys.
     */
    std::list<KT> duplicates;

    /**
     * @brief Shared pointer to the left child node.
     */
    std::shared_ptr<Node> left = nullptr;

    /**
     * @brief Shared pointer to the right child node.
     */
    std::shared_ptr<Node> right = nullptr;

    /**
     * @brief Default empty constructor is deleted.
     */
    Node() = delete;

    /**
     * @brief Constructs a `Node` object with the given key.
     *
     * @param key The key to be stored in the node.
     */
    Node(KT key);

    /**
     * @brief Adds the duplicate key.
     *
     * @param key The key to add. Note that although `key` is equal to
     *            `this->key`, they might not be effectively identical in
     *            the sense that not all `KT` data fields might be considered
     *            in comparison operations.
     *
     * @return True if successfully added the duplicate item and false
     *         otherwise.
     */
    bool addDuplicateItem(KT key);

    /**
     * @brief Returns the last entered duplicate key.
     *
     * @return The last entered key.
     *
     * @note In case we have duplicates, it will return the last key in
     *       `duplicates`. If no duplicates, it will return `this->key'.
     */
    KT getLastEncounteredItem() const;

    /**
     * @brief Returns a string representation of the AVL node.
     *
     * @tparam T (default: KT) It is used for enablement within the unnamed
     *         template argument.
     *
     * @return A string describing the node.
     */
    template<typename T = KT,
             typename = std::enable_if_t<IsConvertibleToString<T> &&
                                         std::is_same_v<T, KT>>>
    std::string toString() const;

    /**
     * @brief Returns key occurrence count, which is 1 + duplicates.size().
     *
     * @return The total number of key occurrences.
     */
    size_t getCount() const;
};

/**
 * Defining some aliases for brevity and clarity.
 * Note that we introduced the unnamed bool arg defaulted to false in
 * MetaBSTNode and true in MetaAVLNode as to have MetaBSTNode and MetaAVLNode
 * capable of replacing Node maintaining the same template argument list. Here,
 * the unnamed defaulted bools are discarded even if passed by the user. They
 * will be overridden to false in MetaBSTNode and to true in MetaAVLNode.
 *
 * Note that in the Node template arguments, the last EnableType (ET) is
 * discarded in the signature as it is automatically deduced.
 *
 * Use this as good practice as it allows easier substitution of Node by these
 * aliases without problems regarding template argument list matching.
 *
 * Note that the `Meta` prefix signifies that the alias should be used where
 * the specific brand of Node is needed without changing template argument
 * list maintaining correct substitution, such as in template metaprogramming.
 */
template<typename KT, typename VT = void, bool = false>
using MetaBSTNode = Node<KT, VT, false>;

template<typename KT, typename VT = void, bool = true>
using MetaAVLNode = Node<KT, VT, true>;

/**
 * Here we define BSTNode and AVLNode by reducing template argument list.
 * Nevertheless, as these have different argument list than Node, they cannot
 * replace Node.
 */
template<typename KT, typename VT = void>
using BSTNode = Node<KT, VT, false>;

template<typename KT, typename VT = void>
using AVLNode = Node<KT, VT, true>;

/**
 * Here we include node_implementation.hpp. This is needed in templated
 * entities as implementation in .cpp file is only done for explicit
 * specialization of templated entities (meaning for cases where all
 * template arguments are known). As for primary and partial specialization
 * where not all template argument are known, the definitions of methods
 * should go alongside their definition in the same header file, as in our case
 * here. That's why we wrote implementation in another header-only file
 * and included it here, so the compiler finds the definitions with
 * declarations in same file and thus avoiding linking errors.
 */
#include "node_implementation.hpp"

#endif
