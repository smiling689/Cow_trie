#include "../trie/src.hpp"
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>

int main() {
    sjtu::Trie trie;
    std::unordered_map<std::string, std::string> map;

    std::random_device rd;

    std::cout << "Random number: " << rd() << std::endl;

    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dis(1, 10000);

    // Insert random key-value pairs into the Trie and a std::unordered_map
    for (int i = 0; i < 1000; i++) {
        std::string key = "key" + std::to_string(dis(gen));
        std::string value = "value" + std::to_string(dis(gen));

        std::cout << "Inserting " << key << " -> " << value << std::endl;

        trie = trie.Put<std::string>(key, value);
        map[key] = value;
    }

    std::cout << "mapsize : " << map.size() << std::endl;

    // Check that all key-value pairs can be retrieved correctly from the Trie
    for (const auto& pair : map) {
        std::cout << "Retrieving " << pair.first << " -> " << pair.second << std::endl;
        if(trie.Get<std::string>(pair.first) == nullptr){
            std::cout << "Test failed: " << pair.first << " does not exist" << std::endl;
            return 1;
        }
        if (*trie.Get<std::string>(pair.first) != pair.second) {
            std::cout << "Test failed: " << pair.first << " does not return " << pair.second << std::endl;
            return 1;
        }
    }

    std::cout << "All tests passed!" << std::endl;
    return 0;
}