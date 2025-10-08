#ifndef PADDING_HANDLING_H
#define PADDING_HANDLING_H

#include <sys/stat.h>

#include "opt_parser.h"

/*
 * This structure contain various fields that will hold the maximum length
 * of various stat informations so that the correct padding can be applied
 * when printing, all fields should be self explanatory
 */
typedef struct PaddingInfos {
    int max_link_nb_len;
    int max_uid_str_len;
    int max_gid_str_len;
    int max_uid_int_len;
    int max_gid_int_len;
    int max_nb_byte_len;
    int max_nb_block_len;
    int max_inode_nb_len;
    int max_special_file_byte_len;
    long double total_bytes;
    long double total_blocks;
} PaddingInfos;

void print_padding(char *str1, int longest);
void reset_print_infos(PaddingInfos *infos);
void compute_padding_needed(struct stat sb, UsrOptions *usr_opt);


#endif /* ! PADDING_HANDLING_H */
