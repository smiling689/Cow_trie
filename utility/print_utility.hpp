#include <iostream>
#include <string>
#include <vector>
#include "../trie/src.hpp"

// Feel free to design your own print function for testing / debugging purpose
void print_trie(std::shared_ptr<sjtu::TrieNode> trienode,
                std::vector<std::string> path) {
    auto cur = trienode.get();
    for (char c : path.back()) {
        std::cout << c;

    }
}
