# Fast Thread-Local Memory Allocator in C++

A small custom memory allocator for C++ that makes STL containers such as
`std::list` faster in multithreaded programs. I built it as a scaled-down
version of a larger lock-free allocator project, to learn how memory
allocation works under the hood.

## Why this project

The default allocator (`new` / `malloc`) is general purpose. When several
threads allocate memory at the same time, they can end up waiting for each
other. This project avoids that by giving every thread its own private memory
pool, so no waiting is needed.

## How it works

The project has four small parts stacked on top of each other:

```
STLAdapter   lets std::list / std::vector use this allocator
    |
Manager      picks the right slab by request size (16/32/64/128 bytes)
    |        one Manager per thread (thread_local), so no locks
Slab         reuses freed chunks using a free list
    |
Arena        one big block of memory, handed out in order
```

### 1. Arena (`include/Arena.hpp`)
Gets one big 64 MB block of memory at the start. To hand out memory, it just
moves a counter forward. Requests are rounded up to a multiple of 8 bytes so
the memory stays aligned. This is very fast because there is nothing to search.

### 2. Slab (`include/Slab.hpp`)
Hands out chunks that are all the same size. When a chunk is freed, it is put
on a linked list of free chunks (the free list). The next allocation takes a
chunk from the front of that list instead of asking the Arena for new memory.
A free chunk stores the pointer to the next free chunk inside itself, so no
extra memory is needed. Both allocate and free take O(1) time.

### 3. Manager (`include/Manager.hpp`)
Looks at the requested size and sends it to the smallest slab that fits
(16, 32, 64 or 128 bytes). Anything larger than 128 bytes uses normal `new`.
The Manager is declared `thread_local`, which means every thread gets its own
Manager with its own Arena and Slabs. Threads never share allocator data, so
no mutex is needed.

### 4. STLAdapter (`include/STLAdapter.hpp`)
A small wrapper that follows the C++ allocator interface, so standard
containers can use it:

```cpp
std::list<int, STLAdapter<int>> fastList;
```

## Project structure

```
my-allocator/
  include/
    Arena.hpp
    Slab.hpp
    Manager.hpp
    STLAdapter.hpp
  main.cpp        benchmark
  test.cpp        correctness test
  README.md
```

## Build and run

Requires a C++17 compiler (tested with g++).

Benchmark:
```
g++ -std=c++17 -O3 -Wall -Iinclude main.cpp -o bench -pthread
./bench
```

Correctness test:
```
g++ -std=c++17 -g -Wall -Iinclude test.cpp -o test -pthread
./test
```

## Usage

```cpp
#include <list>
#include "STLAdapter.hpp"

int main() {
    std::list<int, STLAdapter<int>> fastList;
    fastList.push_back(10);
    fastList.push_back(20);
    return 0;
}
```

## Benchmark

**Test:** 4 threads, and each thread does 200,000 `push_back` calls on a
`std::list<int>` and then sums the list. I ran one warm-up round, then 5
timed rounds, and took the average. Compiled with `-O3`.

| Allocator | Average time | Speedup |
|---|---|---|
| Standard (`new` / `malloc`) | 31.46 ms | 1.0x |
| This allocator | 2.11 ms | 14.9x |

Individual rounds, standard vs custom (ms):

| Round | Standard | Custom |
|---|---|---|
| 1 | 30.67 | 1.89 |
| 2 | 29.14 | 2.39 |
| 3 | 31.56 | 2.29 |
| 4 | 32.97 | 1.98 |
| 5 | 32.95 | 1.99 |

**Machine:** [CPU model], [number of cores] cores, [OS], g++ [version]

The exact speedup depends on the CPU, the operating system and the compiler,
so results on other machines will be different.

## Correctness testing

- The benchmark computes a checksum of each list, and the value matched the
  expected result on every run.
- `test.cpp` allocates 10,000 blocks of different sizes, fills each one with a
  unique value, and then checks that no block was overwritten by another one.
  Result: [All tests passed / paste your result].

## Limitations

- If memory is freed by a different thread than the one that allocated it, it
  goes into the freeing thread's free list instead of going back to the
  original thread.
- Memory is not returned to the operating system until the thread exits.
- There are only four size classes (16, 32, 64, 128 bytes). Larger requests
  use normal `new`.
- If a thread's 64 MB arena is full, it falls back to normal `new`.
- The benchmark is a best case, because it makes many small allocations of the
  same size. Programs with mixed or large allocations will see a smaller gain.
- Only tested on [your OS].

## Possible improvements

- Use `mmap` (Linux/Mac) or `VirtualAlloc` (Windows) instead of `new` for the
  Arena.
- Allocate a new arena block when the first one is full.
- Use a bitwise power-of-two calculation instead of `if` statements to pick
  the size class.
- Benchmark other containers, such as `std::map` and `std::vector`.

## What I learned

- A free list lets a program reuse memory in O(1) time by storing the "next
  free chunk" pointer inside the free chunk itself.
- Using `thread_local` gives each thread its own copy of a variable, which
  removes the need for locks and is the main reason the allocator is fast.
- A fast benchmark result is only meaningful if the results are correct, so I
  added a checksum and an overlap test.
- Speedups depend on the workload and the machine, so the limitations matter
  as much as the numbers.

## Credits

Inspired by the lock-free allocator project at
https://github.com/shervan0714/cpp-fast-allocator. This is a smaller
reimplementation written for learning purposes.
