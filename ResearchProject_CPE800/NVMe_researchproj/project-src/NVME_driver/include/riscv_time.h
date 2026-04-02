#ifndef RISCV_TIME_H
#define RISCV_TIME_H

#include <time.h>
/*
 * get_riscv_tick - Read the 64-bit cycle counter (CSR)
 * * Note: On 32-bit RISC-V, you'd need to read 'cycleh' and 'cycle' separately.
 * On 64-bit (Aarch64/RV64), a single rdcycle instruction is sufficient.
 */
#include <time.h>

static inline uint64_t get_riscv_tick(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    /* Convert to a synthetic tick (nanoseconds) */
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

/*
 * us_to_ticks - Convert microseconds to CPU cycles
 * us: microseconds
 * cpu_freq_mhz: CPU frequency in MHz (e.g., 1000 for 1GHz)
 */
static inline uint64_t us_to_ticks(uint64_t us, uint64_t cpu_freq_mhz) 
{
    return us * cpu_freq_mhz;
}

#endif /* !RISCV_TIME_H */