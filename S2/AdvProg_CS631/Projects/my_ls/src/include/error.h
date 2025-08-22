#ifndef ERROR_H
#define ERROR_H

/* Input handling error codes */
#define MEM_ERR           1
#define WRNG_OPT_ERR      2
#define WRNG_TARG_ERR     3

int throw_error(char wrng_opt, char * wrng_targ, char * reason, int err);

#endif /* !_ERROR_H */