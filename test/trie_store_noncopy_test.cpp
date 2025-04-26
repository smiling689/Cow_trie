#include <bitset>
#include <cassert> 
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <thread>  // NOLINT
#include <utility>
#include "../trie/src.hpp"

//This is the hardest part
//The TrieStore class should be able to store non-copyable objects

using Integer = std::unique_ptr<uint32_t>;

namespace sjtu {

/// A special type that will block the move constructor and move assignment operator. Used in TrieStore tests.
class MoveBlocked {
 public:
  explicit MoveBlocked(std::future<int> wait) : wait_(std::move(wait)) {}

  MoveBlocked(const MoveBlocked &) = delete;
  MoveBlocked(MoveBlocked &&that) noexcept {
    if (!that.waited_) {
      that.wait_.get();
    }
    that.waited_ = waited_ = true;
  }

  auto operator=(const MoveBlocked &) -> MoveBlocked & = delete;
  auto operator=(MoveBlocked &&that) noexcept -> MoveBlocked & {
    if (!that.waited_) {
      that.wait_.get();
    }
    that.waited_ = waited_ = true;
    return *this;
  }

  bool waited_{false};
  std::future<int> wait_;
};

}  // namespace sjtu

void TrieStoreTest_NonCopyableTest() {
  sjtu::TrieStore store;
  store.Put<Integer>("tes", std::make_unique<uint32_t>(233));
  store.Put<Integer>("te", std::make_unique<uint32_t>(23));
  store.Put<Integer>("test", std::make_unique<uint32_t>(2333));
  assert(***store.Get<Integer>("te") == 23);
  assert(***store.Get<Integer>("tes") == 233);
  assert(***store.Get<Integer>("test") == 2333);
  store.Remove("te");
  store.Remove("tes");
  store.Remove("test");
  assert(store.Get<Integer>("te") == std::nullopt);
  assert(store.Get<Integer>("tes") == std::nullopt);
  assert(store.Get<Integer>("test") == std::nullopt);
}

void TrieStoreTest_ReadWriteTest() {
  sjtu::TrieStore store;
  store.Put<uint32_t>("a", 1);
  store.Put<uint32_t>("b", 2);
  store.Put<uint32_t>("c", 3);
  std::promise<int> x;

  std::cerr << "[0] begin" << std::endl;

  std::thread t([&store, &x] { store.Put<sjtu::MoveBlocked>("d", sjtu::MoveBlocked(x.get_future())); });

  std::cerr << "[1] thread spawn" << std::endl;

  // Loop for enough time to ensure that the thread is blocked on the promise.
  for (int i = 0; i < 100000; i++) {
    //std::cout << "i : " << i << std::endl;
    {
      auto guard = store.Get<uint32_t>("a");
      assert(**guard == 1);
    }
    {
      auto guard = store.Get<uint32_t>("b");
      assert(**guard == 2);
    }
    {
      auto guard = store.Get<uint32_t>("c");
      assert(**guard == 3);
    }
  }

  std::cerr << "[2] read done" << std::endl;

  x.set_value(233);

  t.join();

  std::cerr << "[3] write complete" << std::endl;

  assert(store.Get<sjtu::MoveBlocked>("d") != std::nullopt);
}


int main() {
  TrieStoreTest_NonCopyableTest();
  TrieStoreTest_ReadWriteTest();
  return 0;
}
