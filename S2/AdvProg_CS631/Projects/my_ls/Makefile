include mkflags/generic_flags.mk

#list all sources files in src/ dir, using a wildcard to prevent dev from adding every source file by hand.
srcs-y:= $(wildcard src/*.c src/*.S)

# Create all objects files from C and Asm source files
objs-y:=$(patsubst %.c,%.o,$(srcs-y))
objs-y:=$(patsubst %.S,%.S.o,$(objs-y))


# Specify that "all" is not a file and rather a command.
.PHONY: all

# Generic receipe when make is invoked without arguments
all: ls

ls: $(objs-y)
	$(CC) $(CFLAGS) -I$(INCLUDE) $^ -o $@

# When debug is invoke it will call a sub_receipe to add 
# debug flags to C flags using the environement and then compile
debug :
	make debug_flag 'CFLAGS=$(DEBUGFLAGS)'
debug_flag: $(objs-y)
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(INCLUDE) $^ -o debug_ls

#C and Asm source file compilation with C flags
%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -I$(INCLUDE) -c $^ -o $@

%.S.o: %.S
	$(AS) $(SFLAGS) -I$(INCLUDE) -c $^ -o $@

# Specify that "clean" is not a file and rather a command.
.PHONY: clean
clean:
	rm -rdf ls debug_ls $(SRC_DIR)/*.o