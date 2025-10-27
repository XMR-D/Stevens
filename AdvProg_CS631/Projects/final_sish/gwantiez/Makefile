include generic_flags.mk

NB_BUILTINS=2
.export NB_BUILTINS

#define all sources files in builtins using find
#and all source files in src/ that correspond to sish source code
SRC_DIR=shell-src builtins/cd builtins/echo
SRC_FILES!=find ${SRC_DIR} -name "*.c"

#Here, due to limitations of BSD make, we need to manually place
#all the linker flags and potential include directories for
#the whole project (builtins included)
LDFLAGS=
INCLUDE=-Ishell-src/include -Ibuiltins/cd/include -Ibuiltins/echo/include

# Create all objects files from C source files
OBJS+= ${SRC_FILES:.c=.o}

PROG_NAME= sish

# Specify that "all" is not a file and rather a command.
.PHONY: all sish sish_debug clean

# Generic receipe when make is invoked without arguments
all: ${OBJS}


# When debug is invoke it will call a sub_receipe to add 
# debug flags to C flags using the environement and then compile
debug :
	@make debug_compil 'CFLAGS=${DEBUGFLAGS}' 'PROG_NAME=debug_sish'
debug_compil: ${OBJS}

#C source file compilation with C flags
${OBJS}:
	@echo "=> Starting to compile Builtins..."
	@echo ""
	@make -C builtins
	@echo ""
	@echo "=> Builtin compilation success, Compiling shell sources..."
	@make -C shell-src
	@echo ""
	@echo "=> Linking shell source code and builtins..."
	${CC} ${CFLAGS} -I${INCLUDE} ${OBJS} -o ${PROG_NAME}

# Specify that "clean" is not a file and rather a command.
.PHONY: clean
clean:
	@echo "=> Cleaning builtin..."
	@make clean -C builtins
	@echo "=> Cleaning sish sources..."	
	rm -rdf sish debug_sish shell-src/*.o valgrind-out.txt *.core
