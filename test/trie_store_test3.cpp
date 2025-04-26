#include "../trie/src.hpp"
#include <iostream>
#include <thread>
#include <optional>
#include <vector>
#include <string>
#include <atomic>
#include <cassert>

const int N = 100;
std::atomic<int> failedSearches{0};
std::atomic<int> failedDeletes{0};

void concurrentPut(sjtu::TrieStore& trie, int start, int end) 
{
    for (int i = start; i < end; ++i) 
    {
        trie.Put<std::string>("key" + std::to_string(i), "value" + std::to_string(i));
    }
}

void concurrentSearch(sjtu::TrieStore& trie, int start, int end, bool button) 
{
    for (int i = start; i < end; ++i) 
    {
        auto result = trie.Get<std::string>("key" + std::to_string(i));
        if (button && (!result.has_value() || (*(result.value()) != ("value" + std::to_string(i))))) 
        {
          if(!result.has_value())
          {
            std::cerr << "Failed search for key: " << "key" + std::to_string(i) << std::endl;
          }
          else
          {
            std::cerr << "Failed search for key: " << "key" + std::to_string(i) << " with value: " << *(result.value()) << std::endl;
          }
          failedSearches++;
        }
    }
}

void concurrentDelete(sjtu::TrieStore& trie, int start, int end) 
{
    for (int i = start; i < end; ++i) 
    {
        trie.Remove("key" + std::to_string(i));
        if (trie.Get<std::string>("key" + std::to_string(i)).has_value()) 
        {
            failedDeletes++;
        }
    }
}

void testConcurrency() 
{
    sjtu::TrieStore trie;
    std::vector<std::thread> threads;

    // Split workload among threads for concurrent Puts
    int chunkSize = N / 10;
    for (int i = 0; i < N; i += chunkSize) 
    {
        threads.emplace_back(concurrentPut, std::ref(trie), i, std::min(N, i + chunkSize));
    }
    for (auto& thread : threads) 
    {
        thread.join();
    }
    threads.clear();

    // Concurrently search and delete elements in overlapping regions
    for (int i = 0; i < N; i += chunkSize) 
    {
        threads.emplace_back(concurrentSearch, std::ref(trie), i, std::min(N, i + chunkSize), false);
        // Start deleting halfway through the search range to increase concurrency complexity
        if (i % (2 * chunkSize) == 0)  //i | 20
        {
            threads.emplace_back(concurrentDelete, std::ref(trie), i, std::min(N, i + 2 * chunkSize));
        }
    }

    for (auto& thread : threads) 
    {
        thread.join();
    }
    threads.clear();

    // Final consistency check
    for (int i = 0; i < N; ++i) 
    {
        //std::cout << "i is : " << i << std::endl; 
        if (trie.Get<std::string>("key" + std::to_string(i)).has_value()) 
        {
            std::cerr << "Consistency test failed for key: " << "key" + std::to_string(i) << std::endl;
        }
    }

    if(failedSearches.load() != 0)
    {
        std::cerr << "Failed searches: " << failedSearches.load() << std::endl;
    }
    if(failedDeletes.load() != 0)
    {
        std::cerr << "Failed deletes: " << failedDeletes.load() << std::endl;
    }
}

int main() 
{
    testConcurrency();
    if (failedSearches == 0 && failedDeletes == 0)
    {
        std::cout << "All tests passed!" << std::endl;
    } 
    else 
    {
        std::cerr << "Tests failed with " << failedSearches << " failed searches and " << failedDeletes << " failed deletes." << std::endl;
    }
    return 0;
}
