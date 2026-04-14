#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "benchmark.h"
#include "riscv_time.h"

void generate_workload_buffer(rnd_bench_ctx_t *b_ctx)
{
    for (uint32_t i = 0; i < b_ctx->max_requests; i++) 
    {
        bench_req_t *req = &b_ctx->buffer[i];

        req->nsid = 1;

        /* Randomized budget assignment based on probability buckets */
        int32_t r = rand_r(&b_ctx->seed) % 100;
        uint64_t budget_us = (r < 10) ? 500 : (r < 30 ? 2000 : 10000);
        uint64_t budget_ticks = us_to_ticks(budget_us, b_ctx->cpu_freq_mhz);
        

        /* Store relative offset and budget for dynamic deadline calculation */
        req->latency_budget_ticks = budget_ticks;

        req->nlb = 7;
        req->prp1 = 0x20000000 + (i * 0x1000);
        req->prp2 = 0;
        
        /* Deterministic cost formula: Base cost of 4KB operation */
        req->expected_duration = 8 * 5000;
        
        /* Set operation type with weight-based cost adjustment */
        if ((rand_r(&b_ctx->seed) % 100) < b_ctx->read_ratio) {
            req->opc = 0x02; /* Read */
        } else {
            req->opc = 0x01; /* Write */
            req->expected_duration = (req->expected_duration * 120) / 100;
        }

        b_ctx->tail++;
    }
}

bool get_next_bench_request(void *ctx, bench_req_t *out_req) 
{
    rnd_bench_ctx_t *b_ctx = (rnd_bench_ctx_t *)ctx;

    if (b_ctx->head >= b_ctx->tail) {
        return false; 
    }

    /* Fast copy of pre-generated request */
    *out_req = b_ctx->buffer[b_ctx->head];

    out_req->latency_budget_ticks += get_riscv_tick();

    b_ctx->head++;
    return true;
}

void log_benchmark(rnd_bench_ctx_t * bench)
{
    printf("\n"
           "===============================================================\n"
           "                 NVMe DETERMINISTIC BENCHMARK                  \n"
           "===============================================================\n");

    printf("\n [PERFORMANCE]\n");

    printf("  Total Dispatch Time : %.4f s\n", ticks_to_us((bench->dispatch_end - bench->dispatch_start), bench->cpu_freq_mhz) / 1000000);
    printf("  Total Latency : %.4f ticks\n", (double) bench->latencies);
    printf("  Max Latency : %lu\n", bench->max_lat);
    printf("  Min Latency : %lu\n", bench->min_lat);
    printf("  Average Latency : %.4f ticks\n", (double) bench->latencies / (double) bench->requests_completed);
    
    printf("  CPU Frequency   : %lu MHz\n", bench->cpu_freq_mhz);
    printf("  Read/Write Ratio: %u/100\n", bench->read_ratio);

    printf("\n [RESULTS]\n");
    printf("  Completed    : %-12lu (%.2f%%)\n", 
            bench->requests_completed, 
            (double)bench->requests_completed / bench->max_requests * 100);
    printf("  Success    : %-12lu (%.2f%%)\n", 
            bench->complete_reason_success, 
            (double)bench->complete_reason_success / bench->max_requests * 100);
    printf("  Failed (Missed) : %-12lu\n", bench->requests_failed);
    printf("  Failed (NVMe error) : %-12lu\n", bench->complete_reason_failure);
    printf("  Dropped         : %-12lu (Dispatcher rejected)\n", bench->requests_not_accepted);



    printf("\n [REJECTION ANALYSIS]\n");
    if (bench->requests_not_accepted > 0) {
        printf("  - Deadline Expired : %-12lu (%.1f%%)\n", 
                bench->drop_reason_already_expired,
                (double) bench->drop_reason_already_expired / bench->requests_not_accepted * 100);
        printf("  - CID Exhaustion   : %-12lu (%.1f%%)\n", 
                bench->drop_reason_no_cid,
                (double) bench->drop_reason_no_cid / bench->requests_not_accepted * 100);
        printf("  - Service time exceeded   : %-12lu (%.1f%%)\n", 
                bench->drop_reason_service_time,
                (double) bench->drop_reason_service_time / bench->requests_not_accepted * 100);
    } else {
        printf("  No requests were dropped. System saturated correctly.\n");
    }
    printf("===============================================================\n\n");
}