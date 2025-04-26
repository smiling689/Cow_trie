#include "../trie/src.hpp"
#include <iostream>
#include <thread>
#include <optional>
#include <vector>
#include <string>

const int N = 10000;

void concurrentPut(sjtu::TrieStore& trie, const std::string& key) 
{
    trie.Put<std::string>(key, "value" + key);
    std::cerr << "doing " << key << " " << key << std::endl;

}

void concurrentSearch(sjtu::TrieStore& trie, const std::string& key, bool expected) 
{
    if (trie.Get<std::string>(key).has_value() != expected) 
    {
        std::cerr << "Search test failed for key: " << key << std::endl;
    }
}

void concurrentDelete(sjtu::TrieStore& trie, const std::string& key) 
{
    trie.Remove(key);
}

void testConcurrency() 
{
    sjtu::TrieStore trie;
    std::vector<std::thread> threads;

    // Concurrently Put elements
    for (int i = 0; i < N; ++i) 
    {
        threads.emplace_back(concurrentPut, std::ref(trie), "key" + std::to_string(i));
    }

    // Wait for all Puts to complete
    for (auto& thread : threads) 
    {
        thread.join();
    }
    threads.clear();

    // Concurrently search for elements
    for (int i = 0; i < N; ++i) 
    {
        threads.emplace_back(concurrentSearch, std::ref(trie), "key" + std::to_string(i), true);

        std::cerr << "do " << std::to_string(i) << std::endl;
    }

    // Wait for all searches to complete
    for (auto& thread : threads) 
    {
        thread.join();
    }
    threads.clear();

    // Concurrently delete elements
    for (int i = 0; i < N; ++i) 
    {
        threads.emplace_back(concurrentDelete, std::ref(trie), "key" + std::to_string(i));

        std::cerr << "do do " << std::to_string(i) << std::endl;
    }

    // Wait for all deletions to complete
    for (auto& thread : threads) 
    {
        thread.join();
    }
    threads.clear();

    // Final check - all should be deleted
    for (int i = 0; i < N; ++i) 
    {
        if (trie.Get<std::string>("key" + std::to_string(i))) 
        {
            std::cerr << "Deletion test failed for key: " << "key" + std::to_string(i) << std::endl;
        }
    }
}

int main() {
    testConcurrency();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
