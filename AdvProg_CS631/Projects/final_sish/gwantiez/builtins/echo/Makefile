include ../../generic_flags.mk

#define builtin name and number.
BUILTIN_NAME=echo
BUILTIN_NB=2



#define all sources files in the builtin source file.
SRCS= echo.c
OBJS+= ${SRCS:.c=.o}

.PHONY: all, clean

all: ${OBJS}

#generate all object files from sources + print a compile info message to user.
${OBJS}:
	@echo "Compiling $(BUILTIN_NAME) [${BUILTIN_NB}/${NB_BUILTINS}]"
	${CC} ${CFLAGS} ${LDFLAGS} -Iinclude -c ${@:.o=.c} -o ${@:.c=.o}

clean:
	rm *.o 
