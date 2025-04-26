#ifndef SJTU_TRIE_HPP
#define SJTU_TRIE_HPP

#include <algorithm>
#include <cstddef>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sjtu {
    //——————————————————————————————————TrieNode—————————————————————————————————————————————————————————————————————//
    // A TrieNode is a node in a Trie.
    class TrieNode {
    public:
        friend class Trie;

        // Create a TrieNode with no children.
        TrieNode() = default;

        // Create a TrieNode with some children.
        explicit TrieNode(std::map<char, std::shared_ptr<const TrieNode> > children): children_(std::move(children)) {
        }

        virtual ~TrieNode() = default;

        // Clone returns a copy of this TrieNode. If the TrieNode has a value, the
        // value is copied. The return type of this function is a unique_ptr to a
        // TrieNode.
        // You cannot use the copy constructor to clone the node because it doesn't
        // know whether a `TrieNode` contains a value or not.
        virtual auto Clone() const -> std::unique_ptr<TrieNode> {
            auto clonedNode = std::make_unique<TrieNode>();
            clonedNode->children_ = children_;
            clonedNode->is_value_node_ = is_value_node_;
            return clonedNode;
        }

    protected:
        // A map of children, where the key is the next character in the key, and
        // the value is the next TrieNode.
        std::map<char, std::shared_ptr<const TrieNode> > children_;

        // Indicates if the node is the terminal node.
        bool is_value_node_{false};
    };


    //——————————————————————————————————TrieNodeWithValue————————————————————————————————————————————————————————————//

    // A TrieNodeWithValue is a TrieNode that also has a value of type T associated
    // with it.
    template<class T>
    class TrieNodeWithValue : public TrieNode {
    public:

        friend class Trie;
        // Create a trie node with no children and a value.
        explicit TrieNodeWithValue(std::shared_ptr<T> value)
            : value_(std::move(value)) {
            this->is_value_node_ = true;
        }

        // Create a trie node with children and a value.
        TrieNodeWithValue(std::map<char, std::shared_ptr<const TrieNode> > children,
                          std::shared_ptr<T> value)
            : TrieNode(std::move(children)), value_(std::move(value)) {
            this->is_value_node_ = true;
        }

        // Override the Clone method to also clone the value.
        auto Clone() const -> std::unique_ptr<TrieNode> override {
            auto clonedNode = std::make_unique<TrieNodeWithValue<T> >(value_);
            clonedNode->children_ = this->children_;
            clonedNode->is_value_node_ = this->is_value_node_;
            return clonedNode;
        }

    protected:
        // The value associated with this trie node.
        std::shared_ptr<T> value_;
    };

    //——————————————————————————————————Trie—————————————————————————————————————————————————————————————————————————//

    // A Trie is a data structure that maps strings to values of type T. All
    // operations on a Trie should not modify the trie itself. It should reuse the
    // existing nodes as much as possible, and create new nodes to represent the new
    // trie.
    class Trie {
    private:
        // The root of the trie.
        std::shared_ptr<const TrieNode> root_{nullptr};

        // Create a new trie with the given root.
        explicit Trie(std::shared_ptr<const TrieNode> root): root_(std::move(root)) {
        }

    public:
        // Create an empty trie.
        Trie() = default;

        bool operator==(const Trie &other) const {
            if (root_ == other.root_) {
                return true;
            } else {
                return false;
            }
        }

        // Get the value associated with the given key.
        // 1. If the key is not in the trie, return nullptr.
        // 2. If the key is in the trie but the type is mismatched, return nullptr.
        // 3. Otherwise, return the value.
        template<class T>
        auto Get(std::string_view key) const -> const T * {
            const TrieNode *cur = root_.get();
            if (cur == nullptr) {
                return nullptr;
            }
            for (auto c: key) {
                auto it = cur->children_.find(c);
                if (it == cur->children_.end()) {
                    return nullptr;
                }
                cur = it->second.get();
            }

            if (!cur->is_value_node_) {
                return nullptr;
            }
            const auto *value_node = dynamic_cast<const TrieNodeWithValue<T> *>(cur);
            if (!value_node) {
                return nullptr;
            }
            return value_node->value_.get();
        }

        // Put a new key-value pair into the trie. If the key already exists,
        // overwrite the value. Returns the new trie.
        template<class T>
        auto Put(std::string_view key, T value) const -> Trie {
            if (root_ == nullptr) {
                // 处理空树的情况
                std::shared_ptr<TrieNode> cur = std::make_shared<TrieNode>();
                //记录路径
                std::stack<std::pair<std::shared_ptr<TrieNode>, char> > path;

                for (auto c: key) {
                    path.push({cur, c});
                    auto newNode = std::make_shared<TrieNode>();
                    cur->children_[c] = newNode;
                    cur = newNode;
                }

                // 创建带值的节点，继承cur的children
                auto valueNode = std::make_shared<TrieNodeWithValue<T> >(
                    cur->children_, std::make_shared<T>(std::move(value)));
                cur = valueNode;

                while (!path.empty()) {
                    auto [parent, c] = path.top();
                    path.pop();
                    parent->children_[c] = cur;
                    cur = parent;
                }

                return Trie(cur);
            }

            std::shared_ptr<TrieNode> cur = std::shared_ptr<TrieNode>(root_->Clone());
            std::stack<std::pair<std::shared_ptr<TrieNode>, char> > path;

            for (auto c: key) {
                path.push({cur, c});
                auto it = cur->children_.find(c);
                if (it == cur->children_.end()) {
                    // 如果子节点不存在，则创建一个新的子节点
                    auto newNode = std::make_shared<TrieNode>();
                    cur->children_[c] = newNode;
                    cur = newNode;
                } else {
                    // 克隆子节点并更新路径
                    auto clonedChild = it->second->Clone();
                    cur->children_[c] = std::shared_ptr<const TrieNode>(std::move(clonedChild));
                    cur = std::const_pointer_cast<TrieNode>(cur->children_[c]);
                }
            }

            // 创建带值的节点，继承原有子节点
            auto valueNode = std::make_shared<TrieNodeWithValue<T> >(cur->children_,
                                                                     std::make_shared<T>(std::move(value)));
            cur = valueNode;

            // 反向更新路径中的节点
            while (!path.empty()) {
                auto [parent, c] = path.top();
                path.pop();
                parent->children_[c] = cur;
                cur = parent;
            }

            return Trie(cur);
        }


        // Remove the key from the trie. If the key does not exist, return the
        // original trie. Otherwise, returns the new trie.
        auto Remove(std::string_view key) const -> Trie {
            if (!root_) {
                return *this;
            }

            std::shared_ptr<TrieNode> new_root = std::shared_ptr<TrieNode>(root_->Clone());
            std::stack<std::pair<std::shared_ptr<TrieNode>, char> > path;

            std::shared_ptr<TrieNode> current = new_root;

            // 遍历键的每个字符
            for (char c: key) {
                auto it = current->children_.find(c);
                if (it == current->children_.end()) {
                    return *this;
                }

                auto cloned_child = it->second->Clone();
                current->children_[c] = std::shared_ptr<const TrieNode>(std::move(cloned_child));
                path.push({current, c});
                current = std::const_pointer_cast<TrieNode>(current->children_[c]);
            }

            // 若当前节点不是值节点，无需删除
            if (!(current->is_value_node_)) {
                return *this;
            }

            current->is_value_node_ = false;
            current = std::dynamic_pointer_cast<TrieNode>(current);


            // 反向遍历
            while (!path.empty()) {
                auto [parent, c] = path.top();
                path.pop();
                if (current->children_.empty() && !current->is_value_node_) {
                    parent->children_.erase(c);
                }
                current = parent; // 回溯
            }

            if (new_root->children_.empty() && !new_root->is_value_node_) {
                return Trie();
            }

            return Trie(new_root);
        }
    };


    //——————————————————————————————————ValueGuard————————————————————————————————————————————————————————————————————//

    // This class is used to guard the value returned by the trie. It holds a
    // reference to the root so that the reference to the value will not be
    // invalidated.
    template<class T>
    class ValueGuard {
    public:
        ValueGuard(Trie root, const T &value)
            : root_(std::move(root)), value_(value) {
        }

        auto operator*() const -> const T & { return value_; }

    private:
        Trie root_;
        const T &value_;
    };


    //——————————————————————————————————TrieStore—————————————————————————————————————————————————————————————————————//

    // This class is a thread-safe wrapper around the Trie class. It provides a
    // simple interface for accessing the trie. It should allow concurrent reads and
    // a single write operation at the same time.
    class TrieStore {
    public:
        // This function returns a ValueGuard object that holds a reference to the
        // value in the trie of the given version (default: newest version). If the
        // key does not exist in the trie, it will return std::nullopt.
        //Get函数是一个模板函数，接受一个键（key）和一个版本号（version），返回一个std::optional<ValueGuard<T>>对象。
        template<class T>
        auto Get(std::string_view key, size_t version = -1) -> std::optional<ValueGuard<T> > {
            size_t target_version;
            Trie target_trie; {
                //锁定快照：使用std::shared_lock<std::shared_mutex>锁定snapshots_lock_，
                //以确保在读取快照时不会被其他线程修改。
                std::shared_lock<std::shared_mutex> lock(snapshots_lock_);
                //若版本号为-1，给最新版，否则给传入的版本
                if (version == static_cast<size_t>(-1)) {
                    if (snapshots_.empty()) return std::nullopt;
                    target_version = snapshots_.size() - 1;
                } else {
                    target_version = version;
                }
                if (target_version >= snapshots_.size()) {
                    return std::nullopt;
                }
                target_trie = snapshots_[target_version];
            }
            const T *value = target_trie.Get<T>(key);
            if (!value) return std::nullopt;
            return ValueGuard<T>(target_trie, *value);
        }

        // This function will insert the key-value pair into the trie. If the key
        // already exists in the trie, it will overwrite the value return the
        // version number after operation Hint: new version should only be visible
        // after the operation is committed(completed)
        //lockguard和uniquelock都是不需要释放的，使用RAII机制，作用于结束后会自动释放
        template<class T>
        size_t Put(std::string_view key, T value) {
            std::lock_guard<std::mutex> lock(write_lock_);

            const Trie &current_trie = snapshots_.back();
            Trie new_trie = current_trie.Put<T>(key, std::move(value)); {
                std::unique_lock<std::shared_mutex> snapshot_lock(snapshots_lock_);
                snapshots_.push_back(std::move(new_trie));
            }
            return snapshots_.size() - 1;
        }

        // This function will remove the key-value pair from the trie.
        // return the version number after operation
        // if the key does not exist, version number should not be increased
        size_t Remove(std::string_view key) {
            std::lock_guard<std::mutex> lock(write_lock_);
            const Trie &current_trie = snapshots_.back();
            Trie new_trie = current_trie.Remove(key);
            if (new_trie == current_trie) {
                return snapshots_.size() - 1; // 无变化
            } {
                std::unique_lock<std::shared_mutex> snapshot_lock(snapshots_lock_);
                snapshots_.push_back(std::move(new_trie));
            }
            return snapshots_.size() - 1;
        }

        // This function return the newest version number
        size_t get_version() {
            std::shared_lock<std::shared_mutex> lock(snapshots_lock_);
            return snapshots_.empty() ? 0 : snapshots_.size() - 1;
        }

    private:
        // This mutex sequences all writes operations and allows only one write
        // operation at a time. Concurrent modifications should have the effect of
        // applying them in some sequential order
        std::mutex write_lock_; //互斥锁，用于单一的增删
        std::shared_mutex snapshots_lock_; //共享互斥锁，允许多线程同时读，但在写的时候独占锁

        // Stores all historical versions of trie
        // version number ranges from [0, snapshots_.size())
        std::vector<Trie> snapshots_{1};
        //保存历史版本的trie，每次增删创建一个新的trie到这里面，版本号的范围是[0, snapshots_.size())
    };
} // namespace sjtu

#endif  // SJTU_TRIE_HPP
