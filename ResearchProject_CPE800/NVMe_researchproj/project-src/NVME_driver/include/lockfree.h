#ifndef LOCKFREE_H
#define LOCKFREE_H

#define MEM_FENCE(opA, opB) do { \
    __asm__ volatile ("fence " #opA ", " #opB ::: "memory"); \
} while (0)

#endif