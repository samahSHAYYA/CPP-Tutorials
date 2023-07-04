/**
 * @file application.cpp
 *
 * @author Samah A. SHAYYA
 *
 * @brief This file contains the implementation of application.hpp.
 */

#include <string>
#include <filesystem>

#include "application.hpp"

// Creating an alias for std::filesystem namespace
namespace fs = std::filesystem;

/**
 * Anonymous namespace to hide helper methods making them visible only in
 * this translation unit (i.e., this cpp file).
 */
namespace {
    bool getUserInput(bool defaultValue) {
        std::string inputStr;
        bool ans = defaultValue;

        /**
         * Note that std::getline allows you to read a full line of input,
         * including spaces, and store it as a string.
        */
        std::getline(std::cin, inputStr);

        if(defaultValue) {
            // Check if it is no (meaning n or N).
            if(!inputStr.empty() && std::tolower(inputStr[0]) == 'n') {
                ans = false;
            }
        }
        else {
            // Check if it is yes (meaning y or Y).
            if(!inputStr.empty() && std::tolower(inputStr[0]) == 'y') {
                ans = true;
            }
        }

        return ans;
    }

    size_t getUserInput(size_t defaultValue) {
        std::string inputStr;
        size_t ans = defaultValue;

        std::getline(std::cin, inputStr);

        if(!inputStr.empty()) {
            /**
             * Below, if we can discard all exceptions by using
             * `catch(...)`, but this is less recommended.
             */
            try {
                ans = std::stoull(inputStr);
            }
            catch (const std::exception &e) {
                // We will discard it.
                std::cout << "Your input is invalid and defaultValue ";
                std::cout << "= " << defaultValue << " was used!\n";
            }
        }

        return ans;
    }

    std::string getUserInput() {
        std::string ans;
        std::getline(std::cin, ans);

        return ans;
    }

    int getValidKey(std::string msg) {
        std::string inputStr;
        int key;

        while(true) {
            inputStr = getUserInput();

            /**
             * Note using std::cin >> key is not enough as it stills accepts
             * float numbers by taking the integral part, which we do not
             * want. That's why we get input as string, check if it does not
             * contain decimal point and then attempt converting it to integer.
             */
            if(inputStr.find(".") == std::string::npos) {
                try {
                    key = std::stoi(inputStr);
                    break;
                }
                catch(const std::exception &e) {
                    // Re-ask for data entry.
                }
            }

            // Failed to read valid key.
            std::cout << msg;
        }

        return key;
    }

}

Application::Application() {
    /**
     * We can connect signal only with a standalone method or static class
     * method such as here (handleSignal is static method).
     */
    std::signal(SIGINT, handleSignal);

    // Assuring boolean values are printed as true / false.
    std::cout << std::boolalpha;
}

bool Application::hasValue() const {
    size_t index = tree.index();

    return index >= 2;
}

void Application::displayWelcomeMessage() {
    std::string msg = "Welcome to Tree Console Application!";
    msg += "\n(To terminate press Ctrl+C)\n\n";
    std::cout << msg;
}

void Application::requestTreeType() {
    std::string msg = "Choose the tree type:\n";
    msg += "0 -> KEY_ONLY_BSTREE (default)\n";
    msg += "1 -> KEY_VALUE_BSTREE\n";
    msg += "2 -> KEY_ONLY_AVLTREE\n";
    msg += "3 -> KEY_VALUE_AVLTREE\n";
    msg += "Enter your choice (default: 0): ";
    std::cout << msg;

    /**
     * Notice the use of UL suffix to hint on which overload version of
     * getUserInput to use but note that size_t might have different size
     * than unsigned long. A better approach would be to use static_cast.
     *
     * So, `size_t  choice = getUserInput(0UL)' is fine and the following is
     * much better:
     * 'size_t choice = getUserInput(static_cast<size_t>(0))`.
     */
    size_t choice = getUserInput(static_cast<size_t>(0));

    msg = "Do you want to allow duplicate keys: y|Y = yes (default) / ";
    msg += "n|N = no? ";
    std::cout << msg;
    bool allowDuplicates = getUserInput(true);

    switch(choice) {
        case 1:
            tree = BSTree<int, std::string>(allowDuplicates);
            break;

        case 2:
            tree = AVLTree<int>(allowDuplicates);
            break;

        case 3:
            tree = AVLTree<int, std::string>(allowDuplicates);
            break;

        default:
            // 0 or > 3
            tree = BSTree<int>(allowDuplicates);
    }

    std::cout << "\n\n";
}

void Application::performInsertion() {
    int key = 0;
    std::string value = "";
    bool inserted;

    std::string msg = "Enter integer key: ";
    std::cout << msg;
    key = getValidKey(msg);

    if(hasValue()) {
        msg = "Enter string value (default = \"\"): ";
        std::cout << msg;
        value = getUserInput();
    }

    size_t index = tree.index();
    if(hasValue()) {
        /**
         * We can use `std::holds_alternative<BSTree<int, std::string>>(tree)`
         * to check if it holds the BSTree key-value version or AVLTree version
         * as active type.
         *
         * @code{.cpp}
         *  if(std::holds_alternative<BSTree<int, std::string>>(tree)) {
         *      inserted = std::get<BSTree<int>>(tree).insert(key);
         *   }
         *   else {
         *      inserted = std::get<AVLTree<int>>(tree).insert(key);
         *   }
         * @code
         *
         * Below, we use index. Note as index is determined at runtime we need
         * to check its value and pass corresponding const value to
         * 'std::get' as below.
         */
        if(index == 2) {
            inserted = std::get<2>(tree).insert(key, value);
        }
        else {
            inserted = std::get<3>(tree).insert(key, value);
        }
    }
    else {
        if(index == 0) {
            inserted = std::get<0>(tree).insert(key);
        }
        else {
            inserted = std::get<1>(tree).insert(key);
        }
    }

    if(inserted) {
        std::cout << "Insertion was successful!\n";
    }
    else {
        std::cout << "Insertion was unsuccessful!\n";
    }
}

void Application::performRemoval() {
    int key = 0;
    bool enterValue;
    bool deleteAll = false;
    std::string value = "";
    size_t deletedCount;

    std::string msg = "Enter the integer key to remove: ";
    std::cout << msg;
    key = getValidKey(msg);

    if(hasValue()) {
        msg = "Do you want to enter a value as well ";
        msg += "(y|Y = yes / n|N = no (default)): ";
        std::cout << msg;

        enterValue = getUserInput(false);
        if(enterValue) {
            msg = "Please enter string value: ";
            std::cout << msg;
            value = getUserInput();
        }
    }

    // Check if all occurrences should be deleted.
    msg = "Do you want to delete all occurrences (y|Y = yes / n|N = no";
    msg += " default): ";
    std::cout << msg;
    deleteAll = getUserInput(false);

   size_t index = tree.index();
   if(hasValue()) {
        if(enterValue) {
            if(index == 2) {
                deletedCount = std::get<2>(tree).remove(key, value, deleteAll);
            }
            else {
                deletedCount = std::get<3>(tree).remove(key, value, deleteAll);
            }
        }
        else {
            if(index == 2) {
                deletedCount = std::get<2>(tree).remove(key, deleteAll);
            }
            else {
                deletedCount = std::get<3>(tree).remove(key, deleteAll);
            }
        }
   }
   else {
        if(index == 0) {
            deletedCount = std::get<0>(tree).remove(key, deleteAll);
        }
        else {
            deletedCount = std::get<1>(tree).remove(key, deleteAll);
        }
   }

    std::cout << "Removed " << deletedCount << " instances.\n";
}

void Application::performSearch() const {
    int key = 0;
    bool enterValue;
    bool lastEncounter = false;
    std::string value = "";

    std::string msg = "Enter the integer key to search: ";
    std::cout << msg;
    key = getValidKey(msg);

    if(hasValue()) {
        msg = "Do you want to enter a value as well ";
        msg += "(y|Y = yes / n|N = no (default)): ";
        std::cout << msg;
        enterValue = getUserInput(false);

        if(enterValue) {
            msg = "Please enter string value: ";
            std::cout << msg;
            value = getUserInput();
        }
    }

    // Check if all occurrences should be deleted.
    msg = "Do you want to search for last occurrence (y|Y = yes / n|N = no";
    msg += " default): ";
    std::cout << msg;
    lastEncounter = getUserInput(false);

    size_t index = tree.index();
    bool found;
    int foundKey;
    std::string foundValue;

    std::pair<bool, int> pairResult;
    std::tuple<bool, int, std::string> tupleResult;

    switch(index) {
        case 1:
            pairResult = std::get<1>(tree).search(key, lastEncounter);
            found = pairResult.first;
            foundKey = pairResult.second;
            break;

        case 2:
            tupleResult = enterValue ? std::get<2>(tree).search(key,
                                                                value,
                                                                lastEncounter)
                                            : std::get<2>(tree).search(key,
                                                                       lastEncounter);

            found = std::get<0>(tupleResult);
            foundKey = std::get<1>(tupleResult);
            foundValue = std::get<2>(tupleResult);
            break;

        case 3:
            tupleResult = enterValue ? std::get<3>(tree).search(key,
                                                                value,
                                                                lastEncounter)
                                            : std::get<3>(tree).search(key,
                                                                       lastEncounter);

            found = std::get<0>(tupleResult);
            foundKey = std::get<1>(tupleResult);
            foundValue = std::get<2>(tupleResult);
            break;

        default:
            // Case 0
            pairResult= std::get<0>(tree).search(key, lastEncounter);
            found = pairResult.first;
            foundKey = pairResult.second;
            break;
    }

    std::cout << "Search result: found = " << found;
    if(found) {
        std::cout << ", key = " << foundKey;

        if(hasValue()) {
            std::cout << ", value = " << foundValue;
        }
    }

    std::cout << ".\n";
}

void Application::performClear() {
    std::string msg = "Are you sure you want to clear the entire tree ";
    msg += "(y|Y = yes / n|N = no - default)? ";
    std::cout << msg;

    if(getUserInput(false)) {
        /**
         * Clear the tree. Here it is simpler to use std::visit as clear has
         * same signature in all variant types.
         */
        std::visit([&](auto &activeTree){activeTree.clear();}, tree);
        std::cout << "The tree has been cleared.\n";
    }
    else {
        std::cout << "The clearing operation has been cancelled.\n";
    }
}

void Application::performSerialization() const {
    std::string msg = "Enter a valid filepath (default: tree.bin): ";
    std::string inputStr;
    fs::path filepath;

    std::cout << msg;
    inputStr = getUserInput();

    /**
     * Checking if inputStr corresponds to a valid filepath whether absolute
     * or relative (we assure it has no illegal characters).
     */
    while(!inputStr.empty()) {
        filepath = inputStr;

        // If invalid path, we should re-ask for one.
        if(!filepath.is_absolute() && !filepath.is_relative()) {
            std::cout << "Invalid path entered (" << filepath << ")!\n";
            std::cout << msg;
            inputStr = getUserInput();
        }
        else {
            // Valid path.
            break;
        }
    }

    if(inputStr.empty()) {
        filepath = "tree.bin";
    }

    std::cout << "Saving to " << filepath << "...\n";

    // Now we can serialize.
    bool serialized;
    std::visit([&](auto &activeTree) {
               serialized = activeTree.serialize(filepath.string());},
               tree);

    if(serialized) {
        std::cout << "The tree has been successfully saved to: ";
        std::cout << filepath << ".\n";
    }
    else {
        std::cerr << "The tree has not been saved to: ";
        std::cerr << filepath << ".\n";
    }
}

void Application::performDeserialization() {
    std::string msg = "Enter the filepath to load: ";
    fs::path filepath;

    std::cout << msg;

    while(true) {
        filepath = getUserInput();
        if(fs::is_regular_file(filepath)) {
            break;
        }

        std::cout << "The given filepath (" << filepath << ") does not ";
        std::cout << "exist or is invalid.\n";
        std::cout << msg;
    }

    // Upon deserialization, any errors that occur will be reported.
    size_t index = tree.index();
    switch(index) {
        case 1:
            tree = AVLTree<int>(filepath.string());
            break;

        case 2:
            tree = BSTree<int, std::string>(filepath.string());
            break;

        case 3:
            tree = AVLTree<int, std::string>(filepath.string());
            break;

        default:
            // Case 0
            tree = BSTree<int>(filepath.string());
    }

    std::cout << "Deserialization was done!";
}

void Application::performExit() {
    std::string msg = "Are you sure you want to exit (y|Y = yes / n|N = no ";
    msg += "- default)? ";
    std::cout << msg;

    if(getUserInput(false)) {
        // Exiting with no error.
        exit(0);
    }
    // Else exit will be cancelled.
}


void Application::requestOperation() {
    size_t choice = 0;
    std::string msg = "Choose operation:\n";
    msg += "0 -> Insert node (default)\n";
    msg += "1 -> Remove\n";
    msg += "2 -> Search\n";
    msg += "3 -> Clear\n";
    msg += "4 -> Save\n";
    msg += "5 -> Load\n";
    msg += "6 -> Exit\n";
    msg += "Enter your choice (default = 0 - i.e., insert): ";
    std::cout << msg;
    choice = getUserInput(static_cast<size_t>(0));

    switch(choice) {
        case 1:
            performRemoval();
            break;

        case 2:
            performSearch();
            break;

        case 3:
            performClear();
            break;

        case 4:
            performSerialization();
            break;

        case 5:
            performDeserialization();
            break;

        case 6:
            performExit();
            break;

        default:
            performInsertion();
    }

    // Displaying the tree after each operation.
    std::visit([&](const auto &activeTree){
        std::cout << "\n" << activeTree.toString();
    }, tree);

    std::cout << "\n" << std::string(80, '=') << "\n\n";
}

void Application::handleSignal(int signal) {
    std::cout << "\nCtrl+C signal (Termination)." << std::endl;
    std::exit(0);
}

void Application::run() {
    displayWelcomeMessage();
    requestTreeType();

    while(true) {
        requestOperation();
    }
}
