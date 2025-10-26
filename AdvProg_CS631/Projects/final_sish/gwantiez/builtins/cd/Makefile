include ../../generic_flags.mk

#define builtin name and number.
BUILTIN_NAME=cd
BUILTIN_NB=1


#define all sources files in the builtin source file.
SRCS= cd.c
OBJS+= ${SRCS:.c=.o}

.PHONY: all, clean

all: ${OBJS}

#generate all object files from sources + print a compile info message to user.
${OBJS}:
	${CC} ${CFLAGS} ${LDFLAGS} -Iinclude -c ${@:.o=.c} -o ${@:.c=.o}

clean:
	rm *.o 
