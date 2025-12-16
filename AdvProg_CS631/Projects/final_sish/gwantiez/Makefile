include generic_flags.mk

#define all sources files in builtins using find
#and all source files in src/ that correspond to sish source code
SRC_DIR=sish-src builtins/cd builtins/echo
SRC_FILES!=find ${SRC_DIR} -name "*.c"

# Create all objects files from C source files
OBJS+= ${SRC_FILES:.c=.o}
PROG_NAME= sish

# Specify that "all" is not a file and rather a command.
.PHONY: all sish_omnios sish_gnu_bsd sish_debug clean

# Generic receipe when make is invoked without arguments
all:
	@echo "Following receipes are available:"
	@echo "sish_gnu_bsd : Build the project for Linux and NetBSD plat"
	@echo "sish_omnios : Build the project for OmniOS plat"
	@echo "debug : (unstable) Build the project with debugging flags"
	@echo "clean : Delete all the files except from source or build files"

sish_omnios: 
	@gmake sish 'MAKE-UTIL=gmake'

sish_gnu_bsd: ${OBJS}

# When debug is invoke it will call a sub_receipe to add 
# debug flags to C flags using the environement and then compile
debug :
	@${MAKE-UTIL} debug_compil 'CFLAGS=${DEBUGFLAGS}' 'PROG_NAME=debug_sish'
debug_compil: ${OBJS}

#C source file compilation with C flags
${OBJS}:
	@echo "=> Starting to compile Builtins..."
	@echo ""
	@${MAKE-UTIL} -C builtins
	@echo ""
	@echo "=> Builtin compilation success, Compiling shell sources..."
	@${MAKE-UTIL} -C sish-src
	@echo ""
	@echo "=> Linking shell source code and builtins..."
	${CC} ${CFLAGS} -I${INCLUDE} ${OBJS} -o ${PROG_NAME}

# Specify that "clean" is not a file and rather a command.
.PHONY: clean
clean:
	@echo "=> Cleaning builtin..."
	@${MAKE-UTIL} clean -C builtins
	@echo "=> Cleaning sish sources..."	
	rm -rf sish debug_sish sish-src/*.o valgrind-out.txt *.core
