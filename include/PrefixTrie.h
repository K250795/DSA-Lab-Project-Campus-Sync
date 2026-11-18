#pragma once
#include <string>
#include <vector>
#include <utility>   // std::pair

// Fixed alphabet size for a-z (all keys are lowercased before use).
static constexpr int ALPHA_SIZE = 26;

// A single node in the prefix trie.
struct TrieNode {
    TrieNode* children[ALPHA_SIZE];
    bool      isEnd;      // true  → a complete location string ends here
    int       nodeId;     // the campus-node ID stored at this endpoint (-1 if !isEnd)

    TrieNode() : isEnd(false), nodeId(-1) {
        for (int i = 0; i < ALPHA_SIZE; ++i)
            children[i] = nullptr;
    }
};

// Pointer-based Prefix Trie mapping location name strings (case-insensitive)
// to integer campus-node IDs.
//
// Rules:
//   - Alphabet is a-z only after lowercasing (spaces / digits are ignored).
//   - Destructor uses a post-order recursive traversal to free every node.
//   - No STL containers other than std::string and std::vector used internally.
class PrefixTrie {
public:
    PrefixTrie();
    ~PrefixTrie();

    // Disallow copying (deep copy is non-trivial; add later if required).
    PrefixTrie(const PrefixTrie&)            = delete;
    PrefixTrie& operator=(const PrefixTrie&) = delete;

    // Insert 'name' → 'id' mapping.
    // Existing IDs at the same key are overwritten.
    void insert(const std::string& name, int id);

    // Returns the ID for an exact match, or -1 if not found.
    int searchExact(const std::string& name) const;

    // Returns all (name, id) pairs whose stored key begins with 'prefix'.
    // The returned names are the original lowercased alpha-only keys.
    std::vector<std::pair<std::string, int>> autocomplete(const std::string& prefix) const;

private:
    TrieNode* root_;

    // Convert a string to its lowercase alpha-only form used as the trie key.
    static std::string normalize(const std::string& s);

    // Post-order destructor helper.
    static void destroyNode(TrieNode* node);

    // Recursive DFS helper for autocomplete.
    static void collectAll(TrieNode* node,
                           std::string& current,
                           std::vector<std::pair<std::string, int>>& results);
};
