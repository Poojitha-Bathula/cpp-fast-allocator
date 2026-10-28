#pragma once
#include "Arena.hpp"
#include "slab.hpp"

// Manager: looks at the requested size and picks the right slab
class Manager {
public:
    // "thread_local" means: every thread gets its OWN Manager.
    // That is why no locks are needed.
    static Manager& get() {
        thread_local Manager m;
        return m;
    }

    void* allocate(int n) {
        if (n <= 16)  return slab16.allocate();
        if (n <= 32)  return slab32.allocate();
        if (n <= 64)  return slab64.allocate();
        if (n <= 128) return slab128.allocate();
        return new char[n];               // big request: normal memory
    }

    void deallocate(void* p, int n) {
        if (n <= 16)       slab16.deallocate(p);
        else if (n <= 32)  slab32.deallocate(p);
        else if (n <= 64)  slab64.deallocate(p);
        else if (n <= 128) slab128.deallocate(p);
        else               delete[] (char*)p;
    }

private:
    Manager() {
        slab16.init(&arena, 16);
        slab32.init(&arena, 32);
        slab64.init(&arena, 64);
        slab128.init(&arena, 128);
    }

    Arena arena;
    Slab slab16;
    Slab slab32;
    Slab slab64;
    Slab slab128;
};