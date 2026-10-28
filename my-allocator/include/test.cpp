#include <cassert>
#include <iostream>
#include <vector>
#include "manager.hpp"

int main() {
    Manager& m = Manager::get();
    std::vector<std::pair<unsigned char*, size_t>> blocks;

    // allocate 10,000 blocks of mixed sizes, fill each with a unique byte
    for (int i = 0; i < 10000; ++i) {
        size_t n = 1 + (i * 37) % 200;
        auto* p = static_cast<unsigned char*>(m.allocate(n));
        assert(p);
        for (size_t j = 0; j < n; ++j) p[j] = (unsigned char)(i & 0xFF);
        blocks.push_back({p, n});
    }
    // verify nothing got overwritten by a neighbor
    for (int i = 0; i < 10000; ++i)
        for (size_t j = 0; j < blocks[i].second; ++j)
            assert(blocks[i].first[j] == (unsigned char)(i & 0xFF));

    for (auto& b : blocks) m.deallocate(b.first, b.second);
    std::cout << "All tests passed\n";
}