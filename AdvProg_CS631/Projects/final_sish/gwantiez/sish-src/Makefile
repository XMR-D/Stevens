include ../generic_flags.mk

SRC=sish.c opt-parser.c shell.c cmd-parser.c state-machine.c 
SRC+=handle-redirections.c signals-handling.c pipeline-exec.c logging.c 
SRC+=var-expanding.c portability.c

OBJS= ${SRC:.c=.o}

.PHONY: all, clean

all: ${OBJS}

#generate all object files from sources + print a compile info message to user.
${OBJS}:
	${CC} ${CFLAGS} ${LDFLAGS} -I${INCLUDE} -c ${@:.o=.c} -o ${@:.c=.o}

clean:
	@echo "Cleaning sources"
	rm *.o 
