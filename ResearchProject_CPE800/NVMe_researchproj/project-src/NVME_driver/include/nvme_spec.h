#ifndef NVME_SPEC_H
#define NVME_SPEC_H

#include <stdint.h>

/* NVME CUSTOM DEFINITIONS */

#define NVME_QUEUE_DEPTH 128            /* starting value for queue depth */
#define PAGESIZE 4096                   /* Mem page size */
#define NVME_BUSY_WAIT_THRESHOLD 1000   /* Nb of iteration for busy wait optimization */
/* 
 * 16KB is sufficient for standard NVMe controller registers.
 * Note: If using Controller Memory Buffer (CMB), the actual BAR0 
 * size should be queried via sysfs (resource0_size).
 */
#define NVME_BAR0_SIZE 0x4000       /* NVME bar0 size */


/* Submission and colmpletion queues size computation */
#define SQ_SIZE (NVME_QUEUE_DEPTH * 64) 
#define CQ_SIZE (NVME_QUEUE_DEPTH * 16) 

/* DDMA buffer size round up to next pagesize just in case */
#define DEVICE_DDMA_BUFF_SIZE (((SQ_SIZE + CQ_SIZE) + PAGESIZE) & ~PAGESIZE)


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
    uint32_t res1;          /* 0x18: Reserved */
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
    uint64_t resv2;         /* 0x50: Reserved */
    uint32_t cmbmsc;        /* 0x58: Controller Memory Buffer Memory Space Control */
    uint32_t cmbsts;        /* 0x5C: Controller Memory Buffer Status */
    uint32_t cmbebs;        /* 0x60: CMB Elasticity Buffer Size */
    uint32_t cmbswtp;       /* 0x64: CMB Sustained Write Throughput */
    uint32_t nssd;          /* 0x68: NVM Subsystem Shutdown */
    uint32_t crto;          /* 0x6C: Controller Ready Timeouts */
    uint8_t  res3[3504];    /* 0x70 - 0xE00: Reserved range */
    uint64_t pmrcap;        /* 0xE00: Persistent Memory Capabilities */
    uint32_t pmrctl;        /* 0xE08: Persistent Memory Region Control */
    uint32_t pmrsts;        /* 0xE0C: Persistent Memory Region Status */
    uint32_t pmrebs;        /* 0xE10: PMR Elasticity Buffer Size */
    uint32_t pmrswtp;       /* 0xE14: PMR Sustained Write Throughput */
    uint32_t pmrmscl;       /* 0xE18: PMR Controller Memory Space Control Lower */
    uint32_t pmrmscu;       /* 0xE1C: PMR Controller Memory Space Control Upper */
} __attribute__((packed)) Nvme_registers;



/* NVME PROPERTIES */

/* nvme_cap_prop : Define the structure of the NVMe Capabilities
   property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p50-54)
           
*/
typedef struct nvme_cap_prop {
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
} __attribute__((packed)) Nvme_cap_prop;


/* Nvme_cc_prop : Define the structure of the Controller
   Configuration (CC) property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p58-60)
*/
typedef struct nvme_cc_prop {
    uint32_t EN     : 1;  /* Enable */
    uint32_t res0   : 3;  /* Reserved */
    uint32_t CSS    : 3;  /* I/O Command Set Selected */
    uint32_t MPS    : 4;  /* Memory Page Size */
    uint32_t ARS    : 3;  /* Arbitration Mechanism Selected */
    uint32_t SHN    : 2;  /* Shutdown Notification */
    uint32_t IOSQES : 4;  /* I/O Submission Queue Entry Size */
    uint32_t IOCQES : 4;  /* I/O Completion Queue Entry Size */
    uint32_t res1   : 8;  /* Reserved */
} __attribute__((packed)) Nvme_cc_prop;


/* Nvme_csts_prop : Define the structure of the Controller
   Status (CSTS) property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p60-61)
*/
typedef struct nvme_csts_prop {
    uint32_t RDY    : 1;  /* Controller Ready */
    uint32_t CFS    : 1;  /* Controller Fatal Status */
    uint32_t SHST   : 2;  /* Shutdown Status */
    uint32_t NSSRO  : 1;  /* NVM Subsystem Reset Occurred */
    uint32_t PP     : 1;  /* Processing Paused */
    uint32_t ST     : 1;  /* Shutdown Type */
    uint32_t res0   : 25; /* Reserved */
} __attribute__((packed)) Nvme_csts_prop;



/* nvme_cap_prop : Define the structure of the Controller
   Memory Buffer Location (CMBLOC) property.

   Note : Field details can be found here : 
   (NVM Express® Base Specification, Revision 2.2 p63-64)
           
*/
typedef struct nvme_cmbloc_prop {
    uint32_t BIR     : 3;  /* Base Indicator Register */
    uint32_t CQMMS   : 1;  /* CMB Queue Mixed Memory Support */
    uint32_t CQPDS   : 1;  /* CMB Queue Physically Discontiguous Support */
    uint32_t CDPMLS  : 1;  /* CMB Data Pointer Mixed Locations Support */
    uint32_t CDPCILS : 1;  /* CMB Data Pointer and Command Independent Locations Support */
    uint32_t CDMMMS  : 1;  /* CMB Data Metadata Mixed Memory Support */
    uint32_t CQDA    : 1;  /* CMB Queue Dword Alignment */
    uint32_t res0    : 3;  /* Reserved */
    uint32_t OFST    : 20; /* Controller Memory Buffer Location Offset */
} __attribute__((packed)) Nvme_cmbloc_prop;


#endif /* ! nvme_spec_h */


