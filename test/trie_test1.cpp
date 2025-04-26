#include "../trie/src.hpp"
#include <iostream>
#include <string>

int main() {
    sjtu::Trie trie;
    trie = trie.Put<std::string>("key1", "value1");
    trie = trie.Put<std::string>("key2", "value2");
    trie = trie.Put<std::string>("key3", "value3");
    trie = trie.Put<std::string>("key4", "value12");

    if (*trie.Get<std::string>("key1") != "value1") {
        std::cout << "Test failed: key1 does not return value1" << std::endl;
        return 1;
    }

    if (*trie.Get<std::string>("key2") != "value2") {
        std::cout << "Test failed: key2 does not return value2" << std::endl;
        return 1;
    }

    if (*trie.Get<std::string>("key3") != "value3") {
        std::cout << "Test failed: key3 does not return value3" << std::endl;
        return 1;
    }

    if (*trie.Get<std::string>("key4") != "value12") {
        std::cout << "Test failed: key4 does not return value4" << std::endl;
        return 1;
    }

    trie = trie.Remove("key2");

    if (trie.Get<std::string>("key2") != nullptr) {
        std::cout << "Test failed: key2 was not removed" << std::endl;
        return 1;
    }

    std::cout << "All tests passed!" << std::endl;
    return 0;
}