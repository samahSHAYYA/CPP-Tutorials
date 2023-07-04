/**
 * @file node_implementation.hpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains the implementation of node structure template
 *        defined in node.hpp.
 */

#ifndef NODE_IMPLEMENTATION_HPP
#define NODE_IMPLEMENTATION_HPP

#include <type_traits>

#include "tree/node.hpp"

/**
 * @brief In the following anonymous namespace, we place the helper functions
 *        that will be used in the implementation of Node methods, making them
 *        visible only within this file.
 */
namespace
{
    /**
     * @brief Converts a Node object to a string representation.
     *
     * @tparam KT The type of the key stored in the Node.
     *
     * @tparam VT The type of the value stored in the Node (default: void).
     *
     * @tparam balanced Tells whether the Node is a balanced node (default:
     *                  false).
     *
     * @tparam ET It is the EnableType propagated from Node.
     *
     * @param node The Node object to convert to a string.
     *
     * @return A string representation of the Node.
     *
     * @attention We could have set the unnamed enable type to ET and used it.
     *            But we kept ET as separate template parameter for clarity.
     */
    template<typename KT,
             typename VT,
             bool balanced,
             typename ET,
             typename = std::enable_if_t<IsConvertibleToString<KT>>>
    std::string toString(const Node<KT, VT, balanced, ET> &node) {
        std::string str = "<K = ";

        /**
         * In all cases, we have key.
         * Below notice how we the constexpr to signal to compiler
         * that the following is constant and replacement should be done at
         * compile time. Note it will not wok if you make it as follows:
         * `if (constexpr (std::is_fundamental_v<KT>)`. Use:
         * `if constexpr (the-expression-to-evaluate-at-compile-time)'.
         *
         * Note this is C++17 feature.
         */
        if constexpr (std::is_fundamental_v<KT>) {
            str += std::to_string(node.key);
        }
        else {
            if constexpr (std::is_same_v<KT, std::string> ||
                          std::is_base_of_v<std::string, KT>) {
                str += node.key;
            }
            else {
                // It has a toString() method
                str += node.key.toString();
            }
        }

        if constexpr (!std::is_void_v<VT> && IsConvertibleToString<VT>) {
            // ValueType is not void and can be converted to string
            str += ", V = ";

            if constexpr (std::is_fundamental_v<VT>) {
                str += std::to_string(node.value);
            }
            else {
                if constexpr (std::is_same_v<VT, std::string> ||
                              std::is_base_of_v<std::string, VT>) {
                    str += node.value;
                }
                else {
                    // It has toString() method
                    str += node.value.toString();
                }
            }
        }

        if constexpr (balanced) {
            // We need to add balanceFactor value.
            str += ", BF = " + std::to_string(node.balanceFactor);

            // We also need to add overall count.
            str += ", C = " + std::to_string(node.getCount());
        }

        // We just need to close the string representation.
        str += ">";

        return str;
    }

    /**
     * @brief Retrieves the key-value pair from a Node object.
     *
     * @tparam KT The type of the key stored in the Node.
     *
     * @tparam VT The type of the value stored in the Node.
     *
     * @tparam balanced Tells whether the Node is a balanced node (default:
     *         false).
     *
     * @tparam ET It is the EnableType propagated from Node.
     *
     * @param node The Node object from which to retrieve the key-value pair.
     *
     * @return A std::pair containing the key and value of the Node.
     *
     * @note This function is be use with nodes that non-void VT type.
     *
     * @attention We could have set the unnamed enable type to ET and used it.
     *            But, we kept ET as separate template parameter for clarity.
     */
    template<typename KT,
             typename VT,
             bool balanced,
             typename ET,
             typename = std::enable_if_t<!std::is_void_v<VT>>>
    std::pair<KT, VT> getKeyValuePair(const Node<KT, VT, balanced, ET> &node) {
        // Note std:make_pair automatically identifies key and value types.
        return std::make_pair(node.key, node.value);
    }
}

/**
 * REMARK:
 *
 * We can here simplify by using BSTNode<KT, VT> alias instead of
 * Node<KT, VT, false>, but we did not do that to maintain clarity (avoiding
 * ambiguity due to having class scope name, BSTNode, while constructor name is
 * Node).
 */
template<typename KT, typename VT, bool balanced, typename ET>
Node<KT, VT, balanced, ET>::Node(KT key, VT value) : key(key), value(value) {
};

template<typename KT, typename VT, bool balanced, typename ET>
template<typename, typename>
std::string Node<KT, VT, balanced, ET>::toString() const {
    /**
     * Here we use ::toString to refer to toString in anonymous namespace
     * because it has same name of Node::toString. If we had different name,
     * we can use it without :: operator (to access anonymous namespace
     * entities).
     */
    return ::toString<KT, VT, false>(*this);
}

template<typename KT, typename VT, bool balanced, typename ET>
std::pair<KT, VT> Node<KT, VT, balanced, ET>::getKeyValuePair() const {
    return ::getKeyValuePair<KT, VT, false>(*this);
}

template<typename KT, typename ET>
Node<KT, void, false, ET>::Node(KT key) : key(key) {
};

template<typename KT, typename ET>
template<typename, typename>
std::string Node<KT, void, false, ET>::toString() const {
    return ::toString<KT, void, false>(*this);
}

// AVLNode implementation
template<typename KT, typename VT, typename ET>
Node<KT, VT, true, ET>::Node(KT key, VT value) : key(key), value(value) {
};

template<typename KT, typename VT, typename ET>
bool Node<KT, VT, true, ET>::addDuplicateItem(KT key, VT value) {
    bool added = false;

    // Validating if we are really adding a duplicate key.
    if(this->key == key) {
        duplicates.emplace_back(key, value);
        added = true;
    }

    return added;
};

template<typename KT, typename VT, typename ET>
std::pair<KT, VT> Node<KT, VT, true, ET>::getLastEncounteredItem() const {
    std::pair<KT, VT> lastItem(key, value);

    if(!duplicates.empty()){
        lastItem = duplicates.back();
    }

    return lastItem;
}

template<typename KT, typename VT, typename ET>
std::tuple<bool, KT, VT> Node<KT, VT, true, ET>::searchItem(VT value,
                                                            bool lastEncounter)
                                                            const {
    std::tuple<bool, KT, VT> searchResult;  // Bool part is false by default

    if(this->value == value) {
        /**
         * Note we use `this->value` and not `value` as we need the real
         * entered value, as VT comparison operator might not be considering
         * all its data members, meaning `this->value` and `value` although
         * equal, they might be effectively non-identical.
         */
        searchResult = std::make_tuple(true, key, this->value);
    }

    if(!duplicates.empty() &&
       (!std::get<bool>(searchResult)) || lastEncounter) {
        if(lastEncounter) {
            /**
             * Better traverse the list in reverse manner and stop on first
             * encounter as compared to mandatorily traversing the list from
             * start till end updating on each encounter the searchResult.
             */
            for(auto it = duplicates.rbegin(); it != duplicates.rend(); ++it) {
                if(it->second == value) {
                    searchResult = std::make_tuple(true, it->first, it->second);
                    break;
                }
            }
        }
        else {
            /**
             * First encounter: better traverse from start and stop on
             * encounter.
             */
            for(const auto &item : duplicates) {
                if(item.second == value) {
                    searchResult = std::make_tuple(true,
                                                   item.first,
                                                   item.second);
                    break;
                }
            }
        }
    }
    // Else nothing to do (searchResult contains what we need).

    return searchResult;
}

template<typename KT, typename VT, typename ET>
template<typename, typename>
std::string Node<KT, VT, true, ET>::toString() const {
    return ::toString<KT, VT, true>(*this);
}

template<typename KT, typename VT, typename ET>
std::pair<KT, VT> Node<KT, VT, true, ET>::getKeyValuePair() const {
    return ::getKeyValuePair<KT, VT, true>(*this);
}

template<typename KT, typename VT, typename ET>
size_t Node<KT, VT, true, ET>::getCount() const {
    return 1 + duplicates.size();
}

template<typename KT, typename VT, typename ET>
size_t Node<KT, VT, true, ET>::getCount(VT value) const {
    size_t count = this->value == value ? 1 : 0;

    for(const auto &pair : duplicates) {
        if(pair.second == value) {
            ++count;
        }
    }

    return count;
}

template<typename KT, typename ET>
Node<KT, void, true, ET>::Node(KT key) : key(key) {
};

template<typename KT, typename ET>
bool Node<KT, void, true, ET>::addDuplicateItem(KT key) {
    bool added = false;

    // Validating if we are really adding a duplicate key.
    if(this->key == key) {
        duplicates.emplace_back(key);
        added = true;
    }

    return added;
};

template<typename KT, typename ET>
KT Node<KT, void, true, ET>::getLastEncounteredItem() const {
    KT lastKey = key;

    if(!duplicates.empty()){
        lastKey = duplicates.back();
    }

    return lastKey;
}

template<typename KT, typename ET>
template<typename, typename>
std::string Node<KT, void, true, ET>::toString() const {
    return ::toString<KT, void, true>(*this);
}

template<typename KT, typename ET>
size_t Node<KT, void, true, ET>::getCount() const {
    return 1 + duplicates.size();
}

#endif
