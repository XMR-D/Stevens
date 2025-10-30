include ../generic_flags.mk

SRC=sish.c opt-parser.c shell.c cmd-parser.c signals-handling.c cmd-exec.c 
OBJS= ${SRC:.c=.o}

.PHONY: all, clean

all: ${OBJS}

#generate all object files from sources + print a compile info message to user.
${OBJS}:
	${CC} ${CFLAGS} ${LDFLAGS} -Iinclude -c ${@:.o=.c} -o ${@:.c=.o}

clean:
	@echo "Cleaning sources"
	rm *.o 
