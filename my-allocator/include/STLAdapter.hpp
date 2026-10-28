#pragma once
#include <cstddef>
#include "manager.hpp"

template <class T>
struct STLAdapter {
    typedef T value_type;

    STLAdapter() {}

    template <class U>
    STLAdapter(const STLAdapter<U>& other) {}

    T* allocate(std::size_t n) {
        return (T*)Manager::get().allocate((int)(n * sizeof(T)));
    }

    void deallocate(T* p, std::size_t n) {
        Manager::get().deallocate(p, (int)(n * sizeof(T)));
    }
};

template <class T, class U>
bool operator==(const STLAdapter<T>&, const STLAdapter<U>&) { return true; }

template <class T, class U>
bool operator!=(const STLAdapter<T>&, const STLAdapter<U>&) { return false; }