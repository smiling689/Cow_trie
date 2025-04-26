#include <atomic>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <thread>  // NOLINT
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "../trie/src.hpp"

using Integer = std::unique_ptr<uint32_t>;

void TrieStoreTest_BasicTest() {
    sjtu::TrieStore store;
    if (store.Get<uint32_t>("233") != std::nullopt) {
        std::cout << "Test failed: Expected std::nullopt" << std::endl;
    }
    store.Put<uint32_t>("233", 2333); {
        auto guard = store.Get<uint32_t>("233");
        if (**guard != 2333) {
            std::cout << "Test failed: Expected 2333" << std::endl;
        }
    }
    store.Remove("233"); {
        auto guard = store.Get<uint32_t>("233");
        if (guard != std::nullopt) {
            std::cout << "Test failed: Expected std::nullopt" << std::endl;
        }
    }
}

void TrieStoreTest_GuardTest() {
    sjtu::TrieStore store;
    if (store.Get<uint32_t>("233") != std::nullopt) {
        std::cout << "Test failed: Expected std::nullopt" << std::endl;
    }

    store.Put<std::string>("233", "2333");
    auto guard = store.Get<std::string>("233");
    if (**guard != "2333") {
        std::cout << "Test failed: Expected 2333" << std::endl;
    }

    store.Remove("233"); {
        auto guard = store.Get<std::string>("233");
        if (guard != std::nullopt) {
            std::cout << "Test failed: Expected std::nullopt" << std::endl;
        }
    }

    if (**guard != "2333") {
        std::cout << "Test failed: Expected 2333" << std::endl;
    }
}

void TrieStoreTest_MixedTest() {
    sjtu::TrieStore store;
    for (uint32_t i = 0; i < 23333; i++) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << i;
        std::string key = ss.str();
        ss.str("");

        ss << "value-" << std::setfill('0') << std::setw(8) << i;
        std::string value = ss.str();
        ss.str("");

        store.Put<std::string>(key, value);
    }
    for (uint32_t i = 0; i < 23333; i += 2) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << i;
        std::string key = ss.str();
        ss.str("");

        ss << "new-value-" << std::setfill('0') << std::setw(8) << i;
        std::string value = ss.str();
        ss.str("");

        store.Put<std::string>(key, value);
    }
    for (uint32_t i = 0; i < 23333; i += 3) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << i;
        std::string key = ss.str();
        ss.str("");

        store.Remove(key);
    }

    // verify final trie
    for (uint32_t i = 0; i < 23333; i++) {
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(5) << i;
        std::string key = ss.str();
        ss.str("");

        if (i % 3 == 0) {
            if (store.Get<std::string>(key) != std::nullopt) {
                std::cout << "Test failed: Expected std::nullopt" << std::endl;
            }
        } else if (i % 2 == 0) {
            ss << "new-value-" << std::setfill('0') << std::setw(8) << i;
            std::string value = ss.str();
            ss.str("");

            auto guard = store.Get<std::string>(key);
            if (**guard != value) {
                std::cout << "Test failed: Expected " << value << std::endl;
            }
            std::cerr << i << std::endl;
        } else {
            ss << "value-" << std::setfill('0') << std::setw(8) << i;
            std::string value = ss.str();
            ss.str("");

            auto guard = store.Get<std::string>(key);
            if (**guard != value) {
                std::cout << "Test failed: Expected " << value << std::endl;
            }
        }

    }
    std::cerr << "end_mix" << std::endl;
}

void TrieStoreTest_MixedConcurrentTest() {
    sjtu::TrieStore store;
    std::vector<std::thread> threads;

    std::cerr << "Starting threads" << std::endl;

    const int keys_per_thread = 10000;

    for (int tid = 0; tid < 4; tid++) {
        threads.push_back(std::thread([&store, tid, keys_per_thread] {
            for (uint32_t i = 0; i < keys_per_thread; i++) {
                std::cerr << "Thread " << i << std::endl;
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(5) << i * 4 + tid;
                std::string key = ss.str();
                ss.str("");

                ss << "value-" << std::setfill('0') << std::setw(8) << i * 4 + tid;
                std::string value = ss.str();
                ss.str("");

                store.Put<std::string>(key, value);
            }
            for (uint32_t i = 0; i < keys_per_thread; i++) {
                std::cerr << "Thread thread" << i << std::endl;
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(5) << i * 4 + tid;
                std::string key = ss.str();
                ss.str("");

                store.Remove(key);
            }
        }));
    }

    for (auto &t: threads) {
        t.join();
    }
}

int main() {
    TrieStoreTest_BasicTest();
    TrieStoreTest_GuardTest();
    TrieStoreTest_MixedTest();
    std::cerr << "All tests passed_mix" << std::endl;
    TrieStoreTest_MixedConcurrentTest();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}
