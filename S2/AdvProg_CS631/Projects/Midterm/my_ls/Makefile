include mkflags/generic_flags.mk

#list all sources files in src/ dir
SRCS=src/ls.c src/opt-parser.c src/listing.c src/padding-handling.c src/printing.c src/utility.c

# Create all objects files from C and Asm source files
OBJS= ${SRCS:.c=.o}

PROG= ls

# Specify that "all" is not a file and rather a command.
.PHONY: all

# Generic receipe when make is invoked without arguments
all: ${PROG}

${PROG}: ${OBJS}
	${CC} ${CFLAGS} ${SRCS} -o ${PROG}

# When debug is invoke it will call a sub_receipe to add 
# debug flags to C flags using the environement and then compile
debug :
	make debug_flag 'CFLAGS=${DEBUGFLAGS}'
debug_flag: ${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} ${OBJS} -o debug_ls

#C source file compilation with C flags
${OBJS}:
	${CC} ${CFLAGS} ${LDFLAGS} -c ${@:.o=.c} -o ${@:.c=.o}

# Specify that "clean" is not a file and rather a command.
.PHONY: clean
clean:
	rm -rdf ls debug_ls ${SRC_DIR}/*.o valgrind-out.txt
