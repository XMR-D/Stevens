#ifndef NVME_CQE_H
#define NVME_CQE_H

#include "nvme_transport.h"

/* CQE PARSING COMMANDS */
int8_t nvme_parse_iosubdelete_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_iosubcreate_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_get_log_page_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_iocompcreate_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_iocompdelete_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_identify_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_abort_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_set_features_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_get_features_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_async_event_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_flush_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_write_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_read_cqe(Nvme_cqe_t * cqe);
int8_t nvme_parse_dsm_cqe(Nvme_cqe_t * cqe);


#endif /* !NVME_CQE_H */