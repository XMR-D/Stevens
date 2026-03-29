#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "benchmark.h"
#include "riscv_time.h"

/*
 * random_workload_provider - Benchmark request generator
 * ctx: Pointer to rnd_bench_ctx_t (opaque context)
 * out_req: Pointer to the bench_req_t structure to be filled
 * Returns true if a request was successfully generated, 
 * false if the max_requests limit has been reached.
 */
bool random_workload_provider(void *ctx, bench_req_t *out_req) 
{
    rnd_bench_ctx_t *b_ctx = (rnd_bench_ctx_t *)ctx;

    /* Stop generation if we reached the benchmark request limit */
    if (b_ctx->current_count >= b_ctx->max_requests) {
        return false; 
    }

    /* Record the current CPU cycle as the submission timestamp */
    uint64_t now = get_riscv_tick();
    out_req->timestamp_submit = now;

    /**
     * Deadline Distribution Logic:
     * - 10% Real-Time/Critical: 500us budget
     * - 20% Interactive: 2000us (2ms) budget
     * - 70% Best Effort: 10000us (10ms) budget
     */
    int r = rand_r(&b_ctx->seed) % 100;
    
    if (r < 10) {
        out_req->latency_budget_us = 500; 
    } else if (r < 30) {
        out_req->latency_budget_us = 2000;
    } else {
        out_req->latency_budget_us = 10000;
    }

    /* Randomize Opcode based on the configured Read/Write ratio */
    if ((rand_r(&b_ctx->seed) % 100) < b_ctx->read_ratio) {
        out_req->opc = 0x02; // NVME_CMD_READ
    } else {
        out_req->opc = 0x01; // NVME_CMD_WRITE
    }

    /* Fill random access parameters */
    out_req->slba = rand_r(&b_ctx->seed) % 0xFFFFFFFF; // Random 32-bit LBA
    out_req->nlb  = 1;                                // Transfer 1 block
    out_req->nsid = 1;                                // Default Namespace ID

    b_ctx->current_count++;
    return true;
}