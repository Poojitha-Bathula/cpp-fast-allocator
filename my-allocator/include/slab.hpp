#pragma once
#include "Arena.hpp"

// A free chunk uses its own space to remember the next free chunk
struct Node {
    Node* next;
};

// Slab: gives out chunks that are all the same size, and reuses freed ones
class Slab {
public:
    Slab() {
        arena = nullptr;
        chunkSize = 16;
        head = nullptr;      // head = first free chunk (nullptr = none)
    }

    void init(Arena* a, int size) {
        arena = a;
        chunkSize = size;
    }

    void* allocate() {
        // if there is a freed chunk, reuse it
        if (head != nullptr) {
            Node* chunk = head;
            head = head->next;        // second chunk becomes the first
            return chunk;
        }

        // otherwise cut a new chunk from the arena
        void* p = arena->allocate(chunkSize);
        if (p == nullptr) {
            p = new char[chunkSize];  // arena is full, use normal memory
        }
        return p;
    }

    void deallocate(void* p) {
        Node* chunk = (Node*)p;   // treat the memory as a Node
        chunk->next = head;       // point it at the old first chunk
        head = chunk;             // it is now the first free chunk
    }

private:
    Arena* arena;
    int chunkSize;
    Node* head;
};