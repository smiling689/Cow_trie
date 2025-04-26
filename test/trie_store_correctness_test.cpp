// THIS TEST IS FOR THE CORRECTNESS OF CONCURRENCY
// If your program only goes wrong on this test,
// please consider the thread-safety of std::vector 

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>
#include "../trie/src.hpp"

const int N = 80;
const int INTV = N / 2;

using TrieStore = sjtu::TrieStore;
using std::chrono::duration_cast;

std::atomic<int> RVERSION;  // real_version, std::atomic guarantees

template <class TrieStore>
void ContinuePut(TrieStore& store) {
//    std::cout << "Putting key: " << RVERSION << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::seconds>(end - start);
    while (duration < std::chrono::seconds(200)) {
        store.Put(std::to_string(RVERSION), 86);
        RVERSION++;
        end = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::seconds>(end - start);
    }
}

template <class TrieStore>
void ContinueGet(TrieStore& store, int idx) {
    auto start = std::chrono::high_resolution_clock::now();
    std::this_thread::sleep_for(std::chrono::nanoseconds(400 * idx));
    bool not_found = false;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::seconds>(end - start);
    std::atomic<int> inserted(0);
    while (inserted <= 100) {
        if (store.template Get<int>(std::to_string(inserted.load())) !=
            std::nullopt) {
            inserted++;
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(50));
        // switch the context
    }
    while (duration < std::chrono::seconds(200)) {
        for (int i = 1; i <= 100; i++) {
            not_found |=
                (store.template Get<int>(std::to_string(i)) == std::nullopt);
        }
        if (not_found) {
            std::cerr << "Error: Can't find existed key." << std::endl;
            exit(0);
        }
        end = std::chrono::high_resolution_clock::now();
        duration = duration_cast<std::chrono::seconds>(end - start);
        std::this_thread::sleep_for(std::chrono::nanoseconds(50));
        // switch the context
    }
}

int TestCase() {
    RVERSION = 0;
    TrieStore store;
    std::thread threads[N];
    for (int i = 0; i < INTV; i++) {
        threads[i] = std::thread(ContinueGet<TrieStore>, std::ref(store), i);
    }
    for (int i = INTV; i < N; i++) {
        threads[i] = std::thread(ContinuePut<TrieStore>, std::ref(store));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 1;
}

int main() {
    int result = 0;
    result = TestCase();
    if (result) {
        std::cout << "Testcase passed!" << std::endl;
    } else {
        std::cout << "Testcase Failed!" << std::endl;
    }
    return 0;
}