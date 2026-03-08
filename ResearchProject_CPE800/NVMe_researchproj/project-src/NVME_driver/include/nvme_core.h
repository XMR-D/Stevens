#ifndef NVME_CORE_H
#define NVME_CORE_H

/* PCI Base address register mapping */
void * bar_map(char * ressource_path, char * bdf);
void bar_unmap(volatile void * bar);

/* Logging functions */
int8_t nvme_capability_log(volatile void * bar);
void nvme_cmbloc_log(volatile void * bar);

/* NVMe device control functions */
int8_t nvme_init(volatile void * bar);


#endif /* !NVME_CORE_H */