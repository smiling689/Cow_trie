### Copy-on-Write Trie with Concurrency

> Acknowledgement: CMU15445 Database Spring 2023 Project 0 'Cpp Primer'

In this project, you will implement a key-value store backed by a copy-on-write trie. Tries are efficient ordered-tree data structures for retrieving a value for a given key. To simplify the explanation, we will assume that the keys are variable-length strings, but in practice they can be any arbitrary type.

Each node in a trie can have multiple child nodes representing different possible next characters.

The key-value store you will implement can store string keys mapped to values of any type. The value of a key is stored in the node representing the last character of that key (aka terminal node). For example, consider inserting kv pairs ("ab", 1) and ("ac", "val") into the trie.

<img src="https://notes.sjtu.edu.cn/uploads/upload_b6e9036083fbe8202e19f9d8535e1d81.png" width="300">


The two keys share the same parent node. The value 1 corresponding to key "ab" is stored in the left child, and the value "val" corresponding to key "ac" is stored in the right node.

#### Task1 : Copy-On-Write Trie

In this task, you will need to modify trie.h and trie.cpp to implement a copy-on-write trie. In a copy-on-write trie, operations do not directly modify the nodes of the original trie. Instead, new nodes are created for modified data, and a new root is returned for the newly-modified trie. Copy-on-write enables us to access the trie after each operation at any time with minimum overhead. Consider inserting ("ad", 2) in the above example. We create a new Node2 by reusing two of the child nodes from the original tree, and creating a new value node 2. (See figure below)

<img src="https://notes.sjtu.edu.cn/uploads/upload_a6b0c1197d5b95610b755fc62b51d66b.png" width="500">


If we then insert ("b", 3), we will create a new root, a new node and reuse the previous nodes. In this way, we can get the content of the trie before and after each insertion operation. As long as we have the root object (Trie class), we can access the data inside the trie at that time. (See figure below)

<img src="https://notes.sjtu.edu.cn/uploads/upload_4a69483f918f61563e53641c1e573ac6.png" width="500">



One more example: if we then insert ("a", "abc") and remove ("ab", 1), we can get the below trie. Note that parent nodes can have values, and you will need to purge all unnecessary nodes after removal.

<img src="https://notes.sjtu.edu.cn/uploads/upload_a8b9e344d16516dbcff8586e5db0e255.png" width="400">


Your trie should support 3 operations:

1. Get(key): Get the value corresponding to the key.
2. Put(key, value): Set the corresponding value to the key. Overwrite existing value if the key already exists. Note that the type of the value might be non-copyable (i.e., `std::unique_ptr<int>`). This method returns a new trie.
3. Delete(key): Delete the value for the key. This method returns a new trie.
None of these operations should be performed directly on the trie itself. You should create new trie nodes and reuse existing ones as much as possible.
4. `operator==`: Actually it is not necessary. If you don't need this, just comment out.

To create a new node, you should use the Clone function on the TrieNode class. To reuse an existing node in the new trie, you can copy `std::shared_ptr<TrieNode>`: copying a shared pointer doesn’t copy the underlying data. You should not manually allocate memory by using new and delete in this project. `std::shared_ptr` will deallocate the object when no one has a reference to the underlying object.

For the full specifications of these operations, please refer to the comments in the starter code.
    

#### Task2 : Concurrent Key-Value Store

After you have a copy-on-write trie which can be used in a single-thread environment, implement a concurrent key-value store for a multithreaded environment. In this task, you will need to modify trie/src.hpp. This key-value store also supports 3 operations:

1. Get(key) returns the value.
2. Put(key, value). No return value.
3. Delete(key). No return value.

For the original Trie class, everytime we modify the trie, we need to get the new root to access the new content. But for the concurrent key-value store, the put and delete methods do not have a return value. This requires you to use concurrency primitives to synchronize reads and writes so that no data is lost through the process.

Your concurrent key-value store should concurrently serve multiple readers and a single writer. That is to say, when someone is modifying the trie, reads can still be performed on the old root. When someone is reading, writes can still be performed without waiting for reads.

Also, if we get a reference to a value from the trie, we should be able to access it no matter how we modify the trie. The Get function from Trie only returns a pointer. If the trie node storing this value has been removed, the pointer will be dangling. Therefore, in TrieStore, we return a ValueGuard which stores both a reference to the value and the TrieNode corresponding to the root of the trie structure, so that the value can be accessed as we store the ValueGuard.

To achieve this, we have provided you with the pseudo code for `TrieStore::Get` in src.hpp. Please read it carefully and think of how to implement `TrieStore::Put` and `TrieStore::Remove`.


#### Advice on Concurrency :

1. `std::vector` does not provide built-in thread-safety mechanisms.
2. The granularity of the lock should be carefully considered to prevent the program from effectively becoming single-threaded.
