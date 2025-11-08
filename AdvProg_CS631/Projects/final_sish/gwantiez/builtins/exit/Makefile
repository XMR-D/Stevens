include ../../generic_flags.mk

#define builtin name and number.
BUILTIN_NAME=exit
BUILTIN_NB=3



#define all sources files in the builtin source file.
SRCS= exit.c
OBJS+= ${SRCS:.c=.o}

.PHONY: all, clean

all: ${OBJS}

#generate all object files from sources + print a compile info message to user.
${OBJS}:
	${CC} ${CFLAGS} ${LDFLAGS} -Iinclude -c ${@:.o=.c} -o ${@:.c=.o}

clean:
	@echo "Cleaning builtin: exit"
	rm *.o 
