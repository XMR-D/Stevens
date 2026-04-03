#ifndef NVME_SPEC_H
#define NVME_SPEC_H

#include <stdint.h>

#define CAP_OFFSET      (0x0)       /* Controller Capabilities*/
#define VS_OFFSET       (0x8)       /* Version */
#define INTMS_OFFSET    (0xC)       /* Interrupt Mask Set */
#define INTMC_OFFSET    (0x10)      /* Interrupt Mask Clear */
#define CC_OFFSET       (0x14)      /* Controller Configuration*/
#define CSTS_OFFSET     (0x1C)      /* Controller Status */
#define NSSR_OFFSET     (0x20)      /* NVM Subsystem Reset */
#define AQA_OFFSET      (0x24)      /* Admin Queue Attributes */
#define ASQ_OFFSET      (0x28)      /* Admin Submission Queue Base Address */
#define ACQ_OFFSET      (0x30)      /* Admin Completion Queue Base Address */
#define CMBLOC_OFFSET   (0x38)      /* Controller Memory Buffer Location */
#define CMBSZ_OFFSET    (0x3C)      /* Controller Memory Buffer Size */
#define BPINFO_OFFSET   (0x40)      /* Boot Partition Information */
#define BPRSEL_OFFSET   (0x44)      /* Boot Partition Read Select */
#define BPMBL_OFFSET    (0x48)      /* Boot Partition Memory Buffer Location */
#define CMBMSC_OFFSET   (0x50)      /* Controller Memory Buffer Memory Space Control */
#define CMBSTS_OFFSET   (0x58)      /* Controller Memory Buffer Status */
#define CMBEBS_OFFSET   (0x5C)      /* Controller Memory Buffer Elasticity Buffer Size*/
#define CMBSWTP_OFFSET  (0x60)      /* Controller Memory Buffer Sustained Write Throughput */
#define NSSD_OFFSET     (0x64)      /* NVM Subsystem Shutdown */
#define CRTO_OFFSET     (0x68)      /* Controller Ready Timeouts */
#define PMRCAP_OFFSET   (0xE00)     /* Persistent Memory Capabilities */
#define PMRCTL_OFFSET   (0xE04)     /* Persistent Memory Region Control */ 
#define PMRSTS_OFFSET   (0xE08)     /* Persistent Memory Region Status */
#define PMREBS_OFFSET   (0xE0C)     /* Persistent Memory Region Elasticity Buffer Size */
#define PMRSWTP_OFFSET  (0xE10)     /* Persistent Memory Region Sustained Write Throughput */
#define PMRMSCL_OFFSET  (0xE14)     /* Persistent Memory Region Controller Memory Space Control Lower */
#define PMRMSCU_OFFSET  (0xE18)     /* Persistent Memory Region Controller Memory Space Control Upper */


/* NVME CUSTOM DEFINITIONS */

#define NVME_QUEUE_DEPTH 2048          /* starting value for queue depth */
#define PAGESIZE 4096                  /* Mem page size */
#define NVME_BUSY_WAIT_THRESHOLD 1000  /* Nb of iteration for busy wait optimization */
#define NVME_MAX_CID 65534             /* Nb of total possible cid to indentify tasks */
/* 
 * 16KB is sufficient for standard NVMe controller registers.
 * Note: If using Controller Memory Buffer (CMB), the actual BAR0 
 * size should be queried via sysfs (resource0_size).
 */
#define NVME_BAR0_SIZE 0x4000

#define SQ_ENTRY_SIZE 64
#define CQ_ENTRY_SIZE 16

/* Log2(64) = 6, Log2(16) = 4 */
#define NVME_LOG2_SQ_SIZE 6
#define NVME_LOG2_CQ_SIZE 4

/* Submission and colmpletion queues size computation */
#define SQ_SIZE (NVME_QUEUE_DEPTH * SQ_ENTRY_SIZE) 
#define CQ_SIZE (NVME_QUEUE_DEPTH * CQ_ENTRY_SIZE)

/* DDMA buffer size round up to next pagesize just in case */
#define DEVICE_NVMEQ_BUFF_SIZE (((SQ_SIZE + CQ_SIZE) + PAGESIZE) & ~PAGESIZE)


/* NVME CORE STRUCT */

/* Nvme_registers : Define the structure of the NVMe Capabilities
   property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p50-54) 
*/
typedef struct nvme_regs {
    uint64_t cap;           /* 0x00: Controller Capabilities */
    uint32_t vs;            /* 0x08: Version */
    uint32_t intms;         /* 0x0C: Interrupt Mask Set */
    uint32_t intmc;         /* 0x10: Interrupt Mask Clear */
    uint32_t cc;            /* 0x14: Controller Configuration */
    uint32_t res0;          /* 0x18: Reserved */
    uint32_t csts;          /* 0x1C: Controller Status */
    uint32_t nssr;          /* 0x20: NVM Subsystem Reset */
    uint32_t aqa;           /* 0x24: Admin Queue Attributes */
    uint64_t asq;           /* 0x28: Admin Submission Queue Base Address */
    uint64_t acq;           /* 0x30: Admin Completion Queue Base Address */
    uint32_t cmbloc;        /* 0x38: Controller Memory Buffer Location */
    uint32_t cmbsz;         /* 0x3C: Controller Memory Buffer Size */
    uint32_t bpinfo;        /* 0x40: Boot Partition Information */
    uint32_t bprsel;        /* 0x44: Boot Partition Read Select */
    uint64_t bpmbl;         /* 0x48: Boot Partition Memory Buffer Location */
    uint64_t res1;         /* 0x50: Reserved */
    uint32_t cmbmsc;        /* 0x58: Controller Memory Buffer Memory Space Control */
    uint32_t cmbsts;        /* 0x5C: Controller Memory Buffer Status */
    uint32_t cmbebs;        /* 0x60: CMB Elasticity Buffer Size */
    uint32_t cmbswtp;       /* 0x64: CMB Sustained Write Throughput */
    uint32_t nssd;          /* 0x68: NVM Subsystem Shutdown */
    uint32_t crto;          /* 0x6C: Controller Ready Timeouts */
    uint8_t  res2[3504];    /* 0x70 - 0xE00: Reserved range */
    uint64_t pmrcap;        /* 0xE00: Persistent Memory Capabilities */
    uint32_t pmrctl;        /* 0xE08: Persistent Memory Region Control */
    uint32_t pmrsts;        /* 0xE0C: Persistent Memory Region Status */
    uint32_t pmrebs;        /* 0xE10: PMR Elasticity Buffer Size */
    uint32_t pmrswtp;       /* 0xE14: PMR Sustained Write Throughput */
    uint32_t pmrmscl;       /* 0xE18: PMR Controller Memory Space Control Lower */
    uint32_t pmrmscu;       /* 0xE1C: PMR Controller Memory Space Control Upper */
}  Nvme_registers;



/* NVME PROPERTIES */

/* Nvme_cap_prop : Define the structure of the NVMe Capabilities
   property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p50-54)
           
*/
typedef union {
        struct nvme_cap_prop {
            uint64_t MQES   : 16; /* Maximum Queue Entries Supported */
            uint64_t CQR    : 1;  /* Contiguous Queues Required */
            uint64_t AMS    : 2;  /* Arbitration Mechanism Supported */
            uint64_t res1   : 5;  /* Reserved */
            uint64_t TO     : 8;  /* Timeout */
            uint64_t DSTRD  : 4;  /* Doorbell Stride */
            uint64_t NSSRS  : 1;  /* NVM Subsystem Reset Supported */
            uint64_t CSS    : 8;  /* Command Sets Supported */
            uint64_t BPS    : 1;  /* Boot Partition Support */
            uint64_t CPS    : 2;  /* Controller Power Scope */
            uint64_t MPSMIN : 4;  /* Memory Page Size Minimum */
            uint64_t MPSMAX : 4;  /* Memory Page Size Maximum */
            uint64_t PMRS   : 1;  /* Persistent Memory Region Supported */
            uint64_t CMBS   : 1;  /* Controller Memory Buffer Supported */
            uint64_t NSSS   : 1;  /* NVM Subsystem Shutdown Supported */
            uint64_t CRMS   : 2;  /* Controller Ready Modes Supported */
            uint64_t NSSES  : 1;  /* NVM Subsystem Shutdown Enhancement Supported */
            uint64_t res0   : 2;  /* Reserved */
        }  fields;
        uint64_t raw;
 } Nvme_cap_prop;


/* Nvme_cc_prop : Define the structure of the Controller
   Configuration (CC) property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p58-60)
*/

typedef union {
    struct nvme_cc_prop {
        uint32_t EN     : 1;  /* Enable */
        uint32_t res0   : 3;  /* Reserved */
        uint32_t CSS    : 3;  /* I/O Command Set Selected */
        uint32_t MPS    : 4;  /* Memory Page Size */
        uint32_t ARS    : 3;  /* Arbitration Mechanism Selected */
        uint32_t SHN    : 2;  /* Shutdown Notification */
        uint32_t IOSQES : 4;  /* I/O Submission Queue Entry Size */
        uint32_t IOCQES : 4;  /* I/O Completion Queue Entry Size */
        uint32_t res1   : 8;  /* Reserved */
    }  fields;
    uint32_t raw;
} Nvme_cc_prop;


/* Nvme_csts_prop : Define the structure of the Controller
   Status (CSTS) property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p60-61)
*/
typedef union {
    struct nvme_csts_prop {
        uint32_t RDY    : 1;  /* Controller Ready */
        uint32_t CFS    : 1;  /* Controller Fatal Status */
        uint32_t SHST   : 2;  /* Shutdown Status */
        uint32_t NSSRO  : 1;  /* NVM Subsystem Reset Occurred */
        uint32_t PP     : 1;  /* Processing Paused */
        uint32_t ST     : 1;  /* Shutdown Type */
        uint32_t res0   : 25; /* Reserved */
    }  fields;
    uint32_t raw;
 } Nvme_csts_prop;



/* Nvme_cap_prop : Define the structure of the Controller
   Memory Buffer Location (CMBLOC) property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p63-64)
           
*/
typedef union {
    struct nvme_cmbloc_prop {
        uint32_t BIR     : 3;  /* Base Indicator Register */
        uint32_t CQMMS   : 1;  /* CMB Queue Mixed Memory Support */
        uint32_t CQPDS   : 1;  /* CMB Queue Physically Discontiguous Support */
        uint32_t CDPMLS  : 1;  /* CMB Data Pointer Mixed Locations Support */
        uint32_t CDPCILS : 1;  /* CMB Data Pointer and Command Independent Locations Support */
        uint32_t CDMMMS  : 1;  /* CMB Data Metadata Mixed Memory Support */
        uint32_t CQDA    : 1;  /* CMB Queue Dword Alignment */
        uint32_t res0    : 3;  /* Reserved */
        uint32_t OFST    : 20; /* Controller Memory Buffer Location Offset */
    }  fields;
    uint32_t raw;
} Nvme_cmbloc_prop;


/* Nvme_aqa_prop : Define the structure of the Admin Queue Attributes (AQA) property.
   
   This register defines the attributes for the Admin Submission and Completion Queues.
   Reference: NVM Express® Base Specification, Revision 2.2, p.70
*/
typedef union {
    struct nvme_aqa_prop {
        uint16_t ASQS   : 12; /* Admin Submission Queue Size: Defines the size of the Admin Submission Queue.*/
        uint16_t res0   : 4;  /* Reserved */
        uint16_t ACQS   : 12; /* Admin Completion Queue Size: Defines the size of the Admin Completion Queue. */
        uint16_t res1   : 4;  /* Reserved */
    }  fields;
    uint32_t raw;
 } Nvme_aqa_prop;




/* SQE AND CQE FORMAT SPECIFICATIONS */

/* Opcodes */
#define IODELETE_SUB_OP 0x0
#define IOCREATE_SUB_OP 0x1
#define GETLOG_OP 0x2
#define IODELETE_COM_OP 0x2
#define IOCREATE_COM_OP 0x5
#define IDENTIFY_OP 0x6
#define ABORT_OP 0x8
#define SET_FEATURE_OP 0x9
#define GET_FEATURE_OP 0xA
#define ASYNC_EVENT_OP 0XC

#define IO_READ_OP 
#define IO_WRITE_OP


#endif /* ! NVME_SPEC_H */