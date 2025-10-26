CC= gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c99 -O0 -Isrc/include
DEBUGFLAGS= -g -fsanitize=leak -O0 -Wall -Wextra -Werror -pedantic -std=c99 -Isrc/include
LDFLAGS=-lm
SRC_DIR= src
