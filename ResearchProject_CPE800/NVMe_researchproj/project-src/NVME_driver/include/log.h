#ifndef LOG_H
#define LOG_H

#include <stdio.h>

#define L_ERR(msg, reason) printf("[ERROR]: %s : %s\n", msg, reason)
#define L_WARN(msg, reason) printf("[WARN]: %s : %s\n", msg, reason)
#define L_SUCC(msg) printf("[SUCC]: %s.\n", msg)
#define L_INFO(msg) printf("[INFO]: %s.\n" , msg)

#endif /* !LOG_H */