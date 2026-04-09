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
    uint64_t ticks;
    // 'rdtime' est généralement autorisé là où 'rdcycle' est bloqué
    asm volatile ("rdtime %0" : "=r" (ticks));
    return ticks;
}

/*
 * us_to_ticks - Convert microseconds to ticks
 * us: microseconds
 * cpu_freq_mhz: CPU frequency in MHz (e.g., 1000 for 1GHz)
 */
static inline uint64_t us_to_ticks(uint64_t us, uint64_t cpu_freq_mhz) 
{
    return us * cpu_freq_mhz;
}

/*
 * ticks_to_us - Convert ticks to microseconds
 * us: microseconds
 * cpu_freq_mhz: CPU frequency in MHz (e.g., 1000 for 1GHz)
 */
static inline double ticks_to_us(uint64_t ticks, uint64_t cpu_freq_mhz) 
{
    return (double)ticks / (double)cpu_freq_mhz;
}
#endif /* !RISCV_TIME_H */