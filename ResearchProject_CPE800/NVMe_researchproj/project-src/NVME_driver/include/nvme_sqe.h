#ifndef NVME_SQE_H
#define NVME_SQE_H

#include "nvme_transport.h"

/* Error handling opcode */
#define NVME_IO_INVALID 0xFF

/* I/O commands opcodes */
#define NVME_IO_WRITE 0x01
#define NVME_IO_READ 0x02

/* Admin commands opcodes (Spec NVMe) */
#define NVME_ADM_OP_CREATE_SQ         0x01
#define NVME_ADM_OP_CREATE_CQ         0x05
#define NVME_ADM_OP_IDENTIFY          0x06
#define NVME_ADM_OP_SET_FEATURES      0x09

/* Feature Identifiers (FID) pour Set Features */
#define NVME_FEAT_NUMBER_OF_QUEUES    0x07

/* Configuration des Queues I/O */
#define NVME_QUEUE_PHYS_CONTIGUOUS    (1 << 0)


/* ADMIN COMMANDS CREATION ROUTINES */

Nvme_sqe_t * nvme_create_set_features_sqe(uint8_t fid, uint32_t dword11);
Nvme_sqe_t * nvme_create_iocompcreate_sqe(uint16_t qid, uint16_t size, uint64_t cq_phys);
Nvme_sqe_t * nvme_create_iosubcreate_sqe(uint16_t qid, uint16_t size, uint64_t sq_phys, uint16_t cqid);

/* OPTIONAL FOR RESEARCH PURPOSE

Nvme_sqe_t * nvme_create_iocompdelete_sqe(uint16_t qid, uint16_t size, uint64_t cq_phys);
Nvme_sqe_t * nvme_create_identify_sqe(uint8_t cns, uint16_t nsid);
Nvme_sqe_t * nvme_create_iosubdelete_sqe(uint16_t qid, uint16_t size, uint64_t sq_phys);
Nvme_sqe_t * nvme_create_get_log_page_sqe(uint8_t lid, uint32_t num_dwords);

Nvme_sqe_t * nvme_create_abort_sqe(uint16_t cid, uint16_t sqid);
Nvme_sqe_t * nvme_create_get_features_sqe(uint8_t fid);
Nvme_sqe_t * nvme_create_async_event_sqe(void);
*/


/* IO COMMANDS CREATION ROUTINES */
Nvme_sqe_t * nvme_create_flush_io_sqe(void);

/* IO read/write sqe creation */
Nvme_sqe_t nvme_create_io_sqe(uint8_t opcode, uint64_t slba, uint16_t nlb, uint64_t prp1, uint64_t prp2);

Nvme_sqe_t * nvme_create_dsm_io_sqe(uint32_t num_ranges, uint64_t prp1);


#endif /* !NVME_SQE_H */