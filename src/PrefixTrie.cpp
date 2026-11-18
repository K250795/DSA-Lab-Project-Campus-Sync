#include "PrefixTrie.h"
#include <cctype>    // std::tolower, std::isalpha

// ----------------------------------------------------------- constructor ---
PrefixTrie::PrefixTrie()
    : root_(new TrieNode()) {}

// ------------------------------------------------------------- destructor ---
PrefixTrie::~PrefixTrie() {
    destroyNode(root_);
}

// ----------------------------------------------------------------- insert ---
void PrefixTrie::insert(const std::string& name, int id) {
    std::string key = normalize(name);
    TrieNode*   cur = root_;

    for (char c : key) {
        int idx = c - 'a';
        if (cur->children[idx] == nullptr)
            cur->children[idx] = new TrieNode();
        cur = cur->children[idx];
    }

    cur->isEnd  = true;
    cur->nodeId = id;
}

// ----------------------------------------------------------- searchExact ---
int PrefixTrie::searchExact(const std::string& name) const {
    std::string key = normalize(name);
    TrieNode*   cur = root_;

    for (char c : key) {
        int idx = c - 'a';
        if (cur->children[idx] == nullptr)
            return -1;   // prefix not found
        cur = cur->children[idx];
    }

    return cur->isEnd ? cur->nodeId : -1;
}

// ---------------------------------------------------------- autocomplete ---
std::vector<std::pair<std::string, int>>
PrefixTrie::autocomplete(const std::string& prefix) const {
    std::string key = normalize(prefix);
    TrieNode*   cur = root_;

    // Walk to the end of the prefix.
    for (char c : key) {
        int idx = c - 'a';
        if (cur->children[idx] == nullptr)
            return {};   // prefix not in trie
        cur = cur->children[idx];
    }

    // Collect all words reachable from this node.
    std::vector<std::pair<std::string, int>> results;
    std::string accumulated = key;
    collectAll(cur, accumulated, results);
    return results;
}

// ---------------------------------------------------------------- private --

// Convert to lowercase and strip non-alpha characters.
std::string PrefixTrie::normalize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s)
        if (std::isalpha(static_cast<unsigned char>(c)))
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

// Post-order recursive destructor helper: free children before self.
void PrefixTrie::destroyNode(TrieNode* node) {
    if (node == nullptr) return;
    for (int i = 0; i < ALPHA_SIZE; ++i)
        destroyNode(node->children[i]);
    delete node;
}

// DFS: accumulate characters along the path; record (key, id) at each end.
void PrefixTrie::collectAll(TrieNode* node,
                            std::string& current,
                            std::vector<std::pair<std::string, int>>& results) {
    if (node == nullptr) return;

    if (node->isEnd)
        results.emplace_back(current, node->nodeId);

    for (int i = 0; i < ALPHA_SIZE; ++i) {
        if (node->children[i] != nullptr) {
            current += static_cast<char>('a' + i);
            collectAll(node->children[i], current, results);
            current.pop_back();
        }
    }
}
