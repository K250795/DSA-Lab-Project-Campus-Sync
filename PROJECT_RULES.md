# Campus-Sync Project Rules

- **Compiler & Environment:** Use C++23 (`g++ -std=c++23 -Wall -Iinclude ...`). When compiling in Windows terminals, list `.cpp` files explicitly rather than relying on `src/*.cpp` wildcards. Use standard C++ only—no external third-party libraries.
- **Manual DSA Requirement:** Implement all core data structures from scratch:
  - **Node-based structures (Doubly Linked List, LIFO Undo/Redo Stack, Prefix Trie):** Build manually using raw pointers (`new`/`delete`) with proper destructors and deep-copy constructors/assignment operators (Rule of Three/Five) to prevent memory leaks and dangling pointers.
  - **Array-based structures (Min-Heap, Max-Heap, Weighted Adjacency List):** Build the structure and algorithms (`heapifyUp`, `heapifyDown`, index math, graph traversal) manually, using `std::vector` strictly as the underlying dynamic array storage.
- **Forbidden STL Shortcuts:** Do NOT use `std::list`, `std::stack`, `std::priority_queue`, `std::deque`, `std::push_heap`, `std::pop_heap`, or `std::make_heap`.
- **Allowed Standard Library:** You may use `std::string`, `std::vector` (as backing array storage, for CSV parsing, and for returning paths/results), `std::unordered_map` (for the Bunker Radar room lookup table), `<fstream>`, and `<sstream>`.
- **OOP Hierarchy:** Implement an abstract base class `CampusNode` with derived classes `AcademicRoom` (including `bool schedule[8]`), `ServiceFacility` (queue and processing time attributes), and `TransitNode`.
- **Folder & Code Architecture:** Place all declarations in `include/*.h` and implementations in `src/*.cpp`.
- **Decoupled UI:** Keep all `std::cin` and `std::cout` calls strictly inside `main.cpp`. Data structure and algorithm methods must accept parameters and return values/structs/vectors, never print directly to the console.