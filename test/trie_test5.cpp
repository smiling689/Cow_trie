#include "../trie/src.hpp"
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>

int main() {
    sjtu::Trie trie;
    std::unordered_map<std::string, std::string> map;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100000);

    // Insert random key-value pairs into the Trie and a std::unordered_map
    // Remove random keys from the Trie and the std::unordered_map
    for (int i = 0; i < 100000; i++) {
        std::string key = "key" + std::to_string(dis(gen));
        std::string value = "value" + std::to_string(dis(gen));
        trie = trie.Put<std::string>(key, value);
        map[key] = value;
        std::string key2 = "key" + std::to_string(dis(gen));
        trie = trie.Remove(key2);
        map.erase(key2);
    }

    // Check that all key-value pairs can be retrieved correctly from the Trie
    for (const auto& pair : map) {
        if(trie.Get<std::string>(pair.first) == nullptr){
            std::cout << "Test failed: " << pair.first << " does not exist" << std::endl;
            return 1;
        }
        if (*trie.Get<std::string>(pair.first) != pair.second) {
            std::cout << "Test failed: " << pair.first << " does not return " << pair.second << std::endl;
            return 1;
        }
    }

    // Remove half of the key-value pairs from the Trie and the map
    int i = 0;
    for (auto it = map.begin(); it != map.end(); ) {
        if (i % 2 == 0) {
            trie = trie.Remove(it->first);
            it = map.erase(it);
        } else {
            ++it;
        }
        ++i;
    }

    // Check that the removed keys no longer exist in the Trie
    for (const auto& pair : map) {
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