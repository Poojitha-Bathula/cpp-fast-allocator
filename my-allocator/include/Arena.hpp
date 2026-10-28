#pragma once

// Arena: one big block of memory. We hand it out piece by piece.
class Arena {
public:
    Arena() {
        totalSize = 64 * 1024 * 1024;    // 64 MB
        used = 0;                        // nothing handed out yet
        block = new char[totalSize];     // get the big block once
    }

    ~Arena() {
        delete[] block;
    }

    void* allocate(int n) {
        // round n up to a multiple of 8
        if (n % 8 != 0) {
            n = n + (8 - n % 8);
        }

        // not enough space left
        if (used + n > totalSize) {
            return nullptr;
        }

        void* result = block + used;     // next free spot
        used = used + n;                 // move the counter forward
        return result;
    }

private:
    char* block;
    int totalSize;
    int used;
};