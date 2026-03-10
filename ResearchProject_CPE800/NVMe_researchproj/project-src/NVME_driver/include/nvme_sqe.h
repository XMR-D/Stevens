#ifndef NVME_SQE_H
#define NVME_SQE_H

#include "nvme_transport.h"

Nvme_sqe_t * nvme_create_read_sqe(Nvme_seq_t * sqe);
Nvme_sqe_t * nvme_create_write_sqe(Nvme_seq_t * sqe);

/* add more NVMe Commands here.... */

#endif /* !NVME_SQE_H */