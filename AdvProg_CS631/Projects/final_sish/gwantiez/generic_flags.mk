CC= gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c99 -O0
DEBUGFLAGS= -g -fsanitize=leak -O0 -Wall -Wextra -Werror -pedantic -std=c99

#Here, due to limitations of BSD make, we need to manually place
#all the linker flags and potential include directories for
#the whole project (builtins included)
LDFLAGS=
INCLUDE=include -I../builtins

#define the number of builtin we need to compile
#doing that way allows to have a more modular implementation
NB_BUILTINS=2
