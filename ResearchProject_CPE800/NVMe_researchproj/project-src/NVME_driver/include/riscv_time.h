#ifndef RISCV_TIME_H
#define RISCV_TIME_H

/*
 * get_riscv_tick - Read the 64-bit cycle counter (CSR)
 * * Note: On 32-bit RISC-V, you'd need to read 'cycleh' and 'cycle' separately.
 * On 64-bit (Aarch64/RV64), a single rdcycle instruction is sufficient.
 */
static inline uint64_t get_riscv_tick(void) 
{
    uint64_t cycles;
    // Reading the 'cycle' Control and Status Register (CSR)
    __asm__ volatile ("rdcycle %0" : "=r" (cycles));
    return cycles;
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