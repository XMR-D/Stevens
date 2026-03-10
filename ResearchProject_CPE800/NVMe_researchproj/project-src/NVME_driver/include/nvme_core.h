#ifndef NVME_CORE_H
#define NVME_CORE_H

#include <stdint.h>

#include "nvme_q.h"

/* Macros to write fields within nvme structs */
#define SET_NVME_PROP_FIELD_32(prop_ptr, prop_ty, field, val) do { \
    prop_ty nvme_s; \
    nvme_s.raw = *(volatile uint32_t *)(prop_ptr); \
    nvme_s.fields.field = (val); \
    *(volatile uint32_t *)(prop_ptr) = nvme_s.raw; \
    __asm__ volatile ("fence w, w" ::: "memory"); \
} while(0)

#define SET_NVME_PROP_FIELD_64(prop_ptr, prop_t, field, val) do { \
    prop_t nvme_s; \
    nvme_s.raw = *(volatile uint64_t *)(prop_ptr); \
    nvme_s.fields.field = (val); \
    *(volatile uint64_t *)(prop_ptr) = nvme_s.raw; \
    __asm__ volatile ("fence w, w" ::: "memory"); \
} while (0)

#define SET_NVME_REG_32(reg_ptr, val) do { \
    *(volatile uint32_t*)(reg_ptr) = (val); \
    __asm__ volatile ("fence w, w" ::: "memory"); \
} while (0)

#define SET_NVME_REG_64(reg_ptr, val) do { \
    *(volatile uint64_t *)(reg_ptr) = (val); \
    __asm__ volatile ("fence w, w" ::: "memory"); \
} while (0)


/* PCI Base address register mapping */
void * bar_map(char * ressource_path, char * bdf);
void bar_unmap(volatile void * bar);

/* Logging functions */
void nvme_capability_log(volatile void * bar);
void nvme_cc_log(volatile void * bar);
void nvme_cmbloc_log(volatile void * bar);
void nvme_aqa_log(volatile void * bar) ;

/* NVMe device control functions */
int8_t nvme_init(volatile void * bar, Nvmeq_context_t * nvmeq_ctx);


#endif /* !NVME_CORE_H */