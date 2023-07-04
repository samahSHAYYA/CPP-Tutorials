/**
 * @file application.hpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains the Application class definition.
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <iostream>
#include <variant>          // For std::variant
#include <csignal>          // For signal handling

#include "tree/tree.hpp"

/**
 * @class Application
 *
 * @brief Represents the Tree Application that allows performing operations on
 *        a binary tree (BSTree or AVLTree).
 */
class Application {
    public:
        Application();

        void run();

    private:
        /**
         * We can wrap all types below within a union type. Nevertheless,
         * union is more appropriate for primitive types. In C++17, there
         * is better choice which is std::variant that keeps track of
         * active type.
         *
         * You can test if a typ T is the active type using:
         * std::holds_alternative<T>(instance); // Returns bool
         *
         */
        using TreeType = std::variant<BSTree<int>,
                                      AVLTree<int>,
                                      BSTree<int, std::string>,
                                      AVLTree<int, std::string>>;

        TreeType tree;

        bool hasValue() const;

        void displayWelcomeMessage();

        void requestTreeType();

        void performInsertion();

        void performRemoval();

        void performSearch() const;

        void performClear();

        void performSerialization() const;

        void performDeserialization();

        void performExit();

        void requestOperation();

        /**
         * @brief Signal handler for Ctrl+C.
         * @param signal The signal number.
         */
        static void handleSignal(int signal);
};

#endif
