#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>
#include <stdbool.h>

#define NB_WORLOADS 1

typedef struct {
    uint64_t latency_budget_ticks;
    uint64_t expected_duration;
    uint64_t absolute_deadline;
    uint64_t slba;
    uint64_t prp1;              
    uint64_t prp2;
    uint32_t nsid;
    uint16_t nlb;
    uint8_t  opc;
} bench_req_t;

typedef struct {
    uint32_t max_requests;
    uint32_t head;
    uint32_t tail;
    uint32_t seed;
    uint64_t requests_completed;          /* Successfully processed by worker */
    uint64_t requests_not_accepted;       /* Dropped by dispatcher (no queue/CID) */
    uint64_t requests_failed;             /* Processed but missed deadline */    
    uint64_t drop_reason_already_expired; /* passed_deadline */
    uint64_t drop_reason_no_cid;          /* No more cids available (CID) */
    uint64_t drop_reason_service_time;    /* Service time too long */
    uint64_t complete_reason_success;     /* Completed successfully */
    uint64_t complete_reason_failure;     /* Completed with nvme failure */
    uint64_t latencies;                   /* latencies counter to compute average latency */
    uint64_t dispatch_start;
    uint64_t time_elapsed;
    uint8_t  read_ratio;
    uint64_t cpu_freq_mhz;

    bench_req_t buffer[NB_WORLOADS];

} rnd_bench_ctx_t;

/* Pre-fills the buffer to isolate random generation overhead */
void generate_workload_buffer(rnd_bench_ctx_t *b_ctx);

/* O(1) consumer for the dispatch loop */
bool get_next_bench_request(void *ctx, bench_req_t *out_req);

void log_benchmark(rnd_bench_ctx_t * bench);

#endif