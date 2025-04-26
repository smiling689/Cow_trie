#include <bitset>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <thread>  // NOLINT

#include "../trie/src.hpp"

namespace sjtu {
    using Integer = std::unique_ptr<uint32_t>;

    void TrieTest_NonCopyableTest() {
        auto trie = Trie();
        trie = trie.Put<Integer>("tes", std::make_unique<uint32_t>(233));
        trie = trie.Put<Integer>("te", std::make_unique<uint32_t>(23));
        trie = trie.Put<Integer>("test", std::make_unique<uint32_t>(2333));

        if (**trie.Get<Integer>("te") != 23) {
            std::cout << "Test failed: 'te' does not return 23" << std::endl;
            return;
        }
        if (**trie.Get<Integer>("tes") != 233) {
            std::cout << "Test failed: 'tes' does not return 233" << std::endl;
            return;
        }
        if (**trie.Get<Integer>("test") != 2333) {
            std::cout << "Test failed: 'test' does not return 2333" << std::endl;
            return;
        }

        trie = trie.Remove("te");
        trie = trie.Remove("tes");
        trie = trie.Remove("test");


        if (trie.Get<Integer>("te") != nullptr) {
            std::cout << "Test failed: 'te' still exists after removal" << std::endl;
            return;
        }
        if (trie.Get<Integer>("tes") != nullptr) {
            std::cout << "Test failed: 'tes' still exists after removal" << std::endl;
            return;
        }
        if (trie.Get<Integer>("test") != nullptr) {
            std::cout << "Test failed: 'test' still exists after removal" << std::endl;
            return;
        }

        std::cout << "All tests passed!" << std::endl;
    }
} // namespace sjtu

int main() {
    sjtu::TrieTest_NonCopyableTest();
    return 0;
}
