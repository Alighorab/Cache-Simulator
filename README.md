# Cache Simulator


## Description

- This is part A in the 4th lab of [15-213: Introduction to Computer Systems](https://www.cs.cmu.edu/afs/cs.cmu.edu/academic/class/15213-f15/www/schedule.html) which is a Cache Simulator.
- The simulator in [csim.c](cachelab-handout/csim.c) that takes a [valgrind](https://man7.org/linux/man-pages/man1/valgrind.1.html) memory trace as input, simulates the hit/miss behavior of a cache memory on this trace, and outputs the total number of hits, misses, and evictions.


## My Work

- The simulator simulates the behavior of a cache with arbitrary size and associativity on a valgrind trace file. It uses the LRU (least-recently used) replacement policy when choosing which cache line to evict.
- A [help](cachelab-handout/help) on how to use the simulator `csim` is provided with the files and can be previewed with the command `./csim -h`.
- The simulator is patitioned into three main sections: (1): **parsing** command-line options. (2): **allocating** space for the cache. (3): **simulating** cache and printing the summary. 

## Style

- I followed [suckless](suckless.org/coding_style) coding style.

## Lab Files

- cachelab.pdf: cachelab writup.
- csim.c: my simulator.
- csim-ref: refrernce simulator to compare your results with.
- traces: directory contains valgrind memory trace input files.
- test-sim: program to test the correctness of your simulator.
- coding-style.txt: suckless coding-style in text format.
