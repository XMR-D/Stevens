CC= gcc
CFLAGS= -Wall -Wextra -Werror -pedantic -std=gnu99
DEBUGFLAGS= -g -O0 -fsanitize=address -Wall -Wextra -Werror -pedantic -std=gnu99
LDFLAGS= -lasan
INCLUDE= src/include
SRC_DIR= src
