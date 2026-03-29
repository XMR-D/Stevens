#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>
#include <stdbool.h>

/* Benchmark Context: Stores the state of the random generator */
typedef struct {
    uint32_t max_requests;    // N requests limit
    uint32_t current_count;   // Current progress
    uint32_t seed;            // Seed for reproducibility (rand_r)
    uint8_t  read_ratio;      // Percentage of Reads (e.g., 70 for 70/30 R/W)
} rnd_bench_ctx_t;

/* Benchmark Request: sent to the Dispatcher */
typedef struct {
    uint64_t slba;              // Starting LBA
    uint32_t nsid;              // Namespace ID
    uint16_t nlb;               // Number of Blocks
    uint8_t  opc;               // Opcode (0x01: Write, 0x02: Read)
    uint64_t latency_budget_us; // Budget in microseconds
    uint64_t timestamp_submit;  // Start cycle (submission time)
} bench_req_t;


/* Function pointer type for the provider */
typedef bool (*bench_provider_t)(void *ctx, bench_req_t *out_req);

#endif