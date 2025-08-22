#ifndef LOG_H
#define LOG_H

/* Multiple macros used to log properly the project, use during debug and unit testing only*/
#define INFO(msg) printf("[\033[1;36mINFO\033[0m] " msg "\n")
#define WARN(msg) printf("[\033[1;33mWARN\033[0m] " msg "\n")
#define SUCCESS(msg) printf("[\033[1;32mSUCC\033[0m] " msg "\n")
#define ERROR(msg) printf("[\033[1;31mERR\033[0m] " msg "\n")

#define FUNC_LOG(func, msg) printf("[" func "] " msg "\n")


#endif /* !_LOG_H */