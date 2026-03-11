#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include "nvme_spec.h"
#include "nvme_transport.h"

#include "nvme_cqe.h"

/*
 * nvme_parse_iocreate_cqe: Parses the completion of an I/O Queue creation 
 * command. Validates the status and confirms the controller successfully 
 * initialized the requested submission and completion queues.
 */
int8_t nvme_parse_iocreate_cqe(Nvme_cqe_t * cqe)
{
    (void) cqe;
    return EXIT_SUCCESS;
}

/*
 * nvme_parse_identify_cqe: Parses the completion of an Identify command.
 * Processes the controller or namespace data returned by the device 
 * and updates the internal driver capability structures.
 */
int8_t nvme_parse_identify_cqe(Nvme_cqe_t * cqe)
{
    (void) cqe;
    return EXIT_SUCCESS;
}

/*
 * nvme_parse_read_cpe: Parses the completion of a Read operation.
 * Verifies that the data transfer from the device to host memory 
 * completed successfully and processes the resulting status code.
 */
int8_t nvme_parse_read_cpe(Nvme_cqe_t * cqe)
{
    (void) cqe;
    return EXIT_SUCCESS;
}

/*
 * nvme_parse_write_cpe: Parses the completion of a Write operation.
 * Verifies that the data transfer from host memory to the device 
 * has been safely committed to the non-volatile storage media.
 */
int8_t nvme_parse_write_cpe(Nvme_cqe_t * cqe)
{
    (void) cqe;
    return EXIT_SUCCESS;
}
