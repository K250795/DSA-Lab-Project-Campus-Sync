#include "PrefixTrie.h"
#include <cctype>    // std::tolower, std::isalpha, std::isdigit, std::isspace

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
        int idx = charToIndex(c);
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
        int idx = charToIndex(c);
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
        int idx = charToIndex(c);
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

// --------------------------------------------------------------- normalize ---
// Lowercase letters, keep digits and hyphens, collapse any whitespace run into
// a single space, trim both ends, and drop all other punctuation.
// e.g. "  LLC-1 " -> "llc-1", "Lab.10" -> "lab10"
std::string PrefixTrie::normalize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    bool pendingSpace = false;   // saw whitespace since the last kept char

    for (char c : s) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isspace(uc)) {
            pendingSpace = true;
            continue;
        }
        if (!std::isalpha(uc) && !std::isdigit(uc) && c != '-')
            continue;                            // other punctuation is ignored

        if (pendingSpace && !out.empty())        // no leading space
            out += ' ';
        pendingSpace = false;
        out += static_cast<char>(std::tolower(uc));
    }
    return out;                                  // trailing space never added
}

// ------------------------------------------------------------- charToIndex ---
// 'a'-'z' -> 0-25, '0'-'9' -> 26-35, ' ' -> 36, '-' -> 37.
// Input is already normalized.
int PrefixTrie::charToIndex(char c) {
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    if (c == '-') return 37;
    return 36;                                   // the only other char is ' '
}

// ------------------------------------------------------------- indexToChar ---
char PrefixTrie::indexToChar(int idx) {
    if (idx < 26) return static_cast<char>('a' + idx);
    if (idx < 36) return static_cast<char>('0' + (idx - 26));
    if (idx == 37) return '-';
    return ' ';
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
            current += indexToChar(i);
            collectAll(node->children[i], current, results);
            current.pop_back();
        }
    }
}
