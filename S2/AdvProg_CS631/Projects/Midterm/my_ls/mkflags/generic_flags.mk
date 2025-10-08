CC= gcc
CFLAGS= -Wall -Wextra -Werror -pedantic -std=gnu99 -O0 -I src/include
DEBUGFLAGS= -g -fsanitize=leak -O0 -Wall -Wextra -Werror -pedantic -std=c99 -I src/include
LDFLAGS=-lm
SRC_DIR= src
